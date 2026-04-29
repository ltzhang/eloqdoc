# `$planCacheStats` stage

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 | 4.2 / 5.0 | 2–3 days |

## Scope

A pipeline stage that emits one document per entry in the per-collection plan cache. Used by the shell helper `db.coll.getPlanCache().list()` and replaces the older `planCacheListPlans`/`planCacheListQueryShapes` admin commands (which were deprecated).

```js
db.coll.aggregate([{$planCacheStats: {}}])
// → { queryHash, planCacheKey, query, sort, projection, plans: [...], ... }
```

Must be the first stage. Operates against the named collection's plan cache, not a global one.

## Extension pattern

Pattern B — `REGISTER_DOCUMENT_SOURCE`. The plan cache infrastructure already exists in EloqDoc (4.0.3 has it for the existing CRUD commands); this stage is a thin reader.

```cpp
class DocumentSourcePlanCacheStats final : public DocumentSource {
public:
    GetNextResult getNext() final;
    const char* getSourceName() const final { return "$planCacheStats"; }
    StageConstraints constraints(...) const final {
        // FirstStageOnly, requires collection.
    }
    static intrusive_ptr<DocumentSource> createFromBson(...);
private:
    std::vector<BSONObj> _entries;  // populated lazily on first getNext
    size_t _idx = 0;
};
REGISTER_DOCUMENT_SOURCE(planCacheStats,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourcePlanCacheStats::createFromBson);
```

## Files to create

- `src/mongo/db/pipeline/document_source_plan_cache_stats.cpp`

## Files to modify

- `src/mongo/db/pipeline/SConscript`
- Possibly extend `PlanCache` (in `src/mongo/db/query/plan_cache.h`) with an `iterate()` method if one doesn't exist. Each entry should serialize to BSON via existing helpers.

## Dependencies

- **Blocks:** none.
- **Blocked by:** none.

## EloqDoc-specific considerations

- **Plan-cache invalidation in EloqDoc.** Verify the plan cache is per-collection (it should be, matching upstream). If EloqDoc has a custom plan cache implementation in `src/mongo/db/modules/eloq/`, this stage should iterate that cache.
- **Catalog interactions.** The stage must hold a read lock on the collection to enumerate the cache safely. With `eloq_locker_noop`, this means using whatever EloqDoc's catalog-snapshot mechanism is.
- **Semantic difference:** EloqDoc 4.0 does not have MongoDB's upstream `queryHash` machinery. The compatibility stage reports `queryHash` as a deterministic hash of EloqDoc's local `planCacheKey`, so clients can group entries, but the value is not byte-for-byte comparable with MongoDB 4.2+ `queryHash` output.

## Acceptance criteria

- After running a few `find` queries that hit different shapes, `$planCacheStats` returns one entry per shape.
- The reply documents include `queryHash`, `planCacheKey`, `query`, `sort`, `projection`, `plans` array.
- Running on an empty collection / fresh cache returns zero documents.
- Running on a non-existent collection returns a clear error (not a crash).
- Used as a non-first stage produces a parse error.
- **Test entry point:** `tests/jstests/eloq_basic/agg_plan_cache_stats.js`.

## Notes from source analyses

`analysis_cc/forward_compat_eval.md` rates this Category 1 explicitly: "The plan cache infrastructure already exists in 4.0. This stage just reads entries from the in-memory plan cache and formats them as documents. Effort: 2–3 days." `analysis_gpt5.5` agrees. EloqDoc-specific risk: if the plan cache hooks into Data Substrate caching, the iteration may need locking primitives that don't yet exist for the cache structure. Minor.
