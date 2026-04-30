# Time Series Gap Analysis: EloqDoc vs MongoDB

This document tracks the user-visible time-series API gaps that were identified during the
MongoDB data API backport work. Storage sharding is handled transparently by EloqDoc's Data
Substrate, so this analysis focuses on collection behavior, query/write semantics, and bucket
layout visible through the MongoDB API.

As of May 1, 2026, the original correctness and API-completeness gaps have been implemented on
`dev`. The remaining differences are mostly advanced MongoDB time-series parity and optimizer
coverage.

---

## Implementation Status Summary

| # | Original gap | Current status |
|---|--------------|----------------|
| 1 | Every measurement became its own bucket | Implemented. Inserts append to reusable bucket documents and create a new bucket only when no suitable bucket exists. |
| 2 | Granularity rounding constants were wrong | Implemented. Default rounding/span values follow MongoDB's seconds/minutes/hours windows. |
| 3 | Missing bucket close conditions | Implemented. Buckets close on time span, 1,000 measurements, and approximate 12 MB bucket size. |
| 4 | `control.min`/`control.max` only tracked time | Implemented. Scalar measurement fields are tracked; `_id` and the meta field are excluded from bucket data/control. |
| 5 | Measurement-field bucket predicate pushdown absent | Implemented for conservative equality/range predicates, `$and`, `$or`, and time `$in`. |
| 6 | Updates and deletes unsupported | Implemented for logical time-series namespaces through unpack/filter/repack behavior. |
| 7 | `collMod` granularity changes unsupported | Implemented for one-way granularity promotion. |
| 8 | Secondary indexes on time-series collections unsupported | Implemented with logical-to-bucket index spec translation and reverse translation for `listIndexes`. |
| 9 | `system.buckets.*` visible in `listCollections` | Implemented. Bucket collections are hidden from normal listing output. |
| 10 | Missing time-series option mutual-exclusion validation | Implemented. Granularity and custom span/rounding are mutually exclusive; custom span and rounding must match. |
| 11 | No bucket reopening after process restart | Implemented. Insert routing scans existing bucket documents when the in-memory bucket catalog has no usable entry. |
| 12 | Pipeline pushdown optimizations absent | Partially implemented. Conservative bucket pushdowns exist; advanced MongoDB optimizer rewrites remain deferred. |

---

## Implemented Behavior

### Insert And Bucket Routing

Logical time-series inserts route into `system.buckets.<collection>` through
`src/mongo/db/timeseries/insert_router.cpp`.

Implemented behavior:

- Reuses cached open buckets when possible.
- Reopens existing on-disk buckets by scanning the bucket collection when the in-memory catalog is
  empty or stale.
- Appends measurements into columnar `data.<field>.<index>` bucket columns.
- Maintains `control.count`.
- Maintains `control.min` and `control.max` for scalar measurement fields.
- Excludes `_id` and the configured `metaField` from bucket columns and control min/max.
- Uses MongoDB-like default bucket rounding/span windows:
  - `seconds`: 60 second rounding, 3,600 second max span.
  - `minutes`: 3,600 second rounding, 86,400 second max span.
  - `hours`: 86,400 second rounding, 2,592,000 second max span.
- Starts a new bucket when the existing bucket would exceed time span, 1,000 measurements, or the
  approximate 12 MB bucket size limit.

### Query Translation And Unpack

Logical `find` and `aggregate` on a time-series collection run against the bucket collection with
`$_internalUnpackBucket` inserted by `src/mongo/db/timeseries/query_translator.cpp`.

Implemented bucket-level pruning:

- Time predicates become `control.min` / `control.max` overlap predicates.
- Meta predicates become `meta` path predicates.
- Measurement equality and range predicates become conservative `control.min` / `control.max`
  predicates.
- Time `$in` becomes a bucket-level equality-range disjunction.
- `$and` keeps translatable children.
- `$or` is pushed down only when every branch can be translated safely.
- Multiple leading `$match` stages can each contribute bucket-level pushdown before unpack.
- Leading `$sort` on time/meta paths can be pushed before unpack in conservative bucket form.
- A leading `$limit` can be pushed before unpack when it is the first pipeline stage, while the
  original limit is preserved after unpack for correctness.
- A leading inclusion `$project` that references only the meta field can be pushed before unpack as
  a bucket projection over `control.count` and `meta`, while the original projection is preserved
  after unpack.

The original user pipeline stages remain after unpacking. Bucket pushdown is therefore a
performance optimization and not the source of query correctness.

### Updates And Deletes

Logical time-series updates and deletes are handled in `src/mongo/db/ops/write_ops_exec.cpp` with
helpers in `src/mongo/db/timeseries/bucket_mutation.cpp`.

Implemented behavior:

- `updateOne`, `updateMany`, `deleteOne`, and `deleteMany` operate on the logical namespace.
- Matching bucket documents are unpacked into measurements, filtered with the user predicate, and
  repacked.
- Empty buckets are deleted.
- Non-empty buckets are replaced with rebuilt `control` and `data` sections.
- The bucket catalog is invalidated for mutated buckets.

Intentional update/delete restrictions:

- Time-series updates reject upsert, hint, collation, array filters, command `let`, runtime
  constants, pipeline updates, replacement updates, and positional update operators.
- Updates cannot modify the configured meta field.
- Updates must leave the time field as a Date.
- Time-series deletes reject hint, collation, command `let`, and runtime constants.

### Collection And Index Surface

Implemented behavior:

- `createCollection` accepts `timeseries` options and creates the backing bucket collection.
- Missing bucket collections are repaired on first use.
- `collMod` supports one-way granularity promotion for non-custom-bucket time-series collections.
- `createIndexes` on a logical time-series collection translates user-visible keys to bucket
  schema keys.
- `listIndexes` on the logical collection translates bucket index specs back to logical names.
- `listCollections` hides backing `system.buckets.*` collections.
- Time-series option parsing enforces mutual exclusion between `granularity` and custom
  `bucketMaxSpanSeconds` / `bucketRoundingSeconds`.

---

## Remaining Differences From MongoDB

The original gap list is mostly closed, but EloqDoc still does not claim full MongoDB
time-series parity.

Remaining functional or semantic differences:

- Bucket collections are ordinary collections, not MongoDB clustered bucket collections.
- Bucket compression is not implemented.
- TTL is bucket-level only: EloqDoc deletes expired buckets and does not remove individual expired
  measurements from otherwise-live buckets.
- Explain output may expose the physical `system.buckets.<collection>` namespace.
- Geospatial bucket indexes are not implemented.
- Sharded time-series collection semantics are not implemented.
- Retryability, transactions, write concern, and replication edge cases follow EloqDoc/Data
  Substrate behavior rather than MongoDB replica-set internals.

Remaining performance differences:

- `$group` rewrites that answer min/max/count from `control.min` / `control.max` without unpacking
  are not implemented.
- Last-point and DISTINCT_SCAN style optimizations are not implemented.
- Meta-only `$addFields` pushdown is not implemented.
- Broader sort and index-aware time-series planning remains limited.

---

## Validation Status

Focused C++ validation has been run for the core time-series helper paths:

- `query_translator_test`
- `insert_router_test`
- `bucket_mutation_test`
- `collection_options_test`

The May 1, 2026 Gap 12 `$limit` and meta-only `$project` pushdown changes were validated with:

- `query_translator_test` (15 tests, 0 failures)
- `insert_router_test` (2 tests, 0 failures)
- `bucket_mutation_test` (5 tests, 0 failures across bucket catalog and mutation suites)
- `collection_options_test` (35 tests, 0 failures)
- `install-core` build

The tree also contains focused JS tests under `tests/jstests/eloq_basic/timeseries/`. Local JS
runtime validation is currently limited by server fixture/startup constraints in this work
environment, so the strongest routinely available validation here is focused C++ coverage plus the
`install-core` build.
