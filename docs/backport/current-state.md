# Current Backport State

This document describes the current EloqDoc MongoDB API backport state on the
`backport/tier1-phase1a` branch as of April 29, 2026. It is intended for users and engineers who
need to know what works today, where EloqDoc intentionally differs from MongoDB, and which MongoDB
5.0-8.0 features remain incomplete.

The backport scope is the MongoDB data API surface: commands, aggregation stages and expressions,
index options, collection options, and data-model features. Replication, sharding control-plane
semantics, Atlas-only services, and operational telemetry are not in scope for this branch.

## Validation Status

The branch builds with:

```bash
env WITH_DATA_STORE=ELOQDSS_ROCKSDB python scripts/buildscripts/scons.py \
  MONGO_VERSION=4.0.3 VARIANT_DIR=RelWithDebInfo \
  CXXFLAGS="-Wno-nonnull -Wno-class-memaccess -Wno-interference-size -Wno-redundant-move" \
  --build-dir=#build --prefix=/home/lintaoz/eloq/bin --disable-warnings-as-errors -j16 install-core
```

Focused C++ unit coverage has been run for the time-series query translator and collection option
parser. The tree also contains focused JS tests under `tests/jstests/eloq_basic/` for the backported
features. Local JS runtime validation is currently limited by server fixture/startup issues in this
work environment, so the strongest routinely available validation here is C++ unit coverage plus the
`install-core` build.

## High-Level Compatibility Summary

| Area | Current status |
| ---- | -------------- |
| Command envelope compatibility | Implemented permissive parsing for modern driver fields and Stable API fields. Semantics are intentionally partial. |
| Aggregation expressions | Broad Tier 1 expression coverage is implemented, with documented edge-case differences for date arithmetic, Decimal128 trig precision, and JavaScript operators. |
| Aggregation stages | `$documents`, `$planCacheStats`, `$listCatalog`, `$unionWith`, `$merge`, `$fill`, `$densify`, and `$setWindowFields` are implemented to useful compatibility levels. Some remain bounded-memory or semantic subsets. |
| Write syntax and commands | `bulkWrite`, update/delete hints, command `let`, and pipeline updates are implemented. Advanced MongoDB retryability/transaction/write-concern edge cases are not fully matched. |
| Index features | Hidden indexes, basic wildcard indexes, wildcard projection, simple wildcard planning, and `prepareUnique` enforcement are implemented. Compound wildcard and full index-build quorum semantics are not. |
| Time-series collections | Basic create/insert/find/aggregate/unpack/TTL support is implemented. Full MongoDB time-series parity is not. |
| Security/encryption/search | Queryable Encryption/FLE2, OIDC auth, Atlas Search/vector search, and query settings are not implemented. |
| Sharding/replication control surface | Deferred. EloqDoc relies on Data Substrate rather than MongoDB replica-set/sharding internals. |

## Implemented Items With MongoDB-Parity-Like Behavior

The following items are present and should behave close enough to MongoDB for ordinary client
compatibility. Edge cases may still differ because EloqDoc is based on MongoDB 4.0.3 internals.

- Aggregation aliases: `$set`, `$unset`, and `$replaceWith`.
- `$documents` aggregation source.
- `$count` accumulator.
- Field-access expressions: `$getField`, `$setField`, `$unsetField`.
- Regex and string replacement expressions: `$regexMatch`, `$regexFind`, `$regexFindAll`,
  `$replaceOne`, `$replaceAll`.
- Bitwise expressions: `$bitAnd`, `$bitOr`, `$bitXor`, `$bitNot`.
- Rounding/truncation expressions: `$round` and two-argument `$trunc`.
- Trigonometric expressions.
- Array N-value expressions: `$firstN`, `$lastN`.
- Distinct command `hint`.
- Update/delete command `hint`, including coverage for hinted `_id` write plans.
- Pipeline-form updates using the safe update-stage subset.
- Hidden indexes, including planner exclusion.
- `$unionWith` including nested pipeline support and authorization checks.
- `$merge` including cross-database targets, dotted `on` fields, `let`, pipeline `whenMatched`,
  authorization checks, and streaming write behavior.

## Command Envelope And Compatibility Stubs

### Stable API Fields

EloqDoc accepts `apiVersion`, `apiStrict`, and `apiDeprecationErrors` so modern drivers can send
Stable API envelopes without command rejection.

Semantic difference: MongoDB uses these fields to enforce API version contracts and deprecation
errors. EloqDoc currently strips and ignores them. A command that MongoDB would reject under
`apiStrict: true` may still run in EloqDoc.

### Common Modern Command Fields

EloqDoc accepts and strips several modern command-envelope fields used by newer drivers, including
fields whose purpose is routing, telemetry, or namespace verification.

Important difference: MongoDB's `collectionUUID` can verify that a namespace still refers to the
expected collection. EloqDoc currently treats it as a compatibility envelope field rather than a
catalog assertion. This is acceptable for basic driver compatibility but is not a substitute for
MongoDB's stale-namespace protection during concurrent rename/drop workflows.

### `commitQuorum`

`createIndexes` accepts `commitQuorum`.

Semantic difference: MongoDB uses `commitQuorum` for replicated two-phase index builds. EloqDoc has
no MongoDB replica-set coordination layer, so the field is accepted and ignored. Indexes commit with
EloqDoc's local/Data Substrate semantics.

### `rotateCertificates`

The command exists as a no-op compatibility stub. It requires the same broad operational privilege
class used by existing host management commands and returns a result indicating no TLS reload was
performed.

Semantic difference: MongoDB reloads TLS certificate material for future connections. EloqDoc does
not currently reload TLS contexts through this command.

### `hello`

Modern `hello` command compatibility is present for clients that no longer prefer `isMaster`.
This is a command-surface compatibility feature, not a replication-protocol implementation.

## Aggregation Expressions

### Date Arithmetic

Implemented: `$dateAdd`, `$dateSubtract`, `$dateDiff`, and `$dateTrunc`.

Known differences:

- Fixed units use elapsed UTC milliseconds.
- Month/quarter/year handling uses calendar-part differences and truncation approximations rather
  than MongoDB's full timezone/DST boundary semantics.
- Edge cases around DST, partial months, negative ranges, large `binSize`, and `startOfWeek` may
  differ from MongoDB.

These operators are suitable for common UTC and fixed-duration use cases. Applications that depend
on exact MongoDB calendar-boundary behavior should validate those cases explicitly.

### Decimal128 Math

Rounding and truncation are intended to match MongoDB numerically. Decimal128 display cohorts may
serialize differently in some zero-precision cases, for example `NumberDecimal("2")` instead of
`NumberDecimal("2.0")`.

Trigonometric expressions preserve Decimal128 result type for Decimal128 input, but compute through
`double` math and convert back. MongoDB's newer implementation can provide higher-precision
Decimal128 transcendental results.

### JavaScript Aggregation Operators

`$function` and custom `$accumulator` syntax are accepted as compatibility stubs but do not execute
server-side JavaScript. Runtime evaluation raises a server-side scripting disabled error.

Semantic difference: MongoDB can execute these operators when server-side scripting is enabled.
EloqDoc intentionally does not provide that execution path in this branch.

### Percentile And Median Expressions

`$percentile` and `$median` are implemented as expressions and accumulators for numeric input using
exact sorted interpolation over available values.

Semantic difference: MongoDB uses t-digest style approximate machinery for some modes and can merge
partial digests in distributed plans. EloqDoc currently favors exact local computation for bounded
sets and does not implement distributed partial digest merge semantics.

## Aggregation Stages

### `$planCacheStats`

Implemented as a compatibility stage over EloqDoc's existing plan cache.

Semantic difference: MongoDB reports upstream `queryHash` values. EloqDoc derives a deterministic
hash from the local `planCacheKey`; it is useful for grouping local entries but is not byte-for-byte
comparable with MongoDB's `queryHash`.

### `$listCatalog`

Implemented for database-local catalog introspection. It returns collection catalog documents for
the current database.

This is a compatibility stage, not a full administrative inventory subsystem.

### `$fill`

Implemented:

- Literal `value` fill.
- `method: "locf"`.
- Numeric `method: "linear"`.
- Date-sort linear interpolation.
- Partition fields and partition expressions.
- Sort-order validation.

Known differences:

- MongoDB supports broader bounded/spilling behavior for linear interpolation. EloqDoc's linear
  implementation uses a buffered compatibility path and is not a fully memory-bounded streaming
  implementation.
- Linear interpolation is limited to numeric/date sort keys and supported value shapes covered by
  local tests.

### `$densify`

Implemented:

- Numeric densification.
- Partitioned densification.
- String bounds.
- Date densification for fixed-duration units: millisecond, second, minute, hour, day, and week.

Known differences:

- Calendar-aware date units such as month, quarter, and year remain unsupported.
- MongoDB's DST-sensitive behavior for calendar units is not implemented.

### `$setWindowFields`

Implemented:

- Document windows.
- Numeric range windows.
- Fixed-duration date range units for millisecond, second, minute, hour, day, and week.
- Position operators.
- `$expMovingAvg`.
- `$locf`.
- `$covariancePop` and `$covarianceSamp`.
- `$integral` and `$derivative`, including fixed date units.
- N-value window operators.
- `$percentile` and `$median`.
- `$top`, `$topN`, `$bottom`, and `$bottomN`.

Known differences:

- The implementation buffers input partitions and is not equivalent to MongoDB's optimized window
  executor for large partitions.
- Calendar date range units such as month, quarter, and year remain unsupported.
- Some operators skip malformed/non-numeric inputs according to the local compatibility
  implementation; exact MongoDB edge-case behavior should be verified before relying on unusual
  input types.

### `$merge`

Implemented:

- Current-db and cross-db `into` targets.
- Dotted `on` fields.
- `let`.
- Pipeline `whenMatched`.
- Authorization checks.
- Streaming writes for large result sets.

Known differences and caveats:

- MongoDB requires careful unique-key enforcement and retry/write-concern semantics around
  `whenMatched`/`whenNotMatched`. EloqDoc routes through local write paths and Data Substrate
  transaction behavior; it is not guaranteed to match every MongoDB retryable-write or distributed
  write-concern edge case.
- Earlier crash investigations around `$merge` should be treated as resolved only for the covered
  streaming/write-path tests. Very large workloads and unusual target/index combinations deserve
  additional stress testing.

### `$unionWith`

Implemented with nested pipeline execution and authorization checks. It should be suitable for
typical same-server union pipelines.

Gaps are mostly around exotic nested command options, distributed execution, and interactions with
features EloqDoc does not implement, such as sharded routing.

## Write Commands And Syntax

### `bulkWrite`

Implemented:

- Basic MongoDB 7.0-style command surface.
- Insert/update/delete operation dispatch through existing CRUD execution paths.
- Cursor-style result pagination.
- Per-op hint coverage.

Known differences:

- The command does not make the whole batch a single MongoDB-style atomic unit outside explicit
  user transaction semantics.
- Advanced retryable-write, write-concern, transaction, and 8.0 extras should be treated as partial
  unless explicitly covered by tests.

### Command `let` And Runtime Constants

Implemented command-level `let` plumbing for modern command shapes.

Semantic difference: MongoDB exposes selected runtime constants such as stable request-time `$$NOW`.
EloqDoc accepts the `runtimeConstants` envelope but does not expose every MongoDB internal runtime
constant with identical behavior.

### Pipeline Updates

Pipeline-form updates are implemented with a conservative allowed-stage list:

- `$set` / `$addFields`
- `$unset` / `$project`
- `$replaceWith` / `$replaceRoot`

Stages such as `$lookup`, `$group`, `$merge`, and `$out` are rejected in update context. This is
intentional and follows MongoDB's documented update-pipeline restrictions.

## Index Features

### Hidden Indexes

Implemented:

- Index catalog `hidden` flag.
- Planner exclusion of hidden indexes.
- Hide/unhide behavior through command paths covered by local tests.

Operational caveat: with EloqDoc's no-op locker model, catalog visibility must be treated carefully
for concurrent hide/unhide plus query planning. Existing implementation is designed for stable
catalog reads, but high-concurrency workloads should be tested.

### Wildcard Indexes

Implemented:

- Basic `$**` wildcard index storage.
- `wildcardProjection`.
- Simple predicate planning through wildcard indexes.
- Projected wildcard planner coverage.

Current model:

- Wildcard keys are represented internally with path/value style entries.
- Planner support is conservative and targets simple predicate shapes.

Not implemented:

- Compound wildcard indexes.
- Full MongoDB wildcard planner reasoning for complex boolean predicates, multikey interactions,
  collation, covered projection, and all index intersection cases.

### `prepareUnique`

Implemented:

- Catalog state for prepared unique indexes.
- Write-path duplicate enforcement for prepared unique indexes.

Partial semantics:

- Full conversion workflow from non-unique to unique remains incomplete.
- Existing duplicate cleanup, catalog transition lifecycle, and storage-side validation are not at
  full MongoDB parity.

## Collection Options

### Pre/Post Image Collection Option

Implemented as catalog metadata for client compatibility.

Semantic difference: MongoDB can store pre/post images and expose them through change streams.
EloqDoc currently does not capture pre/post images, does not populate `config.system.preimages`, and
does not alter change-stream output for this option. Full semantics require a future event/preimage
storage subsystem.

## Time-Series Collections

Time-series support is implemented as a Tier 2-compatible feature set with conservative bucket
packing, logical reads/writes, index translation, and targeted bucket-level query pruning. Full
MongoDB time-series parity is still not claimed.

Implemented:

- `createCollection` accepts `timeseries` with `timeField`, optional `metaField`, `granularity`,
  `bucketMaxSpanSeconds`, and `bucketRoundingSeconds`.
- Collection-level `expireAfterSeconds` is accepted for time-series collections.
- `system.buckets.<collection>` is created as the physical bucket collection.
- Missing bucket collections can be repaired on first use.
- Inserts into the logical namespace route into reusable columnar bucket documents.
- `$_internalUnpackBucket` unpacks bucket documents into measurement documents.
- `find` and `aggregate` on the logical namespace translate to aggregation on the bucket
  collection with `$_internalUnpackBucket`.
- Bucket-level TTL deletes buckets whose `control.max.<timeField>` is expired.
- Logical `updateOne`, `updateMany`, `deleteOne`, and `deleteMany` are supported by unpacking
  matching buckets, filtering/updating measurements, and repacking survivors.
- `collMod` supports one-way granularity promotion for time-series collections that do not use
  custom bucket span/rounding.
- `createIndexes` and `listIndexes` on logical time-series collections translate between logical
  user key patterns and bucket key patterns.
- `listCollections` hides backing `system.buckets.<collection>` collections.

Bucket layout:

- Bucket collections are ordinary collections, not clustered collections.
- Buckets store `control`, optional `meta`, and columnar `data` fields.
- Inserts reuse cached open buckets and can reopen existing bucket documents after process restart.
- Buckets close when the measurement is outside the configured time span, the bucket reaches 1,000
  measurements, or the resulting bucket document would exceed the approximate 12 MB threshold.
- `control.min` and `control.max` track scalar measurement fields. `_id` and the configured
  `metaField` are not stored as bucket data columns and are not tracked in control min/max.
- Current packing is conservative and favors correctness over MongoDB's mature high-density bucket
  compression behavior.
- `bucketRoundingSeconds`, when supplied, drives insert-time bucket rounding. Without custom
  rounding, MongoDB-like default rounding/span windows are used for `seconds`, `minutes`, and
  `hours` granularity.

Query pruning:

- Conservative bucket-level `$match` stages may be inserted before unpacking.
- Time predicates are translated to `control.min` / `control.max` overlap checks.
- Meta predicates are translated to `meta` paths.
- Supported measurement equality/range predicates are translated to conservative `control.min` /
  `control.max` checks.
- Date `$in` on the time field translates to bucket-level equality-range disjunctions.
- `$and` keeps translatable children.
- `$or` is pushed down only when every branch has a bucket-level translation.
- Multiple leading `$match` stages can contribute bucket-level predicates.
- Leading `$sort` on time/meta paths may be pushed before unpacking while preserving the original
  user sort after unpack.
- A leading `$limit` can be pushed before unpacking when it is the first stage, with the original
  limit still applied after unpack.
- A leading simple inclusion `$project` over time, meta, and measurement fields can be pushed before
  unpacking as a bucket projection that keeps `control.count`, `meta`, and selected `data.<field>`
  paths.
- A leading meta-only `$addFields` / `$set` with literal values can be pushed before unpacking as a
  bucket stage over `meta` paths.
- A single-stage whole-collection `$group` can be answered without unpacking when it only computes
  count and time/measurement min/max from bucket `control` fields.
- The original measurement-level predicate remains after unpacking, so bucket pruning is a
  performance optimization rather than the source of correctness.

Update/delete restrictions:

- Time-series updates reject upsert, hint, collation, array filters, command `let`, runtime
  constants, pipeline updates, replacement updates, and positional update operators.
- Time-series updates cannot modify the configured meta field and must leave the time field as a
  Date.
- Time-series deletes reject hint, collation, command `let`, and runtime constants.

Important differences from MongoDB:

- Logical time-series collections are represented by collection options, not by MongoDB's full
  view-based catalog model.
- Bucket collections are ordinary collections; clustered bucket storage is deferred.
- Bucket compression is not implemented.
- Geospatial bucket indexes and sharded time-series are not implemented.
- Explain output may expose `system.buckets.<collection>` rather than rewriting everything back to
  the logical namespace.
- TTL is bucket-level only; EloqDoc deletes entire expired buckets and does not delete individual
  expired measurements from otherwise-live buckets.
- Advanced MongoDB optimizer rewrites such as grouped `$group` min/max/count from bucket control
  fields, meta-field or computed `$group` accumulators, last-point DISTINCT_SCAN, computed
  `$addFields` pushdown, and broader index-aware planning are not implemented.

## Analyze Command

Implemented as a basic statistics collection command with dotted key support.

Caveat: the optimizer does not yet consume the collected statistics for full MongoDB-style
cost-based planning. The command is useful for compatibility and local statistics inspection but is
not the complete optimizer feature.

## `find.allowDiskUse`

Implemented for blocking sort spill.

Known limitation: the implementation can externalize sort keys and working-set ids, but still keeps
matching `WorkingSetMember`s in memory until they are returned. It reduces sort-key memory pressure
but is not equivalent to a fully externalized MongoDB query executor for arbitrarily large result
sets.

## Not Implemented Or Deliberately Deferred

The following are not implemented in this branch, or exist only as parser/catalog stubs:

- Full Stable API enforcement.
- Full `collectionUUID` namespace assertion semantics.
- Full replicated `commitQuorum` index-build semantics.
- Full TLS certificate reload for `rotateCertificates`.
- Full pre/post image capture and change-stream integration.
- Queryable Encryption / FLE2.
- OIDC authentication.
- Atlas Search, vector search, `$rankFusion`, and `$scoreFusion`.
- Query settings.
- Clustered collections.
- Compound wildcard indexes.
- Columnstore indexes. These are explicitly low-priority because MongoDB deprecated columnstore
  indexes in 8.0.
- MongoDB sharding command semantics.
- MongoDB replica-set coordination semantics.
- Oplog-dependent features that do not map directly to Data Substrate.
- Geospatial bucket indexes for time-series.
- Full time-series parity, including compression, clustered bucket storage, geospatial bucket
  indexes, advanced time-series aggregation rewrites, and sharded time-series.

## Practical Guidance For Users

Use the current branch for:

- Modern driver compatibility where command envelopes previously caused hard failures.
- Applications using common 5.0-7.0 aggregation expressions and stages.
- Basic `bulkWrite`, `$merge`, `$unionWith`, window analytics, `$fill`, and `$densify` workflows.
- Basic time-series ingestion, querying, updates/deletes, and logical secondary indexes within the
  documented restrictions.
- Hidden indexes and simple wildcard index use cases.

Avoid assuming full MongoDB parity for:

- Security/encryption/search features.
- Sharding/replication behavior.
- Very large analytic windows or interpolation workloads.
- Exact date arithmetic across DST/month/quarter/year boundaries.
- Time-series workloads that rely on compression, clustered bucket storage, geospatial bucket
  indexes, sharded time-series, advanced aggregation rewrites, or exact MongoDB retryable-write
  semantics.
- Workloads depending on exact MongoDB retryable-write or write-concern edge cases.

When in doubt, check the focused task spec under `docs/backport/tasks/` and add an EloqDoc-specific
test under `tests/jstests/eloq_basic/` before relying on an edge case in production.
