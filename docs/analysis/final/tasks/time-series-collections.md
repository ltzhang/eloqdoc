# Time-series collections

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 (basic insert+find+TTL) / 3 (full parity) | 5.0 | 6 weeks (basic) / 3–6 months (full) |

## Scope

Time-series collections are the largest data-model addition in 5.0. They appear to clients as ordinary collections but are stored internally as **bucket collections** — each "bucket" is a single document containing many measurements packed into columnar arrays.

**User-facing API:**

```js
db.createCollection("metrics", {
  timeseries: {
    timeField: "ts",
    metaField: "tags",
    granularity: "minutes",                // "seconds" | "minutes" | "hours"
    bucketMaxSpanSeconds: 3600,            // optional, fine-grained
    bucketRoundingSeconds: 60
  },
  expireAfterSeconds: 86400                  // TTL on the timeField
})
```

Inserts into `metrics` route to a hidden `system.buckets.metrics`. Queries on `metrics` are translated to queries on `system.buckets.metrics` plus an `$_internalUnpackBucket` stage.

## Phasing (per `analysis_cc/forward_compat_eval.md`)

### Tier 2 — basic (~6 weeks)
1. **Storage:** create `system.buckets.<name>` as the physical storage (regular collection, clustered or hash-keyed by `_id`).
2. **Bucket catalog:** in-memory map `(metaValue, timeBucket) → bucketId`. ~2 weeks.
3. **Insert routing:** intercept `insert` on time-series namespace; route to bucket catalog; pack measurements into the bucket's columnar `data` field. ~2 weeks.
4. **Query translation:** intercept `find`/`aggregate`; rewrite namespace to `system.buckets.<name>`; inject `$_internalUnpackBucket` stage. ~1 week.
5. **TTL:** background task scans buckets and deletes those entirely past `expireAfterSeconds`. ~3 days.
6. **`create` parsing:** `timeseries` option in CollectionOptions. ~half a week.
7. **View registration:** `<name>` registered as a view over `system.buckets.<name>` so that mongosh `db.getCollectionInfos()` etc. see both. ~half a week.

### Tier 3 — full parity (~3–6 months on top of basic)
- **Updates** on time-series collections (7.0, 8.0 added upserts).
- **Deletes** on individual measurements (7.0).
- **Secondary indexes** on time-series fields including `metaField`.
- **Geospatial bucket indexes** (2dsphere_bucket — internal).
- **Sharded time-series** — out of scope (Tier 4).
- **Bucket compression** (BSON delta compression) — significant performance feature.
- **Bucket reopening** (re-using a previously closed bucket when a late insert arrives within span).
- **Extended timestamp range** (8.0).

## Extension pattern

This crosses many patterns: G (collection option), A (TTL command/background-task changes), B (`$_internalUnpackBucket` stage), and significant new infrastructure for the bucket catalog.

## Files to create (basic phase)

- `src/mongo/db/timeseries/bucket_catalog.{cpp,h}` — in-memory mapping
- `src/mongo/db/timeseries/insert_router.{cpp,h}` — intercept and route inserts
- `src/mongo/db/timeseries/query_translator.{cpp,h}` — rewrite queries
- `src/mongo/db/pipeline/document_source_internal_unpack_bucket.{cpp,h}`
- `src/mongo/db/timeseries/timeseries_options.{cpp,h}` — option parsing helpers
- New directory: `src/mongo/db/timeseries/`

## Files to modify

- `src/mongo/db/catalog/collection_options.{h,cpp}` — `TimeseriesOptions`
- `src/mongo/db/commands/create_command.cpp` — handle the new option
- `src/mongo/db/ops/insert.cpp` — intercept time-series namespace
- `src/mongo/db/query/get_executor.cpp` — handle the time-series namespace rewrite
- `src/mongo/db/ttl.cpp` — TTL on bucket-level

## Dependencies

- **Blocks:** [`pre-post-images-collection-option.md`](./pre-post-images-collection-option.md) full semantics (depends on event hooks shared with TS).
- **Blocked by:** none directly, but pairs with [`clustered-collections.md`](./clustered-collections.md) — the `system.buckets.*` collections are clustered by `_id` in stock MongoDB. EloqDoc may need basic clustering support first, depending on how `eloq_record_store` orders documents.

## EloqDoc-specific considerations

- **Storage backend.** EloqStore's record-id allocation and sort order must be inspected. Stock MongoDB clusters bucket collections so consecutive buckets are physically adjacent — important for query performance on time-range scans. Verify EloqStore can order by `_id` (likely yes, since RocksDB is its base).
- **Bucket compression** in stock MongoDB uses BSON column-format delta compression. This is a non-trivial bring-over but is a perf optimization, not correctness — defer to Tier 3 follow-up.
- **No replica set ⇒ no oplog-based bucket synchronization.** Bucket catalog rebuild on restart is simpler — read all buckets, populate the catalog from each bucket's metadata.
- **Concurrent inserts to the same bucket** must serialize. With `eloq_locker_noop`, this requires a Substrate-level mutex per bucket key. The bucket catalog can own the locks.

## Acceptance criteria (basic phase)

- `db.createCollection("m", {timeseries: {timeField: "t"}})` succeeds.
- `db.m.insertMany([{t: ISODate(), v: 1}, ...])` distributes measurements into buckets.
- `db.m.find({t: {$gte: ...}})` returns measurements with bucket-time-range pruning.
- `db.m.find({tags.host: "h1"})` queries by `metaField`.
- TTL deletes expired measurements bucket-by-bucket.
- `listCollections` shows the time-series collection plus `system.buckets.m`.
- `$_internalUnpackBucket` stage transforms bucket documents to measurement documents.
- **Test entry point:** `tests/jstests/eloq_basic/timeseries/`. Adapt `jstests/core/timeseries/`.

## Tier 2 MVP implementation notes

The current Tier 2 implementation deliberately keeps several compatibility and performance items
out of scope for the first working slice:

- `system.buckets.<collection>` is created as an ordinary collection. Clustered bucket storage is
  deferred until clustered collection support is available in this tree.
- Bucket packing is conservative. The first implementation favors correctness and simple routing
  over reopening and appending to large existing buckets.
- Logical `find` and `aggregate` operations translate to an aggregation over the bucket collection
  with `$_internalUnpackBucket`; native find executor unpacking is a later optimization.
- Query translation injects a conservative bucket-level `$match` before unpacking for a leading
  simple `$match` on the configured time field or `metaField`. The original measurement predicate
  is still evaluated after unpacking, so bucket pruning is a performance optimization rather than
  the source of query correctness.
- Measurement-level updates and deletes are explicitly rejected on logical time-series collections.
  Implementing update/delete semantics is deferred to the Tier 3 parity phase.
- TTL is bucket-level only and deletes entire buckets whose `control.max.<timeField>` is expired
  by the collection-level `expireAfterSeconds` option.
- Explain output can expose `system.buckets.<collection>` until a later compatibility pass rewrites
  explain results back to the logical time-series namespace.

## Notes from source analyses

**Source agreement.** All three analyses split this 2/3 way: `cc` and `5.5` explicitly call out "basic ⟶ Tier 2; full parity ⟶ Tier 3". `5.4` rates "Tier 3" overall but acknowledges the basic path is feasible.

**The phasing in this spec mirrors `analysis_cc/forward_compat_eval.md` C1**, which is the most concrete planning doc on time-series.

**Architectural risk:** the bucket catalog, query translator, and insert router are deeply coupled. A failed delivery wastes weeks. Consider dedicating one engineer fully for the basic phase and treating it as a project, not a casual task.
