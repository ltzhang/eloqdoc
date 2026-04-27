# `prepareUnique` workflow (convert non-unique → unique)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 8.0 | 2–3 weeks |

## Scope

The workflow for safely converting a non-unique index into a unique one without rebuilding it. Three states:

1. **Normal** (no flags) — current behavior; allows duplicates.
2. **`prepareUnique: true`** — set via `collMod`. Index still allows reads of existing duplicates, but rejects new writes that would create duplicates. Used to monitor for duplicates before flipping unique.
3. **`unique: true`** — set via `collMod` (with `prepareUnique: true` precondition). Atomic: scans the collection for existing duplicates, fails if any found, otherwise sets the unique flag.

```js
db.runCommand({collMod: "c", index: {keyPattern: {email: 1}, prepareUnique: true}})
// ... wait days, let writes detect dup attempts ...
db.runCommand({collMod: "c", index: {keyPattern: {email: 1}, unique: true}})
```

## Extension pattern

Catalog state machine + write-path interception.

```cpp
// IndexDescriptor gets a "prepareUnique" flag (similar to "hidden")
bool IndexDescriptor::prepareUnique() const { return _infoObj.getBoolField("prepareUnique"); }

// Insert path: BtreeAccessMethod::insert (or the EloqDoc equivalent):
if (descriptor->prepareUnique() || descriptor->unique()) {
    // Probe for an existing key; if found, return DuplicateKey.
}

// collMod path for unique:true:
//  1. Verify prepareUnique was set (precondition)
//  2. Scan all keys; abort if any duplicates exist
//  3. Atomically flip unique:true and clear prepareUnique
```

## Files to modify

- `src/mongo/db/index/index_descriptor.h` and `.cpp` — `prepareUnique()` accessor
- `src/mongo/db/catalog/index_catalog_entry_impl.{h,cpp}` — propagate flag
- `src/mongo/db/index/btree_access_method.cpp` — duplicate-detection in insert path when `prepareUnique` is set
- `src/mongo/db/catalog/coll_mod.cpp` — accept `prepareUnique` and the unique-conversion form; implement scan-and-flip
- `src/mongo/db/list_indexes.cpp` — surface `prepareUnique` in output
- `src/mongo/db/modules/eloq/eloq_index.{h,cpp}` — verify the index implementation surfaces duplicate detection

## Dependencies

- **Blocks:** none.
- **Blocked by:** [`hidden-indexes.md`](./hidden-indexes.md) — share the IndexDescriptor flag plumbing pattern.

## EloqDoc-specific considerations

- **Distributed write detection.** With Data Substrate, duplicate detection must be consistent across nodes. Verify whether a parallel write on another node could pass the local probe and create a duplicate. The Substrate's MVCC/conflict-detection should already handle this for the existing unique index path; reuse that mechanism for `prepareUnique`.
- **Atomicity of the flip.** The scan-and-flip in `collMod unique: true` must be atomic from clients' perspective. Stock MongoDB uses a write-conflict-on-flip trick; EloqDoc may need a Data Substrate transaction wrapping the catalog write.
- **No oplog.** Don't expect an op-observer event to propagate the state change; emit any required notification directly from the `collMod` execution path (relevant for plan-cache invalidation if the unique flag affects planning).

## Acceptance criteria

- `collMod` with `prepareUnique: true` succeeds; subsequent inserts that would dup fail with `DuplicateKey`; existing dup-bearing reads still succeed.
- `collMod` with `unique: true` and a clean collection succeeds and flips the flag.
- `collMod` with `unique: true` on a collection with existing duplicates fails with `DuplicateKey` and does NOT flip.
- `listIndexes` reflects intermediate state.
- After `unique: true` succeeds, reverting to non-unique requires a new `collMod` (`unique: false` if supported, else dropIndex/recreate).
- **Test entry point:** `tests/jstests/eloq_basic/prepare_unique.js`. Adapt `jstests/core/index_collmod_unique_*.js`.

## Notes from source analyses

`analysis_gpt5.5` rates this Tier 2: "Needs duplicate detection, catalog state, write conflict handling, and `collMod` integration." `analysis_cc` agrees and explicitly groups it with hidden indexes for shared infrastructure. The duplicate-detection-in-write-path is the trickiest piece; consider it the architectural risk.
