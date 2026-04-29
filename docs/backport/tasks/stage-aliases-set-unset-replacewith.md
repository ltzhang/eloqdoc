# `$set` / `$unset` / `$replaceWith` stage aliases

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 | 5.0 | 1–2 days |

## Scope

Three syntactic-sugar aggregation stages added in 4.2/5.0 that rewrite to existing stages.

| Alias | Equivalent stock stage | Notes |
| ----- | ---------------------- | ----- |
| `$set` | `$addFields` | Identical semantics; `$set` is the new preferred name. |
| `$unset` | `$project` with `{f: 0}` | Single field or array of fields: `{$unset: "f"}` or `{$unset: ["a", "b"]}`. |
| `$replaceWith` | `$replaceRoot` with `{newRoot: <expr>}` | Same semantics; `$replaceWith` takes the expression directly instead of wrapping in `{newRoot: ...}`. |

EloqDoc already has `$addFields`, `$project`, and `$replaceRoot` registered — these are pure aliases.

## Extension pattern

Pattern B — `REGISTER_MULTI_STAGE_ALIAS` (single replacement) or just `REGISTER_DOCUMENT_SOURCE` returning a wrapped existing stage.

For `$set`:

```cpp
// document_source_set.cpp — thin wrapper that produces a $addFields
class DocumentSourceSet {
public:
    static boost::intrusive_ptr<DocumentSource> createFromBson(
        BSONElement elem, const intrusive_ptr<ExpressionContext>& expCtx) {
        return DocumentSourceAddFields::createFromBson(elem, expCtx);  // identical
    }
};
REGISTER_DOCUMENT_SOURCE(set,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourceSet::createFromBson);
```

`$unset` is slightly more involved — it must construct a `$project` with all listed fields excluded. `$replaceWith` rewrites `{$replaceWith: <expr>}` to `{$replaceRoot: {newRoot: <expr>}}` and dispatches.

## Files to create

- `src/mongo/db/pipeline/document_source_set.cpp`
- `src/mongo/db/pipeline/document_source_unset.cpp`
- `src/mongo/db/pipeline/document_source_replace_with.cpp`

(Some teams prefer to put all three in one file `document_source_aliases_50.cpp`. Either works.)

## Files to modify

- `src/mongo/db/pipeline/SConscript`

## Dependencies

- **Blocks:** none.
- **Blocked by:** none.

## EloqDoc-specific considerations

None. Pure pipeline-level rewrites; no storage or concurrency interaction.

## Acceptance criteria

- `db.c.aggregate([{$set: {x: 1}}])` produces identical results to `db.c.aggregate([{$addFields: {x: 1}}])`.
- `{$unset: "x"}` and `{$unset: ["x", "y"]}` both work.
- `{$replaceWith: "$nested"}` produces identical results to `{$replaceRoot: {newRoot: "$nested"}}`.
- Pipeline explain output shows the rewritten form (this is OK and matches upstream).
- **Test entry point:** `tests/jstests/eloq_basic/stage_aliases_50.js`.

## Notes from source analyses

All three analyses agree: trivial. `analysis_gpt5.5/raw/v5.0-aggregation_stages.txt` confirms these as the only 5.0 alias additions. EloqDoc already has `$count` (alias of `[{$group: {_id: null, count: {$sum: 1}}}, {$project: {_id: 0}}]`) — verify the existing `$count` alias still works after this change (it should; they're independent).
