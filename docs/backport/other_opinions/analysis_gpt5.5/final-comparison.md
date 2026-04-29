# Final Cross-Version Comparison and Roadmap

## Executive Summary

A 4.0-derived server can become forward-compatible with many later client command shapes, but not with every later feature.  The easy wins are parsers, shell helpers, pure aggregation expressions, and compatibility rejections.  The hard boundary is any feature that changes storage layout, requires a new access method, or depends on an external subsystem: time-series, clustered collections, Queryable Encryption, columnstore indexes, and search/vector search.

The most useful practical target is:

1. Accept later CRUD/DDL command fields where safe.
2. Add missing index metadata options that can be represented in 4.0.
3. Implement pure aggregation/query operators.
4. Add a functional `bulkWrite` command.
5. Explicitly reject advanced storage/search/encryption features with clear errors until architecture work is funded.

## Version-by-Version API Evolution

| Version | Data API evolution | Backport posture |
| --- | --- | --- |
| 4.0 | Baseline CRUD, aggregation, classic indexes, validators, views, collation, capped collections, legacy shell helpers. | Starting point. |
| 5.0 | Stable API, IDL command normalization, `hello`, time-series create option, clustered index option, hidden/wildcard index metadata, index `commitQuorum`, more aggregation stages/expressions, `let` in commands. | Parser/helper/expression pieces are good early wins; time-series/clustered are architecture projects. |
| 6.0 | Queryable Encryption command fields and compact command, `encryptedFields`, pre/post images, date arithmetic, `$documents`, `$fill`. | Date/pipeline additions are reasonable; FLE2 and real pre/post image semantics are hard. |
| 7.0 | `bulkWrite`, `analyze`, query sampling flags, public `$densify`, `$setWindowFields`, N/top/bottom accumulators, Queryable Encryption count info. | `bulkWrite` is the best high-value command project; window/fill/densify are substantial executor work; FLE remains hard. |
| 8.0 | Search/vector/search-index commands and stages, query settings, mature `bulkWrite`, bit/percentile operators, columnstore/prepareUnique index options, FLE cleanup. | Implement bit ops and `bulkWrite` refinements; reject or stub search/vector/queryable encryption/columnstore unless doing major architecture. |

## Differences and Gaps: 8.0 vs 6.0

Data-facing commands in 8.0 that are absent from 6.0:

- `bulkWrite` (introduced in 7.0).
- `analyze` (introduced in 7.0).
- `getQueryableEncryptionCountInfo` (introduced in 7.0).
- `cleanupStructuredEncryptionData` (8.0).
- Search index commands: `createSearchIndexes`, `dropSearchIndex`, `updateSearchIndex`, `listSearchIndexes`.
- Query settings: `setQuerySettings`, `removeQuerySettings`.

Important field/semantic gaps:

- `bulkWrite` 8.0 adds `let`, `errorsOnly`, stable reply counters, and richer cursor/error behavior.
- `find`/`aggregate`/`distinct` can carry internal `querySettings`.
- `distinct` adds `hint`.
- `compactStructuredEncryptionData` grows `anchorPaddingFactor`, `validator`, and `encryptionInformation`.
- `createIndexes` supports newer index spec fields such as `columnstoreProjection`, `columnstoreCompressor`, `clustered`, and `prepareUnique`.
- Aggregation adds search/vector stages, bit expressions, and percentile/median accumulators.

## Differences and Gaps: 8.0 vs 4.0

The largest gaps from a 4.0 start are:

- Command framework: 8.0 expects typed commands, IDL parsing, API stability metadata, structured auth checks, and strict-field behavior.
- Data models: time-series, clustered collections, Queryable Encryption metadata/state collections, and columnstore indexes are not simple 4.0 catalog flags.
- Query execution: 8.0 has more aggregation stages, SBE-era planner/execution assumptions, query settings, sampling, and search/vector integration.
- Index lifecycle: hidden indexes, commit quorum, resumable/two-phase build behavior, `prepareUnique`, columnstore, and time-series index rewriting.
- Client surface: old shell helpers exist in the server repo, and modern `mongosh` exists as sibling checkout `../mongosh`.  Current `mongosh` is Node-driver-backed, so server compatibility must account for the commands and options emitted by the driver as well as shell method names.

## Recommended Implementation Plan for 4.0 Forward Compatibility

### Phase 1: Safe command-shape compatibility

- Add compatibility parsing for modern fields on existing commands: `let`, `runtimeConstants`, `comment`, `hint` where possible, API version parameters, internal sampling fields, `collectionUUID`, `isTimeseriesNamespace`.
- For unsupported advanced features, return explicit `CommandNotSupported` or `InvalidOptions` errors.  Do not silently create unusable catalog metadata.
- Add or update legacy shell helpers only if the 4.0-based distribution still ships the legacy shell.  For modern `mongosh`, focus on server behavior because helpers call the Node driver.

### Phase 2: Pure computation APIs

- Add simple aggregation stage aliases: `$set`, `$unset`, `$replaceWith`, `$count`.
- Add pure expressions from 5.0/6.0/8.0: conversions, regex expression family, string trim/replace, trigonometry, date arithmetic, bit expressions.
- Add contained stages like `$documents`.

### Phase 3: Index metadata and planner-contained work

- Implement hidden indexes with planner exclusion and plan cache invalidation.
- Add wildcard index support only if the team is ready for key generation and planner work.
- Parse `commitQuorum` and `returnOnStart`; implement exact behavior only if replica-set index-build semantics matter.
- Consider `prepareUnique` only after duplicate detection and catalog state transitions are designed.

### Phase 4: High-value command addition

- Implement client-level/cross-namespace `bulkWrite` using existing insert/update/delete execution paths.  `mongosh` collection-level `db.collection.bulkWrite()` can still work through driver legacy batch writes, but `db.getMongo().bulkWrite()` is marked 8.0+ and needs the server command.
- Support `ops` variants (`insert`, `update`, `delete`), `nsInfo`, `ordered`, `cursor.batchSize`, `errorsOnly`, `let`, `bypassDocumentValidation`, statement ids, and reply counters.
- Enforce auth per namespace/action and preserve write error ordering.

### Phase 5: Expensive optional executor features

- `$unionWith`, `$merge`, `$fill`, `$densify`, `$setWindowFields`, top/bottom/N, median/percentile.
- These are feasible without a new storage engine, but require careful executor, memory, sort, spill, and semantics work.

### Phase 6: Architecture projects

Treat these as separate product efforts:

- Time-series collections.
- Clustered collections.
- Queryable Encryption/FLE2.
- Columnstore indexes.
- Search/vector search and search index commands.
- Full stable API enforcement.
- Query settings with planner enforcement.

## Final Classification Rollup

Very easily doable:

- Parser acceptance for harmless fields, `hello`, legacy shell helper wrappers, simple aggregation aliases, pure expressions including bit operators, `$documents`, basic metadata command compatibility.

Needs a lot of development:

- `bulkWrite`, hidden indexes, wildcard indexes, update/delete hints, command `let` propagation, `$unionWith`, `$merge`, `$fill`, `$densify`, `$setWindowFields`, top/bottom/N, median/percentile, `analyze`, query settings, pre/post image semantics, `prepareUnique`.

Very hard to do:

- Time-series, clustered collections, Queryable Encryption, columnstore indexes, search/vector search, exact Stable API enforcement across the full command framework.

Ignore for now:

- Sharding/resharding/migration/global-index internals, replication/transaction-specific commands, tenant/serverless commands, test-only commands, diagnostics unless required by tooling.
