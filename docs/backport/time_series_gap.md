# Time Series Gap Analysis: EloqDoc vs MongoDB

This document compares EloqDoc's current time series implementation against MongoDB's reference
implementation and identifies the functional gaps. Storage sharding is handled transparently by
EloqDoc's DataSubstrate, so the analysis focuses purely on the user-visible API surface.

---

## Gap 1 — Write path: every measurement becomes its own bucket (CRITICAL)

**What EloqDoc does:**
`getOrCreateBucket()` in `src/mongo/db/timeseries/bucket_catalog.cpp` generates a new OID
whenever `count > 0`, then `routeInsert()` in `src/mongo/db/timeseries/insert_router.cpp` issues
a fresh `INSERT` for every measurement. Result: 1000 measurements → 1000 bucket documents, each
with `control.count=1`.

**What MongoDB does:**
It `UPDATE`s an open bucket document to append the measurement's values into `data.<field>` arrays,
incrementing `control.count` and updating `control.min`/`control.max`. A new bucket is created only
when the current one is full (1000 measurements, ~16 MB, or time span exhausted).

**Why it matters:**
The bucket format is supposed to be columnar and compact. Running 1:1 defeats the entire
purpose — storage is 10–100× larger, query scans touch orders of magnitude more documents, and
the unpack stage emits one measurement per bucket.

**Fix required:**
The insert path needs to:
1. Query `system.buckets.*` for an open bucket matching
   `{meta: X, "control.min.<timeField>": {$lte: roundedMax}}` that still has room.
2. If found, `UPDATE` it: append values to `data.<field>` arrays, increment `control.count`,
   update `control.min`/`control.max`.
3. If not found, `INSERT` a new bucket document.

---

## Gap 2 — Granularity rounding constants are wrong (IMPORTANT)

**What EloqDoc does** (`src/mongo/db/timeseries/insert_router.cpp`):

| Granularity | EloqDoc rounding |
|-------------|-----------------|
| seconds     | 1 s             |
| minutes     | 60 s            |
| hours       | 3 600 s         |

**What MongoDB uses:**

| Granularity | Rounding (bucketRoundingSeconds) | Max span (bucketMaxSpanSeconds) |
|-------------|----------------------------------|----------------------------------|
| seconds     | 60 s (1 minute boundary)         | 3 600 s (1 hour)                 |
| minutes     | 3 600 s (1 hour boundary)        | 86 400 s (1 day)                 |
| hours       | 86 400 s (1 day boundary)        | 2 592 000 s (30 days)            |

EloqDoc's rounding is 60× too fine at every level. For "seconds" granularity, EloqDoc creates
distinct bucket time windows every 1 second; MongoDB does so every 1 minute. This produces far more
bucket keys and prevents consolidation of measurements within the same minute.

---

## Gap 3 — No bucket closing conditions (IMPORTANT)

MongoDB closes a bucket and opens a new one when any of these triggers fire:
- New measurement's timestamp exceeds `control.min.<timeField> + bucketMaxSpanSeconds`.
- Bucket reaches 1 000 measurements.
- Bucket document would exceed ~12 MB.

EloqDoc has none of this logic. Since gap 1 means every measurement is its own bucket today, this
is currently moot, but once gap 1 is fixed, missing close conditions will allow unbounded bucket
growth.

---

## Gap 4 — `control.min`/`control.max` only track the time field (IMPORTANT)

**What EloqDoc does** (`src/mongo/db/timeseries/insert_router.cpp`):
`control.min` and `control.max` contain only `{<timeField>: <value>}`.

**What MongoDB does:**
`control.min` and `control.max` track the minimum and maximum of *every* scalar measurement field
in the bucket. For a measurement `{t: ..., temp: 22.5, host: "a"}` the bucket's control block
records `{min: {t: ..., temp: 22.5}, max: {t: ..., temp: 22.5}}`.

**Why it matters:**
Without full control.min/max, bucket-level predicate pushdown in `query_translator.cpp` can only
prune on time. A query like `{temp: {$gt: 30}}` must unpack every bucket. This also blocks gap 5.

---

## Gap 5 — Query pushdown only covers time and meta, not measurement fields (MODERATE)

**What EloqDoc does** (`src/mongo/db/timeseries/query_translator.cpp`):
Translates predicates on `timeField` → `control.min/max.<timeField>` and
`metaField` → `meta.*`. All other predicates are dropped; no bucket-level filter is generated,
so those fields require a full bucket scan.

**What MongoDB does:**
For any measurement field `f` with a range predicate like `{f: {$gt: V}}`, MongoDB adds
`{"control.max.f": {$gt: V}}` to the bucket match (a bucket can only be skipped if its max value
for `f` is ≤ V). Requires gap 4 to be filled first.

---

## Gap 6 — Updates and deletes not supported (MISSING)

`updateOne`, `updateMany`, `deleteOne`, `deleteMany` against the logical timeseries namespace are
not intercepted. MongoDB (5.1+) supports these by unpacking matching bucket(s), filtering
measurements, repacking survivors back into the bucket, and deleting the bucket if it becomes
empty.

---

## Gap 7 — `collMod` for granularity changes not implemented (MISSING)

`db.runCommand({collMod: "coll", timeseries: {granularity: "minutes"}})` is not handled. MongoDB
allows one-way granularity promotion (seconds → minutes → hours), which widens the bucket time
window for new inserts without rewriting existing buckets. Existing buckets are left as-is.

---

## Gap 8 — No secondary indexes on timeseries collections (MISSING)

`db.ts.createIndex({host: 1, t: 1})` is not intercepted or translated. MongoDB transparently maps
user-visible index specs to the underlying `system.buckets.*` collection's schema:
- `metaField` path → `meta.*`
- `timeField` → `data.<timeField>`
- measurement field `f` → `data.f`

`listIndexes` on the logical collection returns the translated-back user schema, hiding internal
bucket schema details.

---

## Gap 9 — `listCollections` exposes `system.buckets.*` collections (INCOMPLETE)

`src/mongo/db/commands/list_collections.cpp` has no filtering for `system.buckets.*` namespaces.
`db.getCollectionNames()` returns both `metrics` and `system.buckets.metrics`. MongoDB hides the
bucket collections from `listCollections` output.

---

## Gap 10 — Missing mutual-exclusion validation on timeseries options (MINOR)

`src/mongo/db/timeseries/timeseries_options.cpp` accepts `bucketMaxSpanSeconds` and
`bucketRoundingSeconds` alongside `granularity` without rejecting the combination. MongoDB
enforces:
- `granularity` and custom `bucketMaxSpanSeconds`/`bucketRoundingSeconds` are mutually exclusive.
- When using custom span/rounding (without granularity), `bucketRoundingSeconds` must equal
  `bucketMaxSpanSeconds`.

---

## Gap 11 — No bucket reopening after process restart (MISSING, lower priority)

After a process restart the in-memory `BucketCatalog` is empty. Subsequent inserts for a
`(meta, time)` key that already has an on-disk open bucket will create duplicate buckets instead
of appending. The fix is query-based reopening: before inserting, query `system.buckets.*` for an
existing open bucket — the same mechanism needed for gap 1.

---

## Gap 12 — Pipeline pushdown optimizations absent (PERFORMANCE)

MongoDB has 12+ optimization passes on timeseries aggregation pipelines: sort pushdown before
unpack, `$group` rewrite to use `control.min`/`control.max` (eliminates unpacking entirely for
min/max/count queries), last-point DISTINCT_SCAN, `$limit` pushdown, `$project`/`$addFields`
pushdown for meta-only fields, and others. EloqDoc only prepends a bucket-level time+meta `$match`.
These are pure performance optimizations; correctness is preserved by the unpack stage regardless.

---

## Priority Summary

| # | Gap | Impact |
|---|-----|--------|
| 1 | New bucket per measurement instead of UPDATE | Correctness + storage |
| 2 | Granularity rounding values 60× too fine | Correctness |
| 3 | No bucket closing (max count / span / size) | Correctness once #1 fixed |
| 4 | `control.min`/`control.max` only has timeField | Query correctness / perf |
| 5 | No measurement-field bucket predicate pushdown | Query performance |
| 6 | No updates/deletes on timeseries collections | API completeness |
| 7 | No `collMod` for granularity changes | API completeness |
| 8 | No secondary indexes on timeseries collections | API completeness + perf |
| 9 | `system.buckets.*` visible in `listCollections` | API correctness |
| 10 | Missing mutual-exclusion validation on options | Minor validation |
| 11 | No bucket reopening after process restart | Correctness after restart |
| 12 | Missing pipeline pushdown optimizations | Performance only |

Gaps 1–3 are the write-path core and should be addressed together. Fix gap 1 (bucket accumulation
via UPDATE) and gap 2 (rounding constants) first; gap 3 follows naturally once buckets hold
multiple measurements. Gaps 4–5 are a read-path pair — fix 4 first, then 5. Gaps 6–9 are
independent API surface features that can be tackled in any order.
