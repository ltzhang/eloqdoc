# `$listCatalog` aggregation stage

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 6.0 | 1 week |

## Scope

A pipeline source stage that emits one document per collection (or matching collections) describing catalog metadata: collection options, indexes, UUID, etc.

```js
db.aggregate([{$listCatalog: {}}])           // database-level
db.coll.aggregate([{$listCatalog: {}}])       // collection-level
```

Two forms:
- **Collection-level:** invoked on a specific collection; emits one document for that collection.
- **Database-level:** invoked via `db.aggregate()`; emits one document per collection in the database.

Output schema mirrors `listCollections` + index information.

## Extension pattern

Pattern B — `REGISTER_DOCUMENT_SOURCE`. The stage reads from the catalog, similar to `$indexStats` and `$collStats` (both already in EloqDoc).

```cpp
class DocumentSourceListCatalog final : public DocumentSource {
    GetNextResult getNext() final;
    StageConstraints constraints(...) const final {
        // FirstStageOnly; database-or-collection level
    }
};
REGISTER_DOCUMENT_SOURCE(listCatalog, ...);
```

## Files to create

- `src/mongo/db/pipeline/document_source_list_catalog.{cpp,h}`

## Files to modify

- `src/mongo/db/pipeline/SConscript`
- (potentially) `src/mongo/db/commands/run_aggregate.cpp` — the database-level form requires the same "no source collection" handling as `$documents`.

## Dependencies

- **Blocks:** none.
- **Blocked by:** [`document-source-documents.md`](./document-source-documents.md) — the database-level aggregate path is a prerequisite. Bundle the two PRs together.

## EloqDoc-specific considerations

- **Catalog snapshot.** Iterate the catalog under whatever read snapshot mechanism EloqDoc uses. Don't expose mid-DDL transient state.
- **EloqDoc-specific options.** If EloqDoc has additional collection options (Data Substrate placement hints, etc.), surface them in the output for completeness.

## Acceptance criteria

- `db.aggregate([{$listCatalog:{}}])` returns one document per collection in the current database.
- `db.coll.aggregate([{$listCatalog:{}}])` returns the single document for `coll`.
- Each document contains `db`, `name`, `type`, `md.options`, `md.indexes`, `idxIdent`, `ns`, etc. — match upstream output.
- Authorization: requires `listCollections` privilege on the database.
- **Test entry point:** `tests/jstests/eloq_basic/agg_list_catalog.js`. Adapt `jstests/aggregation/sources/listCatalog.js`.

## Notes from source analyses

`analysis_gpt5.5/raw/v6.0-aggregation_stages.txt` confirms 6.0 addition. `analysis_cc/forward_compat_eval.md` rates 1 week and notes: "Output format is documented but not stable across versions — match what mongosh expects, not the older API." Heed.
