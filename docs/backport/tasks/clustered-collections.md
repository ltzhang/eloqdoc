# Clustered collections

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 3 (depends on EloqStore baseline) | 5.0 (limited) / 6.0 (GA) | 1–3 months |

## Scope

A clustered collection stores documents physically ordered by a chosen key (always `_id` in 5.0/6.0). The benefit is range-scan performance on the cluster key without a separate `_id` index — the data IS the index.

```js
db.createCollection("c", { clusteredIndex: { key: { _id: 1 }, unique: true } })
db.createCollection("logs", { clusteredIndex: true })       // shorthand for {_id:1, unique:true}
```

Once clustered, the collection has no separate `_id` index — the storage layer's primary order IS the `_id` index. Queries on `_id` use a clustered scan; range queries on `_id` are exceptionally fast.

## Architectural risk

The cost depends entirely on whether EloqStore (Data Substrate's record store) **already** stores documents ordered by `_id`. RocksDB-based stores typically do. If yes, this is mostly an exposure task (~1 month). If no, it's a major storage refactor (~3 months).

## Extension pattern

Pattern G + storage-engine plumbing.

```cpp
// CollectionOptions:
struct ClusteredCollectionInfo {
    BSONObj keyPattern;       // typically {_id: 1}
    bool unique = true;
    std::string name;
};
boost::optional<ClusteredCollectionInfo> clusteredIndex;

// Collection creation:
//  - Don't create the separate _id index.
//  - Tell the storage engine to order by the cluster key.
//
// Query planning:
//  - When predicate is on the cluster key, use a clustered scan, not an index scan.
//
// listIndexes:
//  - Synthesize an entry for the clustered index even though it's not a separate B-tree.
```

## Files to modify

- `src/mongo/db/catalog/collection_options.{h,cpp}` — `clusteredIndex` field
- `src/mongo/db/catalog/collection_impl.cpp` — skip `_id` index creation when clustered
- `src/mongo/db/modules/eloq/eloq_record_store.{h,cpp}` — accept "ordered by key" mode (or confirm it already is)
- `src/mongo/db/query/get_executor.cpp` — clustered scan path
- `src/mongo/db/query/planner_access.cpp` — recognize clustered key predicates
- `src/mongo/db/list_indexes.cpp` — synthesize clustered index entry

## Dependencies

- **Blocks:** Time-series basic phase ([`time-series-collections.md`](./time-series-collections.md)) — the `system.buckets.*` collections are clustered.
- **Blocked by:** none.

## EloqDoc-specific considerations

- **EloqStore baseline.** Audit `eloq_record_store.{h,cpp}` to determine if records are stored in `_id` order. RocksDB-based stores are sorted by key, so the answer is likely yes — but EloqStore may encode `_id` differently than the raw key, in which case some plumbing is needed.
- **Removing the `_id` index** requires verifying every code path that assumes it exists. Search for `IndexCatalogImpl::findIdIndex` and equivalents.
- **Distributed/sharded clustering.** Out of scope (single-node EloqDoc).
- **Cluster-key change.** Stock MongoDB doesn't support changing the cluster key after creation. Match — emit a clear error on attempted change.

## Acceptance criteria

- `db.createCollection("c", {clusteredIndex: true})` succeeds.
- `listIndexes` shows the clustered index but `db.c.getIndexes()` reports it specially.
- `db.c.find({_id: ...})` plan is "CLUSTERED_IXSCAN" (or equivalent), not "IXSCAN".
- Range queries on `_id` are correctly resolved via the clustered scan.
- Insert/update/delete work normally; cluster ordering is maintained.
- A non-clustered collection has the `_id` index as before.
- Validation reports the clustered index correctly.
- **Test entry point:** `tests/jstests/eloq_basic/clustered_collection.js`. Adapt `jstests/core/clustered/`.

## Notes from source analyses

`analysis_cc/forward_compat_eval.md` rates "Tier 2" with the explicit caveat: "WiredTiger (the storage engine) already stores records sorted by `_id`. The main work is removing the extra `_id` index and teaching the query planner to exploit the physical order." For EloqDoc, swap WiredTiger for EloqStore and the same logic applies.

`analysis_gpt5.5` rates "Tier 3" because it doesn't assume the storage engine cooperates. The right call for EloqDoc depends on EloqStore audit findings — start with that audit (1 week), then commit to Tier 2 or Tier 3 based on the result.
