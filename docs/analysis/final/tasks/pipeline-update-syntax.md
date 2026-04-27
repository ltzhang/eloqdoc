# Pipeline-form update (`update.u: [{$set:...}, ...]`)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 4.2 / 5.0 widespread | 1 week |

## Scope

In addition to the operator form `{$set: {x: 1}, $inc: {n: 1}}`, MongoDB 4.2+ accepts an array of aggregation stages in the `u` field of an update:

```js
db.c.updateMany(
  {category: "x"},
  [
    {$set: {total: {$add: ["$qty", "$bonus"]}}},
    {$unset: ["bonus"]},
  ]
)
```

Allowed stages in this form: `$set`, `$unset`, `$replaceWith`/`$replaceRoot`, `$addFields`, `$project`. Each runs in sequence on the matched document; the final document replaces the original.

## Extension pattern

Update-command parsing + executor.

```cpp
// In ParsedUpdate::parseRequest (src/mongo/db/ops/parsed_update.cpp):
BSONElement uEl = updateOp["u"];
if (uEl.type() == Array) {
    // Pipeline-form: parse each stage as a Pipeline, then execute on each matched doc.
    _parsedUpdateExpression = ParsedUpdatePipeline::parse(uEl.Obj(), expCtx);
}
else {
    // Operator-form: existing path.
}

// Execute (in UpdateStage::transformAndUpdate):
if (_parsedUpdateExpression->isPipeline()) {
    // Run the matched document through the pipeline (in-memory single-document pipeline).
    // The output document replaces the matched document.
}
```

## Files to modify

- `src/mongo/db/ops/parsed_update.{h,cpp}` — accept array form
- `src/mongo/db/ops/update_driver.{h,cpp}` — represent pipeline-form update as a one-document pipeline
- `src/mongo/db/exec/update.cpp` (or `update_stage.cpp`) — execution path
- `src/mongo/db/commands/find_and_modify.cpp` — pipeline form also valid in `findAndModify`
- `src/mongo/db/ops/write_ops_parsers.h` (if used) — accept array `u`

## Dependencies

- **Blocks:** [`document-source-merge.md`](./document-source-merge.md) — `$merge` with `whenMatched: <pipeline>` reuses the same pipeline-update path.
- **Blocked by:** [`stage-aliases-set-unset-replacewith.md`](./stage-aliases-set-unset-replacewith.md) (Tier 1) — those aliases must work as in-pipeline stages first.

## EloqDoc-specific considerations

- **Update diffs.** Stock MongoDB serializes pipeline-form updates differently in the oplog. EloqDoc has no oplog, so this concern doesn't apply directly — but Data Substrate's redo log may have similar serialization needs. Confirm with the storage team.
- **Atomicity.** A pipeline update on a single matched document must be atomic. The existing operator-form atomicity guarantees should carry over since the pipeline runs synchronously.

## Acceptance criteria

- `db.c.updateOne(filter, [{$set:{...}}])` works.
- `findAndModify` with pipeline-form `update` works.
- Pipeline can reference fields of the matched document (`$qty`, `$$NOW`, etc.).
- Stages outside the allowed list produce a clear parse error (`$lookup`, `$group`, etc.).
- Mixing operator and pipeline form produces an error (`u` must be one or the other).
- `let` variables ([`let-runtime-constants.md`](./let-runtime-constants.md)) accessible inside the pipeline as `$$varName`.
- **Test entry point:** `tests/jstests/eloq_basic/pipeline_update.js`. Adapt `jstests/core/update_pipeline_*.js`.

## Notes from source analyses

`analysis_cc/forward_compat_eval.md` and `analysis_gpt5.5` both rate this Tier 2 (~1 week). `analysis_gpt5.4` flags it as a "missing-on-4.0" feature (4.2 addition). `analysis_cc` notes that downstream features like `$merge`'s pipeline-`whenMatched` build directly on this code path — sequencing this before `$merge` saves duplicate work.
