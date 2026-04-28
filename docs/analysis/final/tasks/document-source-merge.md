# `$merge` aggregation stage

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 4.2 / 5.0 | 3 weeks |

## Scope

`$merge` writes pipeline output to a target collection with upsert semantics. Generalizes the older `$out` (which always replaces the entire collection).

```js
db.source.aggregate([
  ...,
  { $merge: {
      into: "target",
      on: "_id",                       // or array of fields forming a unique key
      whenMatched: "merge",            // "replace" | "keepExisting" | "merge" | "fail" | <pipeline>
      whenNotMatched: "insert",        // "insert" | "discard" | "fail"
      let: {...},
  }}
])
```

Cross-database write supported: `into: {db: "other", coll: "target"}`.

## Extension pattern

Pattern B (`REGISTER_DOCUMENT_SOURCE`) for the parser; the runtime implementation is a sink stage that performs an `update` with `upsert: true` per output document.

```cpp
class DocumentSourceMerge final : public DocumentSource {
public:
    GetNextResult getNext() final;          // pulls from source, writes to target, propagates EOF
    static intrusive_ptr<DocumentSource> createFromBson(...);
    const char* getSourceName() const final { return "$merge"; }
private:
    NamespaceString _targetNss;
    std::vector<FieldRef> _onFields;
    WhenMatched _whenMatched;
    WhenNotMatched _whenNotMatched;
    BSONObj _let;
    std::unique_ptr<MergeStrategy> _strategy;  // dispatches on _whenMatched
};
REGISTER_DOCUMENT_SOURCE(merge,
                         DocumentSourceMerge::LiteParsed::parse,
                         DocumentSourceMerge::createFromBson);
```

For each pipeline output document `D`:
1. Build the lookup query from `_onFields` + `D`'s values.
2. Issue an `update` with `upsert: true` and the constructed update spec:
   - `whenMatched: "replace"` → `{$set: D, $setOnInsert: {...}}`
   - `whenMatched: "merge"` → field-wise `$set`
   - `whenMatched: "keepExisting"` → upsert-only (no update)
   - `whenMatched: "fail"` → assert no match exists; otherwise error
   - `whenMatched: <pipeline>` → pipeline-form update (depends on [`pipeline-update-syntax.md`](./pipeline-update-syntax.md))
3. Honor `whenNotMatched` for the upsert path.

## Files to create

- `src/mongo/db/pipeline/document_source_merge.{cpp,h}`
- `src/mongo/db/pipeline/merge_strategy.{cpp,h}` — one strategy class per `whenMatched` mode

## Files to modify

- `src/mongo/db/pipeline/SConscript`
- `src/mongo/db/pipeline/lite_parsed_pipeline.{cpp,h}` — `$merge`'s lite-parsed form must declare write privileges on the target

## Dependencies

- **Blocks:** none (it's a sink).
- **Blocked by:**
  - [`pipeline-update-syntax.md`](./pipeline-update-syntax.md) — the `whenMatched: <pipeline>` form requires it.
  - [`let-runtime-constants.md`](./let-runtime-constants.md) — the optional `let` field on `$merge`.

## EloqDoc-specific considerations

- **No oplog ⇒ atomicity is per-document.** Stock MongoDB on a replica set guarantees per-document atomicity but not whole-pipeline atomicity. EloqDoc's per-document atomicity is provided by Data Substrate transactions; verify the per-document `update` with `upsert: true` runs as one substrate transaction.
- **Cross-database writes.** EloqDoc's namespace resolution must support writes to a database other than the one being read from. Confirm; this works in stock MongoDB without sharding.
- **Authorization.** The lite-parsed pipeline needs to declare both read on the source collection AND write on the target. Use the existing `LiteParsedPipeline::requiredPrivileges` framework.

## Acceptance criteria

- [x] `$merge` into a new collection (collection auto-created).
- [x] Each `whenMatched` mode produces correct results: `replace`, `keepExisting`, `merge`, `fail`.
- [ ] `whenMatched: <pipeline>` works (depends on pipeline-update task).
- [x] `whenNotMatched: "discard"` skips inserts; `"fail"` errors when no match.
- [ ] Cross-database: `{into: {db: "other", coll: "t"}}` writes to `other.t`.
- [ ] Authorization fails for users without write on the target.
- [ ] A pipeline that writes 1M documents completes (no memory blowup; per-document write).
- **Test entry point:** `tests/jstests/eloq_basic/agg_merge.js`. Adapt `jstests/aggregation/sources/merge/`.

## Implementation status

Implemented an initial same-database sink stage:

- Supports string and object `into` forms targeting the current database.
- Supports top-level `on` fields, defaulting to `_id`.
- Supports string `whenMatched` modes: `merge`, `replace`, `keepExisting`, and `fail`.
- Supports `whenNotMatched` modes: `insert`, `discard`, and `fail`.
- Writes through the existing direct-client write path and checks `getLastErrorDetailed()` after mutations.
- Runtime coverage is in `tests/jstests/eloq_basic/agg_merge.js`.

Deferred:

- Cross-database output currently fails fast with `IllegalOperation`. The direct-client write path hit a catalog invariant when writing another database from an aggregation locked on the source database, so this needs a catalog/locking design pass.
- `whenMatched: <pipeline>` and `$merge.let` are not implemented in this checkpoint.
- Dotted `on` fields, explicit authorization tests, and large streaming coverage remain open.

## Notes from source analyses

All three analyses rate this Tier 2 with consistent scoping. `analysis_cc/forward_compat_eval.md` provides the most concrete strategy decomposition (one class per `whenMatched` mode). `analysis_gpt5.5` flags: "writes from pipeline with unique key and retry/write concern semantics" — be careful that the upsert key must be a uniquely indexed field set, otherwise upsert-on-conflict has undefined behavior. The error-mode handling (`whenMatched: "fail"`) requires a transactional read+write per document; if EloqDoc's substrate doesn't expose per-doc transactions, this mode may need to be deferred.
