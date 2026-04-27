# `$unionWith` aggregation stage

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 (1 if minimal nested execution exists) | 4.4 / 5.0 | 2 weeks |

## Scope

`$unionWith` concatenates documents from another namespace (or another pipeline) into the current pipeline's stream. Order of output is unspecified (effectively the source pipeline's results followed by the unioned results).

```js
db.products.aggregate([
  { $match: { active: true } },
  { $unionWith: { coll: "archive", pipeline: [{ $match: { archived_recently: true }}] } }
])
```

`pipeline` is optional — if omitted, the entire `coll` is unioned.

## Extension pattern

Pattern B (`REGISTER_DOCUMENT_SOURCE`). The stage owns a child Pipeline that's lazily executed once the parent stream is exhausted.

```cpp
class DocumentSourceUnionWith final : public DocumentSource {
    GetNextResult getNext() final {
        // Phase 1: drain own input until EOF.
        // Phase 2: open the child Pipeline against `coll`, drain it.
    }
    static intrusive_ptr<DocumentSource> createFromBson(...);
private:
    NamespaceString _coll;
    std::vector<BSONObj> _stagesBson;
    std::unique_ptr<Pipeline, PipelineDeleter> _childPipeline;
    bool _drainedParent = false;
};
REGISTER_DOCUMENT_SOURCE(unionWith, ...);
```

The hardest part is constructing the child Pipeline against the target namespace from inside another stage's runtime. The infrastructure for this exists in 4.0 (`$lookup` does it) — adapt that pattern.

## Files to create

- `src/mongo/db/pipeline/document_source_union_with.{cpp,h}`

## Files to modify

- `src/mongo/db/pipeline/SConscript`
- `src/mongo/db/pipeline/lite_parsed_pipeline.{cpp,h}` — `$unionWith` must declare read privilege on the unioned namespace

## Dependencies

- **Blocks:** none directly; pairs naturally with `$merge` and other compound pipelines.
- **Blocked by:** none.

## EloqDoc-specific considerations

- **Cross-collection access from a stage.** `$lookup` already does this in 4.0; reuse the namespace acquisition / sub-pipeline construction code.
- **No replica-set targeting.** Stock MongoDB has logic for routing the sub-pipeline to a different shard/secondary. EloqDoc's single-node model means the sub-pipeline runs in-process. Simpler.

## Acceptance criteria

- `$unionWith: "other_coll"` produces source + other collection's documents.
- `$unionWith: { coll: "x", pipeline: [{$match: {...}}] }` filters before union.
- Empty source pipeline / empty target pipeline both produce correct unions.
- Authorization fails if user lacks read on the unioned namespace.
- Nested `$unionWith` (one inside another) works.
- Schema validation off: documents from `other` may have any shape — output is heterogeneous. Caller is responsible for downstream `$project`.
- **Test entry point:** `tests/jstests/eloq_basic/agg_union_with.js`. Adapt `jstests/aggregation/sources/unionWith/`.

## Notes from source analyses

**Source disagreement noted.** `analysis_cc/forward_compat_eval.md` rates this Tier 1 — relying on the existing `$lookup` sub-pipeline infrastructure. `analysis_gpt5.4` and `analysis_gpt5.5` rate it Tier 2, citing nested-pipeline execution complexity.

**Verdict:** Tier 2 is the safer call because EloqDoc's `$lookup` may have local optimizations not generalizable. If profiling shows `$lookup`'s sub-pipeline is cleanly factored, treat as Tier 1 and ship in days. Otherwise, the documented 2 weeks accounts for refactoring sub-pipeline construction into a reusable helper.
