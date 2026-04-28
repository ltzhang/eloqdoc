# Hidden indexes (`hidden: true`)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 5.0 | 2 weeks |

## Scope

A `hidden: true` flag on an index makes it invisible to the query planner — the index is still maintained on writes, but the planner won't choose it. Used by operators to "test before drop": hide a candidate-for-removal index, monitor latency for a few days, then either un-hide or drop.

```js
db.c.createIndex({a:1}, {hidden: true})
db.c.createIndex({a:1}, {hidden: false})        // unhide via collMod
db.runCommand({collMod: "c", index: {keyPattern: {a:1}, hidden: true}})
db.c.hideIndex({a: 1})                          // shell helper (5.0)
db.c.unhideIndex({a: 1})
```

The planner must skip hidden indexes during plan enumeration; the plan cache must invalidate when an index's hidden state changes.

## Extension pattern

Catalog + planner integration. The shape:

1. **Storage:** add a `hidden` boolean to the index spec persisted in the catalog (`IndexDescriptor::hidden()`).
2. **`createIndexes` / `collMod`:** parse and persist the flag.
3. **Planner:** filter out hidden indexes during candidate enumeration.
4. **Plan cache:** invalidate cached plans for the collection when any index's `hidden` flips.
5. **Shell helpers:** `db.c.hideIndex(...)`, `db.c.unhideIndex(...)`.

```cpp
// IndexDescriptor::hidden() returning the persisted value
bool IndexDescriptor::hidden() const { return _infoObj.getBoolField("hidden"); }

// In QueryPlannerParams construction:
for (auto& ix : collection->getIndexCatalog()->getAllReadyIndexes()) {
    if (ix.descriptor()->hidden()) continue;  // skip
    plannerParams.indices.push_back(makeIndexEntry(ix));
}
```

## Files to create

- `src/mongo/shell/collection.js` (or wherever helpers live) — add `hideIndex`/`unhideIndex` JS wrappers

## Files to modify

- `src/mongo/db/index/index_descriptor.h` and `.cpp` — add `hidden()` accessor
- `src/mongo/db/catalog/index_catalog_entry_impl.{h,cpp}` — propagate flag from BSON spec
- `src/mongo/db/commands/create_indexes.cpp` — accept `hidden` in createIndexes
- `src/mongo/db/catalog/coll_mod.cpp` — accept `{collMod, index: {keyPattern, hidden}}` form
- `src/mongo/db/query/get_executor.cpp` (or `query_planner.cpp`) — filter hidden during plan enumeration
- `src/mongo/db/query/plan_cache.cpp` — invalidate on hidden-flag change
- `src/mongo/db/list_indexes.cpp` — surface `hidden` in `listIndexes` output

## Dependencies

- **Blocks:** [`prepareunique-workflow.md`](./prepareunique-workflow.md) — that task adds another transition state to indexes; sharing the catalog-plumbing pattern with hidden indexes makes it cheaper.
- **Blocked by:** none. However, after Tier 1 completion this is intentionally sequenced after the `bulkWrite` prerequisite chain (`update-delete-hint`, `let-runtime-constants`, `pipeline-update-syntax`) unless index compatibility becomes the higher product priority.

## EloqDoc-specific considerations

- **Plan cache invalidation hook.** Stock MongoDB invalidates the plan cache via `OpObserver` when index metadata changes; EloqDoc has no oplog, so the invalidation must be wired directly from the `collMod` / `createIndexes` execution path. Don't rely on observer dispatch.
- **Catalog persistence in EloqDoc.** Index specs are stored via Data Substrate. Verify the round-trip: `hidden: true` → persist → restart server → still `hidden: true`. This is a one-line BSON field, but worth a directed test.
- **Concurrent reads.** With `eloq_locker_noop`, ensure the planner reads `hidden` from a stable catalog snapshot — flipping `hidden` mid-query mustn't make the planner observe both states for the same index.

## Acceptance criteria

- [x] `db.c.createIndex({a:1}, {hidden: true})` creates the index; `db.c.find({a:1}).explain()` does NOT use it.
- [x] `collMod` flips `hidden`; subsequent `find` plans reflect the new state.
- [x] The plan cache invalidates: a query that previously used a now-hidden index gets re-planned, not stale.
- [x] Writes to indexed fields still update the hidden index (verify via TTL behavior or an index validation tool).
- [x] `listIndexes` reports `hidden: true` for hidden indexes.
- [x] Shell helpers `hideIndex`/`unhideIndex` work.
- **Test entry point:** `tests/jstests/eloq_basic/hidden_indexes.js`. Adapt `jstests/core/index_hidden_*.js`.

## Implementation notes

Initial support adds `IndexDescriptor::hidden()` backed by the persisted index spec, accepts `hidden` as a valid index option, and stores `collMod` hidden-state flips through the KV catalog metadata. Query planning filters hidden indexes out when building planner `IndexEntry` lists, including the distinct fast-path, while write maintenance continues to use the normal index catalog entries.

`collMod` supports both `{index: {name, hidden}}` and `{index: {keyPattern, hidden}}`, refreshes the in-memory index descriptor after metadata changes, and clears the collection query cache so the next query replans with the new visibility. Shell helpers `db.collection.hideIndex(...)` and `db.collection.unhideIndex(...)` delegate to that `collMod` form.

Current focused coverage verifies create-time hidden indexes, `listIndexes` visibility, planner exclusion, unhide/hide transitions, shell helpers, and write maintenance by inserting while hidden and then unhiding. A future hardening test should restart EloqDoc between create and `listIndexes` to explicitly validate Data Substrate metadata round-trip persistence.

## Notes from source analyses

`analysis_cc/forward_compat_eval.md`: "Catalog flag plus planner exclusion, plan cache invalidation, `collMod` hide/unhide, shell helpers." Files-to-create list above mirrors that. `analysis_gpt5.5` agrees on Tier 2. `analysis_gpt5.4` notes hidden indexes pair naturally with `commitQuorum` (both are catalog-flag-style features); a single PR adding both the IndexDescriptor field and command-side parsing scaffolding is plausible if you treat `commitQuorum` Tier 1 stub and hidden Tier 2 together.
