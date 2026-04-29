# `$documents` stage

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 | 6.0 | 2 days |

## Scope

A new pipeline source stage that takes a literal array of documents (or an expression that resolves to an array) and emits them as the pipeline input. Useful for testing, fixed lookup tables, and "select <constant>" patterns.

```js
db.aggregate([
  { $documents: [
      { _id: 1, name: "alpha" },
      { _id: 2, name: "beta" }
  ] },
  { $match: { _id: { $gt: 1 } } }
])
```

`$documents` must be the first stage of an aggregation. It's a database-level (or admin-level) command form — `db.aggregate(...)` rather than `db.coll.aggregate(...)` — because there's no source collection.

## Extension pattern

Pattern B — `REGISTER_DOCUMENT_SOURCE`. Subclass `DocumentSource` directly (not a regular pipeline stage; it's a source).

```cpp
class DocumentSourceDocuments final : public DocumentSource {
public:
    GetNextResult getNext() final;
    const char* getSourceName() const final { return "$documents"; }
    StageConstraints constraints(...) const final {
        // FirstStageOnly, no host requirement, can run on mongod.
    }
    static intrusive_ptr<DocumentSource> createFromBson(BSONElement, const intrusive_ptr<ExpressionContext>&);
private:
    std::vector<Document> _docs;
    size_t _idx = 0;
};
REGISTER_DOCUMENT_SOURCE(documents,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourceDocuments::createFromBson);
```

## Files to create

- `src/mongo/db/pipeline/document_source_documents.cpp` (and `.h`)

## Files to modify

- `src/mongo/db/pipeline/SConscript`
- The aggregate command parser path needs to allow a `db.aggregate` form (no collection name). Check `src/mongo/db/commands/run_aggregate.cpp` (or `pipeline_command.cpp`) — most existing code requires a collection. Add a guard: if first stage is `$documents`, don't require a source collection.

## Dependencies

- **Blocks:** none.
- **Blocked by:** none. May coordinate with `document-source-unionwith.md` (Tier 2) since `$unionWith` can take a `$documents`-rooted nested pipeline.

## EloqDoc-specific considerations

- The "no source collection" form (`db.aggregate(...)`) bypasses normal namespace acquisition — verify EloqDoc's command path doesn't unconditionally require a collection lock on the input namespace.
- StageConstraints: must declare `kFirstStage` so the optimizer rejects placement after another stage.

## Acceptance criteria

- `db.aggregate([{$documents: [{a:1},{a:2}]}])` returns two documents.
- Mixing `$documents` with downstream stages (`$match`, `$project`, `$group`) works.
- Using `$documents` not as the first stage produces a parse error.
- Using `$documents` against `db.coll.aggregate(...)` (with a source collection) is allowed only if `$documents` is the first stage and the source-collection name is ignored — verify upstream behavior here.
- **Test entry point:** `tests/jstests/eloq_basic/agg_documents.js`. Adapt `jstests/aggregation/sources/documents/`.

## Notes from source analyses

`analysis_gpt5.5/feature-backport-evaluation.md` rates this Tier 1. `analysis_cc/forward_compat_eval.md` agrees and explicitly notes: "the only complication is the no-collection aggregate form — handle it once, and `$listCatalog` and other db-level pipeline stages get the same plumbing for free." Worth bundling those in a single PR.

## Implementation status

Implemented before the `$listCatalog` slice. EloqDoc supports literal object arrays in
`$documents`, allows the stage as the first stage of collectionless aggregate commands, and allows
collection-level aggregate with `$documents` as the first stage while ignoring the source
collection. The regression entry point is `tests/jstests/eloq_basic/agg_documents.js`.
