# Curated Data Command Matrix

This matrix corrects for older hand-written command registration and filters out sharding, replication, migration, tenant/serverless, diagnostics, and test-only commands.  `yes` means the command exists as a user-facing data command in that version branch.  `new` marks the first version in this branch set where the command becomes relevant.

| Command | 4.0 | 5.0 | 6.0 | 7.0 | 8.0 | Backport note |
| --- | --- | --- | --- | --- | --- | --- |
| `find` | yes | yes | yes | yes | yes | Later versions add `allowDiskUse`, `let`, `collectionUUID`, `encryptionInformation`, `sampleId`, `querySettings`. |
| `getMore` | yes | yes | yes | yes | yes | Mostly cursor semantics; bulkWrite adds a command cursor namespace in 7/8. |
| `killCursors` | yes | yes | yes | yes | yes | Stable baseline. |
| `insert` | yes | yes | yes | yes | yes | Later shared fields include `collectionUUID`, `isTimeseriesNamespace`, `encryptionInformation`, `bypassEmptyTsReplacement`. |
| `update` | yes | yes | yes | yes | yes | Later adds top-level `let`, `runtimeConstants`, update `hint`, `sort`, sample/internal shard-key flags. |
| `delete` | yes | yes | yes | yes | yes | Later adds top-level `let`, `runtimeConstants`, delete `hint`, sample fields. |
| `findAndModify` | yes | yes | yes | yes | yes | Later adds/IDL-normalizes `hint`, `let`, `stmtId`, `encryptionInformation`, sample/time-series fields. |
| `bulkWrite` | no | no | no | new | yes | High-value 4.0 backport: command fan-out to existing CRUD with cursor reply. |
| `count` | yes | yes | yes | yes | yes | Later adds `hint`, encryption/sample fields. |
| `distinct` | yes | yes | yes | yes | yes | 8.0 adds `hint` and internal query settings. |
| `aggregate` | yes | yes | yes | yes | yes | Most evolution is stage/expression support. |
| `explain` | yes | yes | yes | yes | yes | Needs to understand any newly accepted command shape. |
| `mapReduce` | yes | yes | yes | yes | yes | Legacy; not a forward-compat priority. |
| `create` | yes | yes | yes | yes | yes | Later adds `timeseries`, `clusteredIndex`, `changeStreamPreAndPostImages`, `encryptedFields`, `recordIdsReplicated`. |
| `drop` | yes | yes | yes | yes | yes | Stable baseline. |
| `dropDatabase` | yes | yes | yes | yes | yes | Stable baseline, ignoring sharding/system behavior. |
| `renameCollection` | yes | yes | yes | yes | yes | Later sharded variants ignored. |
| `collMod` | yes | yes | yes | yes | yes | Later index hide/unhide, TTL, pre-images, time-series, unique conversion options. |
| `convertToCapped` | yes | yes | yes | yes | yes | Stable baseline; 8.0 sharded participant commands ignored. |
| `createIndexes` | yes | yes | yes | yes | yes | Later hidden, wildcard, commitQuorum, time-series, columnstore, prepareUnique, returnOnStart. |
| `dropIndexes` | yes | yes | yes | yes | yes | Stable baseline; sharded catalog internals ignored. |
| `listIndexes` | yes | yes | yes | yes | yes | Later `includeBuildUUIDs`, `includeIndexBuildInfo`, time-series rewrite flag. |
| `reIndex` | yes | yes | yes | yes | yes | Legacy/admin, usually not worth expanding. |
| `planCacheSetFilter` | yes | yes | yes | yes | yes | Later query settings supersede some use cases. |
| `planCacheListFilters` | yes | yes | yes | yes | yes | Stable baseline. |
| `planCacheClearFilters` | yes | yes | yes | yes | yes | Stable baseline. |
| `planCacheClear` | yes | yes | yes | yes | yes | Stable baseline. |
| `listCollections` | yes | yes | yes | yes | yes | Later output includes newer options when implemented. |
| `listDatabases` | yes | yes | yes | yes | yes | Stable baseline. |
| `collStats` | yes | yes | yes | yes | yes | Later output changes with time-series/columnstore/etc. |
| `dbStats` | yes | yes | yes | yes | yes | Raw extractor undercounts old hand-written versions. |
| `dataSize` | yes | yes | yes | yes | yes | Stable baseline. |
| `validate` | yes | yes | yes | yes | yes | Later validation knows new index/storage metadata. |
| `compactStructuredEncryptionData` | no | no | new | yes | yes | Queryable Encryption; very hard for real semantics. |
| `getQueryableEncryptionCountInfo` | no | no | no | new | yes | Queryable Encryption; very hard. |
| `analyze` | no | no | no | new | yes | Statistics/histograms; optional substantial dev. |
| `cleanupStructuredEncryptionData` | no | no | no | no | new | Queryable Encryption cleanup; very hard. |
| `createSearchIndexes` | no | no | no | no | new | Requires search service/mongot for real behavior. |
| `dropSearchIndex` | no | no | no | no | new | Same. |
| `updateSearchIndex` | no | no | no | no | new | Same. |
| `listSearchIndexes` | no | no | no | no | new | Same; also appears as aggregation stage. |
| `setQuerySettings` | no | no | no | no | new | Planner settings infrastructure. |
| `removeQuerySettings` | no | no | no | no | new | Planner settings infrastructure. |

## Command Families to Ignore for This Task

- Sharding/resharding: `reshardCollection`, `moveRange`, `moveCollection`, `unshardCollection`, `_configsvr*`, `_shardsvr*` except where they are internal support for a chosen user-facing data feature.
- Replication/transactions: `commitTransaction`, `abortTransaction`, vote/prepare/participant commands.
- Tenant/serverless/migration: donor/recipient/shard split commands.
- Diagnostics/test-only: `cpuload`, failpoint, xray, FTDC, test command examples.

