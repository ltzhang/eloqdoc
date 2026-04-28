# Time-Series Basic Plan — Issues

Review of `2026-04-28-timeseries-basic.md`. Eight issues found across two severity levels.

---

## Critical

### C1: Internal insert router will hit `InvalidNamespace` on `system.buckets.*`

**Affects:** Task 3 (Insert Routing)

`userAllowedCreateNS()` — which `userAllowedWriteNS()` delegates to — rejects any `system.*`
namespace not on its explicit allowlist (`src/mongo/db/ops/insert.cpp:227–260`).
`system.buckets.*` is not on that list.

`performInserts()` calls `uassertStatusOK(userAllowedWriteNS(...))` at
`write_ops_exec.cpp:591` before the batch flush loop. This fires on the logical namespace
(`metrics`) which is fine. But the plan then says to acquire the bucket collection and
"Insert into the buckets collection." If that second insert goes through `performInserts()`
recursively, the `system.buckets.metrics` check will throw `InvalidNamespace` and the
entire insert will fail.

**Fix:** The insert router must write to the bucket collection by acquiring it directly via
`AutoGetCollection(opCtx, bucketNss, MODE_IX)` and calling
`collection->insertDocuments(...)` inside its own `WriteUnitOfWork`. Do not call
`performInserts()` for the routed write. Note: `db->createCollection()` inside
`create_collection.cpp` uses `Database::userCreateNS()` directly and bypasses
`userAllowedCreateNS()`, so bucket collection *creation* is unaffected — only the insert
path is broken.

---

### C2: `expireAfterSeconds` is not a field in `CollectionOptions`

**Affects:** Task 1 (Parse And Persist Time-Series Options), Task 6 (TTL)

In the 4.0.3 codebase `expireAfterSeconds` is an index option, not a collection option. It
does not exist in `CollectionOptions` and is not parsed by `CollectionOptions::parse()`. The
`createCollection` path strips generic arguments and passes the remainder to
`CollectionOptions::parse()`, which will reject `expireAfterSeconds` as an unknown field
before `timeseries` is even reached.

The Task 1 test asserts `infos[0].options.expireAfterSeconds == 3600`, but
`createCollection` will fail with `InvalidOptions` first.

**Fix:** Add `boost::optional<long long> expireAfterSeconds` to `CollectionOptions`
(alongside `timeseries`). Parse it in `CollectionOptions::parse()` and serialize it in
`appendBSON()`. Add it to the Task 1 file list:

- Modify: `src/mongo/db/catalog/collection_options.h` — add the field
- Modify: `src/mongo/db/catalog/collection_options.cpp` — parse and serialize it

---

## Important

### I1: TTL monitor is entirely index-based — collection-level bucket TTL needs a new scan path

**Affects:** Task 6 (Bucket-Level TTL)

The TTL monitor (`src/mongo/db/ttl.cpp`) works by iterating TTL indexes from
`ttl_collection_cache`. Its `doTTLForIndex()` takes an index descriptor and deletes
documents older than `expireAfterSeconds` on the indexed field. There is no concept of
collection-level TTL anywhere in the monitor.

For bucket TTL the plan needs either:

- **Option A:** A second scan loop in `TTLMonitor::doTTLPass()` that enumerates all
  collections with `options.timeseries && options.expireAfterSeconds`, acquires the
  corresponding `system.buckets.<name>` collection, and deletes buckets where
  `control.max.<timeField> < now - expireAfterSeconds`. This keeps the bucket structure
  internal.
- **Option B:** At time-series collection creation, register a real TTL index on
  `system.buckets.<name>` over `control.max.<timeField>` with the given
  `expireAfterSeconds`. This reuses the existing monitor with no new scan loop, but
  `listIndexes` on the bucket collection will expose the internal field name.

Option A is cleaner. Either way, the integration point must be designed before Task 6 is
started; the plan's current instruction ("locate the active TTL implementation") does not
describe it.

---

### I2: Two `createCollection` calls in one `WriteUnitOfWork` may not be atomic in EloqDoc

**Affects:** Task 2 (Create Bucket Namespace)

The plan creates the bucket collection inside the same `WriteUnitOfWork` as the logical
collection in `create_collection.cpp`. A comment in `write_ops_exec.cpp` reads:

```
// In EloqDoc, create collection operation commits transaction.
```

If each `createCollection` internally commits its own transaction, the logical collection
and the bucket collection would be created in separate transactions. A crash between them
would leave a logical time-series collection with no corresponding bucket collection — an
inconsistent state that subsequent inserts and reads would not handle gracefully.

**Fix:** Verify EloqDoc's transaction semantics for nested `createCollection` calls before
implementing Task 2. If they cannot be made atomic, add a startup reconciliation pass that
detects and repairs orphaned logical time-series collections (create the missing bucket
collection, or drop the logical collection and let the user retry).

---

## Moderate

### M1: `BucketCatalog` key needs canonical BSON serialization for the meta field

**Affects:** Task 3 (Bucket Catalog)

`BucketKey` contains a `BSONObj meta` field, but `_openBuckets` is
`unordered_map<std::string, BucketHandle>`. The string key must encode namespace + meta +
rounded time. BSON objects are not canonically ordered: two documents with the same fields
in different insertion order produce different byte strings, and would open separate buckets
instead of sharing one.

**Fix:** When building the string key, iterate the meta object's fields in sorted key order
(or use `BSONObj::woCompare` canonical form) before appending to the key string. A simple
approach: copy fields into a `std::map<std::string, BSONElement>`, then serialize in
alphabetical order.

---

### M2: Translated aggregation leaks `system.buckets.*` in explain output

**Affects:** Task 5 (Query Translation)

When a user runs `db.metrics.explain().aggregate(...)`, the translated pipeline runs against
`system.buckets.metrics`. The explain output's `ns` field and query plan will show
`system.buckets.metrics`, leaking the internal bucket namespace. Stock MongoDB hides this
by rewriting the explain output after execution.

This is a UX issue rather than a correctness bug and acceptable for the MVP, but it should
be tracked so it is not forgotten. Add a follow-up note in the Task 7 doc update
(Step 3).

---

## Minor

### N1: `isBucketNamespace` should use `startsWith()` instead of `find() == 0`

**Affects:** Task 2 (Namespace Helpers)

The proposed implementation:

```cpp
bool isBucketNamespace(const NamespaceString& nss) {
    return nss.coll().find("system.buckets.") == 0;
}
```

`StringData` in this codebase has a `startsWith()` method. Use it:

```cpp
bool isBucketNamespace(const NamespaceString& nss) {
    return nss.coll().startsWith("system.buckets.");
}
```

---

### N2: Bucket namespace guard in `create_collection.cpp` is placed after collection creation

**Affects:** Task 2 (Create Bucket Namespace)

The plan inserts this `uassert` inside the `writeConflictRetry` lambda, after
`Database::userCreateNS()` has already created the logical collection:

```cpp
uassert(ErrorCodes::InvalidOptions,
        "time-series collections cannot be created under system.buckets",
        !timeseries::isBucketNamespace(nss));
```

If the assert fires, the write unit rolls back correctly, but work was done unnecessarily.
In practice `userAllowedCreateNS()` (called at the top of `createCollection()`) already
rejects `system.buckets.foo` before reaching this code, making the check redundant. If
kept for defense-in-depth, move it to before the `writeConflictRetry` lambda alongside the
existing `userAllowedCreateNS` call.

---

## Quick Reference

| ID | Severity | Task | One-line summary |
|----|----------|------|-----------------|
| C1 | Critical | 3 | Insert router must bypass `performInserts()`; use `Collection::insertDocuments()` directly |
| C2 | Critical | 1, 6 | Add `expireAfterSeconds` to `CollectionOptions`; it is currently index-only |
| I1 | Important | 6 | TTL monitor is index-based; bucket TTL needs a new scan loop or index registration |
| I2 | Important | 2 | Two `createCollection` calls in one WUOW may not be atomic in EloqDoc |
| M1 | Moderate | 3 | BucketCatalog key must use canonical BSON field ordering for the meta component |
| M2 | Moderate | 5 | Translated aggregation leaks `system.buckets.*` in explain output |
| N1 | Minor | 2 | Use `startsWith()` instead of `find() == 0` in `isBucketNamespace` |
| N2 | Minor | 2 | Move bucket namespace guard before the `writeConflictRetry` lambda |
