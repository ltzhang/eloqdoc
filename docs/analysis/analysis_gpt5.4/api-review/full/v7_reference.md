# V7 Full Reference

<!-- BEGIN v7.md -->
# MongoDB v7.0 API Workbook

Representative branch: `origin/v7.0`

v7 keeps the Stable API envelope, but it changes the ordering of request-metadata handling and adds major analytical / metadata-consistency surfaces.

## Primary source anchors

- `src/mongo/db/service_entry_point_common.cpp`
- `src/mongo/db/initialize_api_parameters.cpp`
- `src/mongo/db/api_parameters.idl`
- `src/mongo/shell/db.js`
- `src/mongo/shell/shell_options.idl`

## Command-entry delta from v6

Observed parse sequence in v7:

1. `initializeAPIParameters(request.body, command)`
2. store `APIParameters::fromClient(...)`
3. `CommandHelpers::uassertShouldAttemptParse(...)`
4. `getClientOperationTime(opCtx)`
5. `rpc::readRequestMetadata(opCtx, request, command->requiresAuth())`
6. `command->parse(opCtx, request)`
7. `CommandInvocation::set(opCtx, _invocation)`

The important change is step ordering:

- request metadata is now read before command-specific parse
- in v5 and v6, request metadata handling was later in the path

This matters if you are porting parser behavior, because command parse now runs with request metadata already applied.

## Server command surface

Exhaustive catalogs:

- `generated/v7_idl_commands.md`
- `generated/v7_manual_commands.md`

Adjacent diffs against v6:

- `generated/v6_to_v7_idl_diff.md`
- `generated/v6_to_v7_manual_diff.md`

Counts:

- IDL commands: `216`
- Manual / non-IDL command definitions: `120`

High-signal public families added or expanded in the v6 -> v7 range:

- query analysis
- analyze shard key
- metadata consistency
- dedicated config-server transition flows

The exact command inventory remains the generated diff files.

## Legacy shell delta from v6

Exhaustive shell option inventory and diff:

- `generated/v7_shell_flags.md`
- `generated/v6_to_v7_shell_flag_diff.md`

Count:

- shell flags/options: `37`

Added shell flag:

- `oidcAccessToken`

Other shell behavior stays close to v6:

- API flags remain
- `_helloOrLegacyHello()` remains
- `_runCommandWithoutApiStrict()` remains
- `listCommands()` still expects `secondaryOk`

## `mongosh` relevance for v7 servers

Current `mongosh` exposes the clearest v7-only shell methods:

- `Collection.analyzeShardKey()` (`7.0+`)
- `Collection.configureQueryAnalyzer()` (`7.0+`)
- `Collection.checkMetadataConsistency()` (`7.0+`)
- `Database.checkMetadataConsistency()` (`7.0+`)

These methods line up directly with the server-side additions seen in the v6 -> v7 command diff.

See:

- `mongosh.md`
- `generated/mongosh_shell_api_inventory.md`
- `generated/mongosh_method_signatures.md`

## Porting checklist from v6

1. Move `readRequestMetadata(...)` so it runs before `command->parse(...)`.
2. Keep the v5/v6 Stable API machinery intact.
3. Apply the v6 -> v7 command additions and field changes from `generated/v6_to_v7_idl_diff.md`.
4. Apply the smaller manual-command delta from `generated/v6_to_v7_manual_diff.md`.
5. Add `oidcAccessToken` if you are matching v7 shell CLI behavior.
6. Add the v7 query-analysis and metadata-consistency helper methods if you are matching the `mongosh` client surface.
<!-- END v7.md -->

<!-- BEGIN v6_to_v7_idl_diff.md -->
# IDL Diff v6 -> v7

- Added commands: 49
- Removed commands: 0
- Changed commands: 32

## Added commands

- `_clusterQueryWithoutShardKey`
- `_clusterWriteWithoutShardKey`
- `_configsvrCheckClusterMetadataConsistency`
- `_configsvrCheckMetadataConsistency`
- `_configsvrCommitChunkMigration`
- `_configsvrCommitIndex`
- `_configsvrCommitMergeAllChunksOnShard`
- `_configsvrDropIndexCatalogEntry`
- `_configsvrGetHistoricalPlacement`
- `_configsvrResetPlacementHistory`
- `_configsvrTransitionFromDedicatedConfigServer`
- `_configsvrTransitionToDedicatedConfigServer`
- `_refreshQueryAnalyzerConfiguration`
- `_shardsvrCheckMetadataConsistency`
- `_shardsvrCheckMetadataConsistencyParticipant`
- `_shardsvrCommitIndexParticipant`
- `_shardsvrCreateGlobalIndex`
- `_shardsvrDeleteGlobalIndexKey`
- `_shardsvrDropCollectionIfUUIDNotMatchingWithWriteConcern`
- `_shardsvrDropGlobalIndex`
- `_shardsvrDropIndexCatalogEntryParticipant`
- `_shardsvrInsertGlobalIndexKey`
- `_shardsvrMergeAllChunksOnShard`
- `_shardsvrMovePrimaryEnterCriticalSection`
- `_shardsvrMovePrimaryExitCriticalSection`
- `_shardsvrNotifyShardingEvent`
- `_shardsvrRegisterIndex`
- `_shardsvrRenameIndexMetadata`
- `_shardsvrUnregisterIndex`
- `_shardsvrValidateShardKeyCandidate`
- `_shardsvrWriteGlobalIndexKeys`
- `analyze`
- `analyzeShardKey`
- `bulkWrite`
- `checkMetadataConsistency`
- `collStats`
- `configureQueryAnalyzer`
- `dataSize`
- `dbStats`
- `getChangeStreamState`
- `getQueryableEncryptionCountInfo`
- `listDatabasesForAllTenants`
- `mergeAllChunksOnShard`
- `moveChunk`
- `resetPlacementHistory`
- `setChangeStreamState`
- `transitionFromDedicatedConfigServer`
- `transitionToDedicatedConfigServer`
- `voteAbortIndexBuild`

## Removed commands

- _None_

## Changed commands

### `_configsvrCommitChunksMerge`
- Fields removed: `validAfter`

### `_configsvrCommitMovePrimary`
- Top-level changed: `type`

### `_configsvrConfigureCollectionBalancing`
- Top-level changed: `strict`

### `_configsvrCreateDatabase`
- Fields removed: `enableSharding`

### `_configsvrEnsureChunkVersionIsGreaterThan`
- Fields modified: `version`

### `_killOperations`
- Top-level changed: `reply_type`

### `_shardsvrDropCollectionParticipant`
- Fields added: `dropSystemCollections`

### `_shardsvrMoveRange`
- Fields modified: `fromShard`

### `_shardsvrParticipantBlock`
- Fields added: `allowViews`, `blockType`, `reason`

### `aggregate`
- Fields added: `$_externalDataSources`, `$_isClusterQueryWithoutShardKeyCmd`, `sampleId`, `stmtId`
- Fields modified: `$_passthroughToShard`, `$_requestReshardingResumeToken`, `$queryOptions`, `collectionUUID`, `encryptionInformation`, `explain`, `isMapReduceCommand`, `runtimeConstants`

### `commitShardSplit`
- Fields modified: `tenantIds`

### `compactStructuredEncryptionData`
- Fields modified: `compactionTokens`

### `count`
- Fields added: `mirrored`, `sampleId`
- Fields modified: `encryptionInformation`, `fields`

### `create`
- Fields removed: `recordPreImages`
- Fields modified: `autoIndexId`, `changeStreamPreAndPostImages`, `flags`, `indexOptionDefaults`, `max`, `size`, `storageEngine`, `temp`

### `createIndexes`
- Fields added: `returnOnStart`
- Fields modified: `collectionUUID`, `commitQuorum`, `ignoreUnknownIndexOptions`

### `createUser`
- Fields removed: `$tenant`

### `delete`
- Fields modified: `runtimeConstants`

### `distinct`
- Fields added: `mirrored`, `sampleId`

### `donorStartMigration`
- Fields added: `tenantIds`
- Fields modified: `tenantId`

### `drop`
- Fields modified: `collectionUUID`

### `find`
- Fields added: `$_startAt`, `mirrored`, `sampleId`
- Fields removed: `ntoreturn`, `options`
- Fields modified: `$_requestResumeToken`, `$_resumeAfter`, `$queryOptions`, `allowSpeculativeMajorityRead`, `awaitData`, `collectionUUID`, `encryptionInformation`, `max`, `min`, `noCursorTimeout`, `oplogReplay`, `readOnce`, `returnKey`, `runtimeConstants`, `showRecordId`, `tailable`, `term`

### `findAndModify`
- Fields added: `$_allowShardKeyUpdatesWithoutFullShardKeyInQuery`, `$_originalCollation`, `$_originalQuery`, `sampleId`
- Fields modified: `bypassEmptyTsReplacement`, `encryptionInformation`

### `getMore`
- Fields modified: `lastKnownCommittedOpTime`, `term`

### `listCollections`
- Fields modified: `includePendingDrops`

### `listIndexes`
- Fields modified: `includeBuildUUIDs`, `includeIndexBuildInfo`

### `profile`
- Top-level changed: `strict`

### `recipientForgetMigration`
- Top-level changed: `reply_type`
- Fields added: `decision`

### `recipientSyncData`
- Top-level changed: `reply_type`

### `replSetResizeOplog`
- Top-level changed: `strict`

### `setFeatureCompatibilityVersion`
- Fields added: `confirm`
- Fields removed: `downgradeOnDiskChanges`

### `setIndexCommitQuorum`
- Top-level changed: `strict`
- Fields modified: `commitQuorum`

### `voteCommitIndexBuild`
- Top-level changed: `description`
<!-- END v6_to_v7_idl_diff.md -->

<!-- BEGIN v6_to_v7_manual_diff.md -->
# Manual Command Diff v6 -> v7

- Added primary names: 2
- Removed primary names: 10

## Added primary names

- `_configsvrTransitionToDedicatedConfigServer`
- `transitionToDedicatedConfigServer`

## Removed primary names

- `_configsvrCommitChunkMigration`
- `_configsvrMoveChunk`
- `availableQueryOptions`
- `dataSize`
- `dbStats`
- `getLastError`
- `getnonce`
- `moveChunk`
- `renameCollection`
- `setShardVersion`
<!-- END v6_to_v7_manual_diff.md -->

<!-- BEGIN manual_added_command_notes.md -->
# Manual Added Command Notes

These notes cover the small set of version-added commands that were not captured purely as “new IDL command names” in the adjacent-version diffs.

They are implementation-oriented and source-backed, intended to close the biggest remaining gap in the generated inventories.

## v4 -> v5

### `cst`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/cst_command.cpp`
- Purpose: test-only CST pipeline parsing/translation command
- Accepted fields:
  - `pipeline`: BSON object parsed by `BSONLexer` with `START_PIPELINE`
- Output fields:
  - `cst`: serialized CST array
  - `ds`: translated pipeline BSON
- Execution traits:
  - secondary allowed: always
  - write concern: not supported
  - auth: requires any action on any resource in the target DB

### `sbe`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/exec/sbe_cmd.cpp`
- Purpose: test-only command for manually constructing and executing an SBE tree
- Accepted fields:
  - `sbe`: string containing SBE query text
  - cursor command options parsed via `CursorRequest::parseCommandCursorOptions(...)`
    - includes normal cursor batch options such as `batchSize`
- Output:
  - standard cursor response object
- Execution traits:
  - secondary allowed: opt-in
  - write concern: not supported
  - auth: requires any action on any resource in the target DB

### `pinHistoryReplicated`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_commands.cpp`
- Purpose: test-only command that pins the oldest timestamp
- Accepted fields:
  - first command element value: timestamp to pin
  - `round`: optional boolean
- Output fields:
  - `requestedPinTs`
  - `pinTs`
- Execution traits:
  - secondary allowed: never
  - write concern: supported
  - admin only: true
  - authz checks disabled in practice because command is test-gated

### `testDeprecation`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_deprecation_command.cpp`
- Purpose: test command for API-version deprecation logic
- Accepted fields:
  - no command-specific fields beyond the command name
- Output:
  - appends current `APIParameters` info to the reply
- Execution traits:
  - API versions: `1`
  - deprecated API versions: `1`
  - secondary allowed: always
  - write concern: not supported
  - auth required: false

### `testVersion2`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_api_version_2_commands.cpp`
- Accepted fields:
  - no command-specific fields beyond the command name
- Output:
  - current `APIParameters`
- Execution traits:
  - API versions: `2`
  - secondary allowed: always
  - write concern: not supported

### `testVersions1And2`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_api_version_2_commands.cpp`
- Accepted fields:
  - no command-specific fields beyond the command name
- Output:
  - current `APIParameters`
- Execution traits:
  - API versions: `1`, `2`
  - secondary allowed: always
  - write concern: not supported

### `testDeprecationInVersion2`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_api_version_2_commands.cpp`
- Accepted fields:
  - no command-specific fields beyond the command name
- Output:
  - current `APIParameters`
- Execution traits:
  - API versions: `1`, `2`
  - deprecated API versions: `2`
  - secondary allowed: always
  - write concern: not supported

### `testRemoval`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_api_version_2_commands.cpp`
- Accepted fields:
  - no command-specific fields beyond the command name
- Output:
  - current `APIParameters`
- Execution traits:
  - API versions: `1`
  - secondary allowed: always
  - write concern: not supported

## v5 -> v6

### `_configsvrRunRestore`

- Introduced on `origin/v6.0`
- Source: `src/mongo/db/s/config/configsvr_run_restore_command.cpp`
- Purpose: internal restore procedure command for restoring config metadata
- Accepted fields:
  - no explicit command-specific fields are parsed in the command body
  - command behavior depends on the presence of `local.system.collections_to_restore`
- Execution traits:
  - skips API version check
  - secondary allowed: never
  - admin only: true
  - write concern: not supported
  - auth: requires cluster `internal`
  - runtime preconditions:
    - standalone mode only
    - restore procedure mode only

### `_recvChunkReleaseCritSec`

- Introduced on `origin/v6.0`
- Source: `src/mongo/db/s/migration_destination_manager_legacy_commands.cpp`
- Purpose: internal migration-destination command to release the recipient critical section
- Accepted fields:
  - migration session fields parsed via `MigrationSessionId::extractFromBSON(cmdObj)`
- Execution traits:
  - write concern: supported, majority required
  - admin only: true
  - auth: cluster `internal`
  - operation context interrupts on stepdown/stepup

## v6 -> v7

### `_configsvrTransitionToDedicatedConfigServer`

- Introduced on `origin/v7.0`
- Source: `src/mongo/db/s/config/configsvr_transition_to_dedicated_config_server_command.cpp`
- Purpose: internal config-server command to transition from config shard to dedicated config server
- Accepted fields:
  - no explicit command-specific fields in the command body
- Execution traits:
  - skips API version check
  - secondary allowed: never
  - admin only: true
  - write concern: supported, majority required
  - auth: cluster `internal`
  - feature flags checked before execution
  - config-server role required

### `transitionToDedicatedConfigServer`

- Introduced on `origin/v7.0`
- Public/mongos-facing form exists in the transition-to-dedicated-config-server command family
- See the version command inventory and topology command sources for the public request/dispatch path

## v7 -> v8

### `_mongotConnPoolStats`

- Introduced on `origin/v8.0`
- Source: `src/mongo/db/commands/query_cmd/search_test_cmds.cpp`
- Purpose: test/internal stats for server-to-mongot connection pooling
- Accepted fields:
  - no command-specific fields required
- Output:
  - either gRPC network-interface stats or connection pool stats
- Execution traits:
  - admin only: true
  - secondary allowed: always
  - write concern: not supported
  - registered for shard and router, test only

### `_dropConnectionsToMongot`

- Introduced on `origin/v8.0`
- Source: `src/mongo/db/commands/query_cmd/search_test_cmds.cpp`
- Purpose: test/internal connection dropping between server and mongot
- Accepted fields:
  - `hostAndPort`: array of host:port strings
- Execution traits:
  - admin only: true
  - secondary allowed: always
  - write concern: not supported
  - registered for shard and router, test only

### `getShardingReady`

- Introduced on `origin/v8.0`
- Source: `src/mongo/db/sharding_environment/get_sharding_ready_command.cpp`
- Purpose: test-only command indicating whether sharding is ready
- Accepted fields:
  - no command-specific fields required
- Output:
  - `isReady`
- Execution traits:
  - admin only: false
  - secondary allowed: always
  - write concern: not supported
  - registered for shard only, test only

### `timeseriesCatalogBucketParamsChanged`

- Introduced on `origin/v8.0`
- Source: `src/mongo/db/commands/test_commands.cpp`
- Purpose: test command returning whether time-series bucket parameters changed
- Accepted fields:
  - namespace parsed via `CommandHelpers::parseNsCollectionRequired(dbName, cmdObj)`
  - effectively the command target collection plus DB context
- Output:
  - `changed` when available
- Execution traits:
  - admin only: false
  - secondary allowed: always
  - write concern: not supported

### `testCommandFeatureFlaggedOnLatestFCV83`

- Present on `origin/v8.0`
- Source: `src/mongo/db/commands/test_commands.cpp`
- Purpose: test-only feature-flagged command
- Accepted fields:
  - no command-specific fields required
- Output:
  - no structured payload beyond success
- Execution traits:
  - admin only: false
  - secondary allowed: always
  - write concern: supported
  - requires feature flag `gFeatureFlagBlender`
<!-- END manual_added_command_notes.md -->

<!-- BEGIN v7_idl_commands.md -->
# v7 IDL Command Inventory

## Summary
- Command count: 216

## `_addShard`
- source=`src/mongo/db/s/add_shard_cmd.idl`, namespace=`ignored`, strict=`None`, api_version=``
- _addShard Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `shardIdentity` | `ShardIdentity` | `False` | `` | `False` | `` | `` |

## `_clusterQueryWithoutShardKey`
- source=`src/mongo/s/request_types/cluster_commands_without_shard_key.idl`, namespace=`ignored`, strict=`None`, api_version=`1`, reply=`clusterQueryWithoutShardKeyResponse`
- "An internal command used to broadcast a query to all shards for updates/deletes /findAndModifies that do not provide a shard key."
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `writeCmd` | `object` | `False` | `` | `False` | `` | `` |

## `_clusterWriteWithoutShardKey`
- source=`src/mongo/s/request_types/cluster_commands_without_shard_key.idl`, namespace=`ignored`, strict=`None`, api_version=`1`, reply=`clusterWriteWithoutShardKeyResponse`
- "An internal command used to target an update/delete/findAndModify command to a specific shard using a shardId."
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `writeCmd` | `object` | `False` | `` | `False` | `` | `` |
| `shardId` | `string` | `False` | `` | `False` | `` | `` |
| `targetDocId` | `object` | `False` | `` | `False` | `` | `` |

## `_configsvrAbortReshardCollection`
- source=`src/mongo/s/request_types/abort_reshard_collection.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal abortReshardCollection command on the config server.
_No IDL fields declared._

## `_configsvrBalancerCollectionStatus`
- source=`src/mongo/s/request_types/balancer_collection_status.idl`, namespace=`type`, strict=`True`, api_version=``
- Internal balancerCollectionStatus command on the config server
_No IDL fields declared._

## `_configsvrCheckClusterMetadataConsistency`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`ignored`, strict=`False`, api_version=``, reply=`CursorInitialReply`
- Internal command sent to the config server to check cluster metadata consistency.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `cursor` | `SimpleCursorOptions` | `True` | `` | `False` | `` | `` |

## `_configsvrCheckMetadataConsistency`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``, reply=`CursorInitialReply`
- Internal command sent to the config server to check metadata consistency.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `cursor` | `SimpleCursorOptions` | `True` | `` | `False` | `` | `` |

## `_configsvrCleanupReshardCollection`
- source=`src/mongo/s/request_types/cleanup_reshard_collection.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal cleanupReshardCollection command on the config server.
_No IDL fields declared._

## `_configsvrClearJumboFlag`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- internal clearJumboFlag command for config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `epoch` | `objectid` | `False` | `` | `False` | `` | `` |
| `minKey` | `object` | `False` | `` | `False` | `` | `` |
| `maxKey` | `object` | `False` | `` | `False` | `` | `` |

## `_configsvrCollMod`
- source=`src/mongo/db/s/sharded_collmod.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal command sent to config server to update routing related information
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `collModRequest` | `CollModRequest` | `False` | `` | `False` | `` | `` |

## `_configsvrCommitChunkMigration`
- source=`src/mongo/db/s/commit_chunk_migration.idl`, namespace=`type`, strict=`False`, api_version=``, reply=`ConfigSvrCommitChunkMigrationResponse`
- internal _configsvrCommitChunkMigration command for config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `fromShard` | `shard_id` | `False` | `` | `False` | `` | `` |
| `toShard` | `shard_id` | `False` | `` | `False` | `` | `` |
| `migratedChunk` | `MigratedChunkType` | `False` | `` | `False` | `` | `` |
| `fromShardCollectionVersion` | `ChunkVersion` | `False` | `` | `False` | `` | `` |

## `_configsvrCommitChunksMerge`
- source=`src/mongo/s/request_types/merge_chunk_request.idl`, namespace=`type`, strict=`False`, api_version=``, reply=`ConfigSvrMergeResponse`
- The internal _configsvrCommitChunksMerge command on the config server.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `shard` | `shard_id` | `False` | `` | `False` | `` | `` |
| `collUUID` | `my_uuid` | `False` | `` | `False` | `collectionUUID` | `` |
| `chunkRange` | `chunk_range` | `False` | `` | `False` | `chunkRange` | `` |
| `epoch` | `objectid` | `True` | `` | `False` | `` | `` |
| `timestamp` | `timestamp` | `True` | `` | `False` | `` | `` |

## `_configsvrCommitIndex`
- source=`src/mongo/db/s/sharded_index_catalog_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- Internal command to commit an index catalog entry locally in the config server.
_No IDL fields declared._

## `_configsvrCommitMergeAllChunksOnShard`
- source=`src/mongo/s/request_types/merge_chunk_request.idl`, namespace=`type`, strict=`False`, api_version=``, reply=`ConfigSvrMergeResponse`
- Definition of the mergeAllChunksOnShard command called on the config server.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `shard` | `shard_id` | `False` | `` | `False` | `` | `{"callback": "ShardId::validate"}` |
| `maxNumberOfChunksToMerge` | `int` | `False` | `2147483647` | `False` | `` | `` |
| `maxTimeProcessingChunksMS` | `int` | `False` | `2147483647` | `False` | `` | `` |

## `_configsvrCommitMovePrimary`
- source=`src/mongo/s/request_types/move_primary.idl`, namespace=`type`, strict=`False`, api_version=`None`
- Reassign a new primary shard for the given database on the config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `expectedDatabaseVersion` | `database_version` | `False` | `` | `False` | `` | `` |
| `to` | `shard_id` | `False` | `` | `False` | `` | `` |

## `_configsvrCommitReshardCollection`
- source=`src/mongo/s/request_types/commit_reshard_collection.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal commitReshardCollection command on the config server.
_No IDL fields declared._

## `_configsvrConfigureCollectionBalancing`
- source=`src/mongo/s/request_types/configure_collection_balancing.idl`, namespace=`type`, strict=`False`, api_version=``
- Internal configureCollectionBalancing command on the config server
_No IDL fields declared._

## `_configsvrCreateDatabase`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`type`, strict=`False`, api_version=``, reply=`ConfigsvrCreateDatabaseResponse`
- The internal createDatabase command on the config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `primaryShardId` | `shard_id` | `True` | `` | `False` | `` | `` |

## `_configsvrDropIndexCatalogEntry`
- source=`src/mongo/db/s/sharded_index_catalog_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- Internal command to drop an index catalog entry locally in the config server.
_No IDL fields declared._

## `_configsvrEnsureChunkVersionIsGreaterThan`
- source=`src/mongo/s/request_types/ensure_chunk_version_is_greater_than.idl`, namespace=`ignored`, strict=`False`, api_version=``
- If a chunk matching 'requestedChunk' exists, bumps the chunk's version to one greater than the current collection version.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `minKey` | `object` | `False` | `` | `False` | `` | `` |
| `maxKey` | `object` | `False` | `` | `False` | `` | `` |
| `version` | `ChunkVersion` | `False` | `` | `False` | `` | `` |
| `collectionUUID` | `uuid` | `False` | `` | `False` | `` | `` |
| `nss` | `namespacestring` | `False` | `` | `False` | `` | `` |

## `_configsvrGetHistoricalPlacement`
- source=`src/mongo/s/request_types/placement_history_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- Internal command to retrieve the list of shard IDs hosting data for the namespace/cluster being targeted at a specific point in time
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `at` | `timestamp` | `False` | `` | `False` | `` | `` |
| `targetWholeCluster` | `bool` | `False` | `False` | `False` | `` | `` |

## `_configsvrMoveRange`
- source=`src/mongo/s/request_types/move_range_request.idl`, namespace=`type`, strict=`False`, api_version=``
- Definition of the moveRange command called from routers on the config server.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `forceJumbo` | `ForceJumbo` | `False` | `kDoNotForce` | `False` | `` | `` |
| `secondaryThrottle` | `MigrationSecondaryThrottle` | `True` | `` | `False` | `` | `` |

## `_configsvrRefineCollectionShardKey`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal refineCollectionShardKey command on the config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `object` | `False` | `` | `False` | `` | `` |
| `epoch` | `objectid` | `False` | `` | `False` | `` | `` |
| `enforceUniquenessCheck` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `_configsvrRemoveChunks`
- source=`src/mongo/db/s/remove_chunks.idl`, namespace=`ignored`, strict=`False`, api_version=``
- internal _configsvrRemoveChunks command for config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `collectionUUID` | `uuid` | `False` | `` | `False` | `` | `` |

## `_configsvrRemoveTags`
- source=`src/mongo/db/s/remove_tags.idl`, namespace=`type`, strict=`False`, api_version=``
- internal _configsvrRemoveTags command for config server
_No IDL fields declared._

## `_configsvrRenameCollectionMetadata`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal command for renaming collection metadata on the CSRS
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `to` | `namespacestring` | `False` | `` | `False` | `` | `` |
| `optFromCollection` | `CollectionType` | `True` | `` | `False` | `` | `` |

## `_configsvrResetPlacementHistory`
- source=`src/mongo/s/request_types/placement_history_commands.idl`, namespace=`ignored`, strict=`False`, api_version=``
- Definition of the resetPlacementHistory command called from routers on the config server.
_No IDL fields declared._

## `_configsvrResetPlacementHistory`
- source=`src/mongo/s/request_types/reset_placement_history.idl`, namespace=`ignored`, strict=`False`, api_version=``
- Definition of the resetPlacementHistory command called from routers on the config server.
_No IDL fields declared._

## `_configsvrReshardCollection`
- source=`src/mongo/s/request_types/reshard_collection.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal reshardCollection command on the config server.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `object` | `False` | `` | `False` | `` | `` |
| `unique` | `bool` | `True` | `` | `False` | `` | `` |
| `numInitialChunks` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `collation` | `object` | `True` | `` | `False` | `` | `` |
| `zones` | `array<ReshardingZoneType>` | `True` | `` | `False` | `` | `` |
| `_presetReshardedChunks` | `array<ReshardedChunk>` | `True` | `` | `False` | `` | `` |

## `_configsvrSetAllowMigrations`
- source=`src/mongo/s/request_types/set_allow_migrations.idl`, namespace=`type`, strict=`False`, api_version=``
- internal setAllowMigrations command for config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `allowMigrations` | `bool` | `False` | `` | `False` | `` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `False` | `` | `` |

## `_configsvrSetClusterParameter`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- Internal command sent to the config server to start a coordinator and set the cluster parameter.
_No IDL fields declared._

## `_configsvrSetUserWriteBlockMode`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`ignored`, strict=`False`, api_version=``
- internal _configsvrSetUserWriteBlockMode command
_No IDL fields declared._

## `_configsvrTransitionFromDedicatedConfigServer`
- source=`src/mongo/s/request_types/transition_from_dedicated_config_server.idl`, namespace=`ignored`, strict=`False`, api_version=``
- The transitionFromDedicatedConfigServer command for config server.
_No IDL fields declared._

## `_configsvrTransitionToDedicatedConfigServer`
- source=`src/mongo/s/request_types/transition_to_dedicated_config_server.idl`, namespace=`ignored`, strict=`False`, api_version=``
- mongos command for transitioning to dedicated config server
_No IDL fields declared._

## `_flushDatabaseCacheUpdates`
- source=`src/mongo/s/request_types/flush_database_cache_updates.idl`, namespace=`type`, strict=`True`, api_version=``
- An internal command to wait for the last routing table cache refresh for a particular database to be persisted to disk
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `syncFromConfig` | `bool` | `False` | `True` | `False` | `` | `` |

## `_flushDatabaseCacheUpdatesWithWriteConcern`
- source=`src/mongo/s/request_types/flush_database_cache_updates.idl`, namespace=`type`, strict=`True`, api_version=``
- The same behavior as _flushDatabaseCacheUpdates but accepts writeConcern
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `syncFromConfig` | `bool` | `False` | `True` | `False` | `` | `` |

## `_flushReshardingStateChange`
- source=`src/mongo/s/request_types/flush_resharding_state_change.idl`, namespace=`type`, strict=`True`, api_version=``
- An internal command to flush resharding state changes to shards.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `reshardingUUID` | `uuid` | `False` | `` | `False` | `` | `` |

## `_flushRoutingTableCacheUpdates`
- source=`src/mongo/s/request_types/flush_routing_table_cache_updates.idl`, namespace=`type`, strict=`True`, api_version=``
- An internal command to wait for the last routing table cache refresh for a particular namespace to be persisted to disk.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `syncFromConfig` | `bool` | `False` | `True` | `False` | `` | `` |

## `_flushRoutingTableCacheUpdatesWithWriteConcern`
- source=`src/mongo/s/request_types/flush_routing_table_cache_updates.idl`, namespace=`type`, strict=`True`, api_version=``
- The same behavior as _flushRoutingTableCacheUpdates but accepts writeConcern.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `syncFromConfig` | `bool` | `False` | `True` | `False` | `` | `` |

## `_getUserCacheGeneration`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`ignored`, strict=`True`, api_version=``, reply=`GetUserCacheGenerationReply`
- Returns the current user cache generation
_No IDL fields declared._

## `_killOperations`
- source=`src/mongo/db/commands/kill_operations.idl`, namespace=`ignored`, strict=`True`, api_version=``, reply=`OkReply`
- Interrupt a list of operations on a remote server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `operationKeys` | `array<uuid>` | `False` | `` | `False` | `` | `` |

## `_mergeAuthzCollections`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Internal command used by mongorestore for updating user/role data
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `tempUsersCollection` | `string` | `False` | `""` | `False` | `` | `` |
| `tempRolesCollection` | `string` | `False` | `""` | `False` | `` | `` |
| `db` | `string` | `False` | `` | `False` | `` | `` |
| `drop` | `bool` | `False` | `False` | `False` | `` | `` |

## `_refreshQueryAnalyzerConfiguration`
- source=`src/mongo/s/refresh_query_analyzer_configuration_cmd.idl`, namespace=`ignored`, strict=`False`, api_version=``
- The command for refreshing the query analyzer configurations for all collections.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `name` | `string` | `False` | `` | `False` | `` | `` |
| `numQueriesExecutedPerSecond` | `double` | `False` | `` | `False` | `` | `` |

## `_shardsvrAbortReshardCollection`
- source=`src/mongo/s/request_types/abort_reshard_collection.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal command used by resharding coordinator to abort participants.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `userCanceled` | `bool` | `False` | `` | `False` | `` | `` |

## `_shardsvrCheckMetadataConsistency`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``, reply=`CursorInitialReply`
- Internal command sent to the primary shard of a database to check metadata consistency.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `cursor` | `SimpleCursorOptions` | `True` | `` | `False` | `` | `` |

## `_shardsvrCheckMetadataConsistencyParticipant`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``, reply=`CursorInitialReply`
- Internal command sent to participants shards to check metadata consistency.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `cursor` | `SimpleCursorOptions` | `True` | `` | `False` | `` | `` |
| `primaryShardId` | `shard_id` | `False` | `` | `False` | `` | `` |

## `_shardsvrCleanupReshardCollection`
- source=`src/mongo/s/request_types/cleanup_reshard_collection.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal cleanupReshardCollection command on the shard server.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `reshardingUUID` | `uuid` | `False` | `` | `False` | `` | `` |

## `_shardsvrCollMod`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal command sent to the primary shard in a collMod procedure
_No IDL fields declared._

## `_shardsvrCollModParticipant`
- source=`src/mongo/db/s/sharded_collmod.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal command sent to all shards to implement collMod locally
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `collModRequest` | `CollModRequest` | `False` | `` | `False` | `` | `` |
| `needsUnblock` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `performViewChange` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `_shardsvrCommitIndexParticipant`
- source=`src/mongo/db/s/sharded_index_catalog_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- Internal command to commit an index catalog entry locally in a shard.
_No IDL fields declared._

## `_shardsvrCommitReshardCollection`
- source=`src/mongo/s/request_types/commit_reshard_collection.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal commitReshardCollection command on the shard server.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `reshardingUUID` | `uuid` | `False` | `` | `False` | `` | `` |

## `_shardsvrCreateCollection`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- The internal createCollection command for a shard.
_No IDL fields declared._

## `_shardsvrCreateCollectionParticipant`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Command to create a collection on participant shards, when called, assumes the primary shard is under the critical section for that namespace.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `indexes` | `array<object>` | `False` | `` | `False` | `` | `` |
| `options` | `object` | `False` | `` | `False` | `` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `False` | `` | `` |
| `idIndex` | `object` | `False` | `` | `False` | `` | `` |

## `_shardsvrCreateGlobalIndex`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`type`, strict=`True`, api_version=``, reply=`OkReply`
- Internal command sent to participant shards to create a global index container with a given UUID.
_No IDL fields declared._

## `_shardsvrDeleteGlobalIndexKey`
- source=`src/mongo/db/s/global_index_crud_commands.idl`, namespace=`type`, strict=`True`, api_version=``, reply=`OkReply`
- Internal command to delete a key from a global index container.
_No IDL fields declared._

## `_shardsvrDropCollection`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Parser for the _shardsvrDropCollection command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `collectionUUID` | `uuid` | `True` | `` | `False` | `` | `` |

## `_shardsvrDropCollectionIfUUIDNotMatching`
- source=`src/mongo/s/request_types/drop_collection_if_uuid_not_matching.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal dropCollectionIfUUIDNotMatching request.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `expectedCollectionUUID` | `uuid` | `False` | `` | `False` | `` | `` |

## `_shardsvrDropCollectionIfUUIDNotMatchingWithWriteConcern`
- source=`src/mongo/s/request_types/drop_collection_if_uuid_not_matching.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal dropCollectionIfUUIDNotMatching request with write concern.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `expectedCollectionUUID` | `uuid` | `False` | `` | `False` | `` | `` |

## `_shardsvrDropCollectionParticipant`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Parser for the _shardsvrDropCollectionParticipant command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `fromMigrate` | `bool` | `True` | `` | `False` | `` | `` |
| `dropSystemCollections` | `bool` | `True` | `` | `False` | `` | `` |

## `_shardsvrDropDatabase`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`ignored`, strict=`None`, api_version=``
- Internal command sent to the primary shard of a database to drop it.
_No IDL fields declared._

## `_shardsvrDropDatabaseParticipant`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`ignored`, strict=`False`, api_version=``
- Internal command sent to participants shards to drop a database.
_No IDL fields declared._

## `_shardsvrDropGlobalIndex`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`type`, strict=`True`, api_version=``, reply=`OkReply`
- Internal command sent to participant shards to drop a global index container with a given UUID.
_No IDL fields declared._

## `_shardsvrDropIndexCatalogEntryParticipant`
- source=`src/mongo/db/s/sharded_index_catalog_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- Internal command to drop an index catalog entry locally in a shard.
_No IDL fields declared._

## `_shardsvrDropIndexes`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Parser for the _shardsvrDropIndexes command
_No IDL fields declared._

## `_shardsvrGetStatsForBalancing`
- source=`src/mongo/s/request_types/get_stats_for_balancing.idl`, namespace=`ignored`, strict=`False`, api_version=``, reply=`ShardsvrGetStatsForBalancingReply`
- Internal command used by the balancer to retrieve stats for balancing.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `collections` | `array<NamespaceWithOptionalUUID>` | `False` | `` | `False` | `` | `` |
| `scaleFactor` | `exactInt64` | `True` | `` | `False` | `` | `` |

## `_shardsvrInsertGlobalIndexKey`
- source=`src/mongo/db/s/global_index_crud_commands.idl`, namespace=`type`, strict=`True`, api_version=``, reply=`OkReply`
- Internal command to insert a key into a global index container.
_No IDL fields declared._

## `_shardsvrJoinMigrations`
- source=`src/mongo/s/request_types/shardsvr_join_migrations_request.idl`, namespace=`ignored`, strict=`False`, api_version=``
- Command to synch the caller on the completion of any chunk migration activity performed by the shard (as either donor or recipient)
_No IDL fields declared._

## `_shardsvrMergeAllChunksOnShard`
- source=`src/mongo/s/request_types/merge_chunk_request.idl`, namespace=`type`, strict=`False`, api_version=``
- Definition of the mergeAllChunksOnShard command called on the shards.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `shard` | `shard_id` | `False` | `` | `False` | `` | `{"callback": "ShardId::validate"}` |
| `maxNumberOfChunksToMerge` | `int` | `False` | `2147483647` | `False` | `` | `` |
| `maxTimeProcessingChunksMS` | `int` | `False` | `2147483647` | `False` | `` | `` |

## `_shardsvrMovePrimaryEnterCriticalSection`
- source=`src/mongo/s/request_types/move_primary.idl`, namespace=`type`, strict=`False`, api_version=`None`
- Block CRUD operations on the given database by entering the critical section.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `reason` | `object` | `False` | `` | `False` | `` | `` |

## `_shardsvrMovePrimaryExitCriticalSection`
- source=`src/mongo/s/request_types/move_primary.idl`, namespace=`type`, strict=`False`, api_version=`None`
- Clear the local metadata of the given database, and unblocks CRUD operations by exiting the critical section.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `reason` | `object` | `False` | `` | `False` | `` | `` |

## `_shardsvrMoveRange`
- source=`src/mongo/s/request_types/move_range_request.idl`, namespace=`type`, strict=`False`, api_version=``
- Definition of the moveRange command called from the config server on shards.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `epoch` | `objectid` | `False` | `mongo::OID()` | `False` | `` | `` |
| `fromShard` | `shard_id` | `False` | `` | `False` | `` | `{"callback": "ShardId::validate"}` |
| `maxChunkSizeBytes` | `safeInt64` | `False` | `` | `False` | `` | `` |
| `forceJumbo` | `ForceJumbo` | `False` | `kDoNotForce` | `False` | `` | `` |
| `secondaryThrottle` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `_shardsvrNotifyShardingEvent`
- source=`src/mongo/db/commands/notify_sharding_event.idl`, namespace=`ignored`, strict=`False`, api_version=``, reply=`OkReply`
- Internal command to be invoked by the config server to notify a shard of an event concerning the shard itself or the whole cluster.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `eventType` | `string` | `False` | `` | `False` | `` | `{"callback": "notify_sharding_event::validateEventType"}` |
| `details` | `object_owned` | `False` | `` | `False` | `` | `` |

## `_shardsvrParticipantBlock`
- source=`src/mongo/db/s/participant_block.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal command sent to shards to block CRUD operations
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `blockType` | `CriticalSectionBlockType` | `True` | `` | `False` | `` | `` |
| `reason` | `object` | `True` | `` | `False` | `` | `` |
| `allowViews` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `_shardsvrRefineCollectionShardKey`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Parser for the _shardsvrRefineCollectionShardKey command
_No IDL fields declared._

## `_shardsvrRegisterIndex`
- source=`src/mongo/db/s/sharded_index_catalog_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- Internal test command to register an index catalog entry in the cluster.
_No IDL fields declared._

## `_shardsvrRenameCollection`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal renameCollection command for a shard.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `allowEncryptedCollectionRename` | `bool` | `True` | `` | `False` | `` | `` |

## `_shardsvrRenameCollectionParticipant`
- source=`src/mongo/db/s/sharded_rename_collection.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal command sent to shard participating in a rename collection procedure
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `sourceUUID` | `uuid` | `False` | `` | `False` | `` | `` |
| `targetUUID` | `uuid` | `True` | `` | `False` | `` | `` |

## `_shardsvrRenameCollectionParticipantUnblock`
- source=`src/mongo/db/s/sharded_rename_collection.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Parser for the _shardsvrRenameCollectionParticipantUnblock command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `sourceUUID` | `uuid` | `False` | `` | `False` | `` | `` |

## `_shardsvrRenameIndexMetadata`
- source=`src/mongo/db/s/sharded_index_catalog_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- Internal command to rename the sharding index catalog metadata in a shard.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `toNss` | `namespacestring` | `False` | `` | `False` | `` | `` |
| `indexVersion` | `CollectionIndexes` | `False` | `` | `False` | `` | `` |

## `_shardsvrReshardCollection`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal reshardCollection command on the shard.
_No IDL fields declared._

## `_shardsvrReshardingOperationTime`
- source=`src/mongo/s/request_types/resharding_operation_time.idl`, namespace=`type`, strict=`True`, api_version=``
- An internal command to query the operation time for participant shards.
_No IDL fields declared._

## `_shardsvrSetAllowMigrations`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal setAllowMigrations command for a shard.
_No IDL fields declared._

## `_shardsvrSetClusterParameter`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- internal _shardsvrSetClusterParameter command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `clusterParameterTime` | `timestamp` | `False` | `` | `False` | `` | `` |

## `_shardsvrSetUserWriteBlockMode`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`ignored`, strict=`False`, api_version=``
- internal _shardsvrSetUserWriteBlockMode command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `phase` | `ShardsvrSetUserWriteBlockModePhase` | `False` | `` | `False` | `` | `` |

## `_shardsvrUnregisterIndex`
- source=`src/mongo/db/s/sharded_index_catalog_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- Internal test command to unregister an index catalog entry in the cluster.
_No IDL fields declared._

## `_shardsvrValidateShardKeyCandidate`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- Command run on shards to ensure we have a viable key to refine the shard key.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `KeyPattern` | `False` | `` | `False` | `` | `` |
| `enforceUniquenessCheck` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `_shardsvrWriteGlobalIndexKeys`
- source=`src/mongo/db/s/global_index_crud_commands.idl`, namespace=`ignored`, strict=`None`, api_version=``, reply=`OkReply`
- Internal command to run _shardsvrInsertGlobalIndexKey and _shardsvrDeleteGlobalIndexKey statements in bulk.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `ops` | `array<object>` | `False` | `` | `False` | `ops` | `` |

## `abortReshardCollection`
- source=`src/mongo/s/request_types/abort_reshard_collection.idl`, namespace=`type`, strict=`True`, api_version=``
- The public abortReshardCollection command on mongos.
_No IDL fields declared._

## `abortShardSplit`
- source=`src/mongo/db/serverless/shard_split_commands.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Parser for the `abortShardSplit` command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `migrationId` | `uuid` | `False` | `` | `False` | `` | `` |

## `abortTransaction`
- source=`src/mongo/db/commands/txn_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=`1`, reply=`OkReply`
- abortTransaction Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `recoveryToken` | `TxnRecoveryToken` | `True` | `` | `False` | `` | `` |

## `aggregate`
- source=`src/mongo/db/pipeline/aggregate_command.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`CursorInitialReply`
- Represents the user-supplied options to the aggregate command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `pipeline` | `pipeline` | `False` | `` | `False` | `` | `` |
| `explain` | `explainVerbosity` | `True` | `` | `False` | `` | `` |
| `allowDiskUse` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `cursor` | `aggregateCursor` | `False` | `SimpleCursorOptions()` | `False` | `` | `` |
| `maxTimeMS` | `safeInt64` | `True` | `` | `False` | `` | `{"gte": 0}` |
| `bypassDocumentValidation` | `safeBool` | `True` | `` | `False` | `` | `` |
| `readConcern` | `object_owned` | `True` | `` | `False` | `` | `` |
| `collation` | `object_owned` | `True` | `` | `False` | `` | `` |
| `hint` | `indexHint` | `True` | `` | `False` | `` | `` |
| `writeConcern` | `WriteConcern` | `True` | `` | `False` | `` | `` |
| `let` | `object_owned` | `True` | `` | `False` | `` | `` |
| `needsMerge` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `fromMongos` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `$queryOptions` | `object_owned` | `True` | `` | `False` | `unwrappedReadPref` | `` |
| `$_requestReshardingResumeToken` | `optionalBool` | `False` | `` | `False` | `requestReshardingResumeToken` | `` |
| `exchange` | `ExchangeSpec` | `True` | `` | `False` | `` | `` |
| `runtimeConstants` | `LegacyRuntimeConstants` | `True` | `` | `False` | `legacyRuntimeConstants` | `` |
| `isMapReduceCommand` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `False` | `` | `` |
| `$_passthroughToShard` | `PassthroughToShardOptions` | `True` | `` | `False` | `passthroughToShard` | `` |
| `$_generateV2ResumeTokens` | `optionalBool` | `False` | `` | `False` | `generateV2ResumeTokens` | `` |
| `encryptionInformation` | `EncryptionInformation` | `True` | `` | `False` | `` | `` |
| `$_externalDataSources` | `array<ExternalDataSourceOption>` | `True` | `` | `False` | `externalDataSources` | `` |
| `sampleId` | `uuid` | `True` | `` | `False` | `` | `` |
| `stmtId` | `int` | `True` | `` | `False` | `` | `` |
| `$_isClusterQueryWithoutShardKeyCmd` | `optionalBool` | `False` | `` | `False` | `isClusterQueryWithoutShardKeyCmd` | `` |

## `analyze`
- source=`src/mongo/db/query/analyze_command.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=``, reply=`OkReply`
- Command to allow for manual update and creation of statistics.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `string` | `True` | `` | `False` | `` | `` |
| `sampleRate` | `safeDouble` | `True` | `` | `False` | `` | `{"gt": 0.0, "lte": 1.0}` |
| `sampleSize` | `safeInt` | `True` | `` | `False` | `` | `{"gt": 0}` |
| `numberBuckets` | `safeInt` | `True` | `` | `False` | `` | `{"gt": 0}` |

## `analyzeShardKey`
- source=`src/mongo/s/analyze_shard_key_cmd.idl`, namespace=`type`, strict=`False`, api_version=``
- The command for calculating metrics for evaluating a shard key for a collection.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `KeyPattern` | `False` | `` | `False` | `` | `{"callback": "validateShardKeyPattern"}` |
| `keyCharacteristics` | `bool` | `False` | `True` | `False` | `analyzeKeyCharacteristics` | `` |
| `readWriteDistribution` | `bool` | `False` | `True` | `False` | `analyzeReadWriteDistribution` | `` |
| `sampleRate` | `safeDouble` | `True` | `` | `False` | `` | `{"gt": 0, "lte": 1}` |
| `sampleSize` | `safeInt64` | `True` | `` | `False` | `` | `{"gt": 0}` |
| `$readPreference` | `readPreference` | `False` | `ReadPreferenceSetting{ReadPreference::SecondaryPreferred}` | `False` | `readPreference` | `` |

## `authenticate`
- source=`src/mongo/db/commands/authentication_commands.idl`, namespace=`ignored`, strict=`True`, api_version=`1`, reply=`AuthenticateReply`
- Begin a X509 based authentication session
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `mechanism` | `string` | `False` | `` | `False` | `` | `` |
| `user` | `string` | `True` | `` | `False` | `` | `` |

## `autoSplitVector`
- source=`src/mongo/s/request_types/auto_split_vector.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal autoSplitVector command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `keyPattern` | `object_owned` | `False` | `` | `False` | `` | `` |
| `min` | `object_owned` | `False` | `` | `False` | `` | `` |
| `max` | `object_owned` | `False` | `` | `False` | `` | `` |
| `maxChunkSizeBytes` | `safeInt64` | `False` | `` | `False` | `` | `` |
| `limit` | `int` | `True` | `` | `False` | `` | `` |

## `balancerCollectionStatus`
- source=`src/mongo/s/request_types/balancer_collection_status.idl`, namespace=`type`, strict=`True`, api_version=``
- Public balancerCollectionStatus command on mongos
_No IDL fields declared._

## `bulkWrite`
- source=`src/mongo/db/commands/bulk_write.idl`, namespace=`ignored`, strict=`True`, api_version=`1`, reply=`BulkWriteCommandReply`
- Parser for the 'bulkWrite' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `ops` | `array<variant<BulkWriteInsertOp, BulkWriteUpdateOp, BulkWriteDeleteOp>>` | `False` | `` | `False` | `` | `` |
| `nsInfo` | `array<NamespaceInfoEntry>` | `False` | `` | `False` | `` | `` |
| `cursor` | `SimpleCursorOptions` | `True` | `` | `False` | `` | `` |
| `bypassDocumentValidation` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `ordered` | `bool` | `False` | `True` | `False` | `` | `` |
| `stmtId` | `int` | `True` | `` | `False` | `` | `` |
| `stmtIds` | `array<int>` | `True` | `` | `False` | `` | `` |
| `bypassEmptyTsReplacement` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `checkMetadataConsistency`
- source=`src/mongo/s/check_metadata_consistency.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``, reply=`CursorInitialReply`
- The public checkMetadataConsistency command on mongos.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `cursor` | `SimpleCursorOptions` | `True` | `` | `False` | `` | `` |

## `cleanupReshardCollection`
- source=`src/mongo/s/request_types/cleanup_reshard_collection.idl`, namespace=`type`, strict=`True`, api_version=``
- The public cleanupReshardCollection command on mongos.
_No IDL fields declared._

## `clearJumboFlag`
- source=`src/mongo/s/commands/cluster_commands.idl`, namespace=`type`, strict=`False`, api_version=``, reply=`OkReply`
- The public clearJumboFlag command on mongos.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `bounds` | `array<object>` | `True` | `` | `False` | `` | `` |
| `find` | `object` | `True` | `` | `False` | `` | `` |

## `clearLog`
- source=`src/mongo/db/commands/generic_servers.idl`, namespace=`type`, strict=`None`, api_version=``, reply=`OkReply`
- Clear the log contents
_No IDL fields declared._

## `cloneCatalogData`
- source=`src/mongo/s/request_types/clone_catalog_data.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal cloneCatalogData command on a shard
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `from` | `string` | `False` | `` | `False` | `` | `` |

## `collMod`
- source=`src/mongo/db/coll_mod.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`CollModReply`
- Specify collMod Command.
_No IDL fields declared._

## `collStats`
- source=`src/mongo/db/dbcommands.idl`, namespace=`concatenate_with_db`, strict=`None`, api_version=``
- { collStats:"blog.posts" , scale : 1 } scale divides sizes e.g. for KB use 1024. avgObjSize - in bytes
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `validationLevel` | `ValidationLevel` | `True` | `` | `False` | `` | `` |

## `commitReshardCollection`
- source=`src/mongo/s/request_types/commit_reshard_collection.idl`, namespace=`type`, strict=`True`, api_version=``
- The public commitReshardCollection command on mongos.
_No IDL fields declared._

## `commitShardSplit`
- source=`src/mongo/db/serverless/shard_split_commands.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Parser for the `commitShardSplit` command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `migrationId` | `uuid` | `False` | `` | `False` | `` | `` |
| `recipientSetName` | `string` | `False` | `` | `False` | `` | `` |
| `recipientTagName` | `string` | `False` | `` | `False` | `` | `` |
| `tenantIds` | `array<tenant_id>` | `False` | `` | `False` | `` | `{"callback": "tenant_migration_util::validateDatabasePrefix"}` |

## `commitTransaction`
- source=`src/mongo/db/commands/txn_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=`1`, reply=`OkReply`
- commitTransaction Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `commitTimestamp` | `timestamp` | `True` | `` | `False` | `` | `` |
| `recoveryToken` | `TxnRecoveryToken` | `True` | `` | `False` | `` | `` |

## `compactStructuredEncryptionData`
- source=`src/mongo/db/commands/fle2_compact.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=``, reply=`CompactStructuredEncryptionDataCommandReply`
- Parser for the 'compactStructuredEncryptionData' command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `compactionTokens` | `object` | `False` | `` | `False` | `` | `` |

## `configureCollectionBalancing`
- source=`src/mongo/s/request_types/configure_collection_balancing.idl`, namespace=`type`, strict=`False`, api_version=``, reply=`OkReply`
- Public configureCollectionBalancing command on mongos
_No IDL fields declared._

## `configureQueryAnalyzer`
- source=`src/mongo/s/configure_query_analyzer_cmd.idl`, namespace=`type`, strict=`False`, api_version=``
- The command for setting the query analyzer configuration for a collection.
_No IDL fields declared._

## `connectionStatus`
- source=`src/mongo/db/commands/connection_status.idl`, namespace=`ignored`, strict=`None`, api_version=``, reply=`ConnectionStatusReply`
- Returns information about the current connection
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `showPrivileges` | `safeBool` | `False` | `False` | `False` | `` | `` |

## `coordinateCommitTransaction`
- source=`src/mongo/db/commands/txn_two_phase_commit_cmds.idl`, namespace=`ignored`, strict=`False`, api_version=``
- Parser for the 'coordinateCommitTransaction' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `participants` | `array<CommitParticipant>` | `False` | `` | `False` | `` | `` |

## `count`
- source=`src/mongo/db/query/count_command.idl`, namespace=`concatenate_with_db_or_uuid`, strict=`True`, api_version=`1`, reply=`OkReply`
- Parser for the 'count' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `query` | `object` | `False` | `BSONObj()` | `False` | `` | `` |
| `limit` | `countLimit` | `True` | `` | `False` | `` | `` |
| `skip` | `safeInt64` | `True` | `` | `False` | `` | `{"gte": 0}` |
| `hint` | `indexHint` | `False` | `BSONObj()` | `False` | `` | `` |
| `collation` | `object` | `True` | `` | `False` | `` | `` |
| `fields` | `object` | `False` | `` | `False` | `` | `` |
| `readConcern` | `object` | `True` | `` | `False` | `` | `` |
| `maxTimeMS` | `maxTimeMS` | `True` | `` | `False` | `` | `` |
| `$queryOptions` | `object` | `True` | `` | `False` | `queryOptions` | `` |
| `encryptionInformation` | `EncryptionInformation` | `True` | `` | `False` | `` | `` |
| `mirrored` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `sampleId` | `uuid` | `True` | `` | `False` | `` | `` |

## `create`
- source=`src/mongo/db/commands/create.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`CreateCommandReply`
- Parser for the 'create' Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `capped` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `autoIndexId` | `safeBool` | `True` | `` | `False` | `` | `` |
| `idIndex` | `object` | `True` | `` | `False` | `` | `` |
| `size` | `safeInt64` | `True` | `` | `False` | `` | `{"gte": 1, "lte": 1125899906842624}` |
| `max` | `safeInt64` | `True` | `` | `False` | `` | `{"lt": 2147483648}` |
| `storageEngine` | `object` | `True` | `` | `False` | `` | `` |
| `validator` | `object` | `True` | `` | `False` | `` | `` |
| `validationLevel` | `ValidationLevel` | `True` | `` | `False` | `` | `` |
| `validationAction` | `ValidationAction` | `True` | `` | `False` | `` | `` |
| `indexOptionDefaults` | `IndexOptionDefaults` | `True` | `` | `False` | `` | `` |
| `viewOn` | `string` | `True` | `` | `False` | `` | `{"callback": "create_command_validation::validateViewOnNotEmpty"}` |
| `pipeline` | `array<object>` | `True` | `` | `False` | `` | `` |
| `collation` | `Collation` | `True` | `` | `False` | `` | `` |
| `changeStreamPreAndPostImages` | `ChangeStreamPreAndPostImagesOptions` | `True` | `` | `False` | `` | `` |
| `timeseries` | `TimeseriesOptions` | `True` | `` | `False` | `` | `` |
| `clusteredIndex` | `{'variant': ['safeBool', 'ClusteredIndexSpec']}` | `True` | `` | `False` | `` | `` |
| `expireAfterSeconds` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `encryptedFields` | `EncryptedFieldConfig` | `True` | `` | `False` | `` | `` |
| `temp` | `safeBool` | `True` | `` | `False` | `` | `` |
| `flags` | `safeInt64` | `True` | `` | `False` | `` | `` |

## `createIndexes`
- source=`src/mongo/db/create_indexes.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`CreateIndexesReply`
- Command for creating indexes on a collection
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `v` | `safeInt` | `False` | `2` | `False` | `` | `` |
| `indexes` | `array<object>` | `False` | `` | `False` | `` | `` |
| `ignoreUnknownIndexOptions` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `commitQuorum` | `CommitQuorum` | `True` | `` | `False` | `` | `` |
| `returnOnStart` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `isTimeseriesNamespace` | `bool` | `True` | `` | `False` | `` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `False` | `` | `` |

## `createRole`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Create a new role
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `privileges` | `array<Privilege>` | `False` | `` | `False` | `` | `` |
| `roles` | `array<RoleNameOrString>` | `False` | `` | `False` | `` | `` |
| `authenticationRestrictions` | `array<address_restriction>` | `True` | `` | `False` | `` | `` |

## `createUser`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Create a user
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `pwd` | `string` | `True` | `` | `False` | `` | `` |
| `customData` | `object` | `True` | `` | `False` | `` | `` |
| `roles` | `array<RoleNameOrString>` | `False` | `` | `False` | `` | `` |
| `digestPassword` | `safeBool` | `False` | `True` | `False` | `` | `` |
| `writeConcern` | `object` | `True` | `` | `False` | `` | `` |
| `authenticationRestrictions` | `array<address_restriction>` | `True` | `` | `False` | `` | `` |
| `mechanisms` | `array<string>` | `True` | `` | `False` | `` | `` |

## `dataSize`
- source=`src/mongo/db/dbcommands.idl`, namespace=`type`, strict=`None`, api_version=``, reply=`DataSizeReply`
- Determine data size for a set of data in a certain range. Example: { dataSize:"blog.posts", keyPattern:{x:1}, min:{x:10}, max:{x:55} } {min} and {max} parameters are optional. They must either both be included or both omitted. {keyPattern} is an optional parameter indicating an index pattern that would be useful for iterating over the min/max bounds. If keyPattern is omitted, it is inferred from the structure of min. Note: This command may take a while to run.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `min` | `object` | `True` | `` | `False` | `` | `` |
| `max` | `object` | `True` | `` | `False` | `` | `` |
| `keyPattern` | `object` | `True` | `` | `False` | `` | `` |
| `estimate` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `maxSize` | `safeInt64` | `False` | `0` | `False` | `` | `` |
| `maxObjects` | `safeInt64` | `False` | `0` | `False` | `` | `` |

## `dbStats`
- source=`src/mongo/db/dbcommands.idl`, namespace=`ignored`, strict=`None`, api_version=``, reply=`DBStats`
- Get stats on a database. Not instantaneous. Slower for databases with large .ns files. Example: { dbStats:1, scale:1 }
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `scale` | `safeInt64` | `False` | `1` | `False` | `` | `` |
| `freeStorage` | `safeBool` | `False` | `False` | `False` | `` | `` |

## `delete`
- source=`src/mongo/db/ops/write_ops.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`DeleteCommandReply`
- Parser for the 'delete' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `deletes` | `array<DeleteOpEntry>` | `False` | `` | `False` | `` | `` |
| `let` | `object` | `True` | `` | `False` | `` | `` |
| `runtimeConstants` | `LegacyRuntimeConstants` | `True` | `` | `False` | `legacyRuntimeConstants` | `` |

## `distinct`
- source=`src/mongo/db/query/distinct_command.idl`, namespace=`concatenate_with_db_or_uuid`, strict=`True`, api_version=``
- Parser for the 'distinct' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `string` | `False` | `` | `False` | `` | `` |
| `query` | `object` | `True` | `` | `False` | `` | `` |
| `collation` | `object` | `True` | `` | `False` | `` | `` |
| `mirrored` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `sampleId` | `uuid` | `True` | `` | `False` | `` | `` |

## `donorAbortMigration`
- source=`src/mongo/db/commands/tenant_migration_donor_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Parser for the 'donorAbortMigration' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `migrationId` | `uuid` | `False` | `` | `False` | `` | `` |

## `donorForgetMigration`
- source=`src/mongo/db/commands/tenant_migration_donor_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Parser for the 'donorForgetMigration' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `migrationId` | `uuid` | `False` | `` | `False` | `` | `` |

## `donorStartMigration`
- source=`src/mongo/db/commands/tenant_migration_donor_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Parser for the 'donorStartMigration' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `migrationId` | `uuid` | `False` | `` | `False` | `` | `` |
| `recipientConnectionString` | `string` | `False` | `` | `False` | `` | `{"callback": "tenant_migration_util::validateConnectionString"}` |
| `tenantId` | `string` | `True` | `` | `False` | `` | `{"callback": "tenant_migration_util::validateDatabasePrefix"}` |
| `readPreference` | `readPreference` | `False` | `` | `False` | `` | `` |
| `donorCertificateForRecipient` | `TenantMigrationPEMPayload` | `True` | `` | `False` | `` | `` |
| `recipientCertificateForDonor` | `TenantMigrationPEMPayload` | `True` | `` | `False` | `` | `` |
| `protocol` | `MigrationProtocol` | `True` | `` | `False` | `` | `{"callback": "tenant_migration_util::validateProtocolFCVCompatibility"}` |
| `tenantIds` | `array<tenant_id>` | `True` | `` | `False` | `` | `{"callback": "tenant_migration_util::validateDatabasePrefix"}` |

## `drop`
- source=`src/mongo/db/drop.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`DropReply`
- Parser for the drop command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `collectionUUID` | `uuid` | `True` | `` | `False` | `` | `` |

## `dropAllRolesFromDatabase`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`ignored`, strict=`True`, api_version=``, reply=`dropAllRolesFromDatabaseReply`
- Drops all roles from the given database. Before deleting the roles completely it must remove them from any users or other roles that reference them. If any errors occur in the middle of that process it's possible to be left in a state where the roles have been removed from some user/roles but otherwise still exist.
_No IDL fields declared._

## `dropAllUsersFromDatabase`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`ignored`, strict=`True`, api_version=``, reply=`dropAllUsersFromDatabaseReply`
- Drop all users in the database
_No IDL fields declared._

## `dropConnections`
- source=`src/mongo/db/commands/drop_connections.idl`, namespace=`ignored`, strict=`None`, api_version=``
- An administrative command which takes a list of host and ports and drops pooled connections to them
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `hostAndPort` | `array<HostAndPort>` | `False` | `` | `False` | `` | `` |

## `dropDatabase`
- source=`src/mongo/db/drop_database.idl`, namespace=`type`, strict=`True`, api_version=`1`, reply=`OkReply`
- Parser for the dropDatabase command
_No IDL fields declared._

## `dropIndexes`
- source=`src/mongo/db/drop_indexes.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`DropIndexesReply`
- Parser for the dropIndexes command
_No IDL fields declared._

## `dropRole`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Drops a single role. Before deleting the role completely it must remove it from any users or roles that reference it. If any errors occur in the middle of that process it's possible to be left in a state where the role has been removed from some user/roles but otherwise still exists.
_No IDL fields declared._

## `dropUser`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Drop a single user
_No IDL fields declared._

## `enableSharding`
- source=`src/mongo/s/commands/cluster_commands.idl`, namespace=`type`, strict=`False`, api_version=``, reply=`OkReply`
- The public enableSharding command on mongos.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `primaryShard` | `shard_id` | `True` | `` | `False` | `` | `` |

## `endSessions`
- source=`src/mongo/db/commands/sessions_commands.idl`, namespace=`type`, strict=`True`, api_version=`1`, reply=`OkReply`
- Parser for the endSessions command
_No IDL fields declared._

## `exampleIncrement`
- source=`src/mongo/db/commands_test_example.idl`, namespace=`concatenate_with_db`, strict=`None`, api_version=``
- increment an integer (TypedCommand example)
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `i` | `None` | `False` | `` | `False` | `` | `` |

## `exampleMinimal`
- source=`src/mongo/db/commands_test_example.idl`, namespace=`concatenate_with_db`, strict=`None`, api_version=``
- like exampleIncrement, but use MinimalInvocationBase
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `i` | `None` | `False` | `` | `False` | `` | `` |

## `exampleVoid`
- source=`src/mongo/db/commands_test_example.idl`, namespace=`concatenate_with_db`, strict=`None`, api_version=``
- no return, just side effects
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `i` | `None` | `False` | `` | `False` | `` | `` |

## `explain`
- source=`src/mongo/db/explain.idl`, namespace=`type`, strict=`True`, api_version=`1`, reply=`OkReply`
- Parser for the explain command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `verbosity` | `Verbosity` | `False` | `kExecAllPlans` | `False` | `` | `` |

## `features`
- source=`src/mongo/db/commands/generic_servers.idl`, namespace=`ignored`, strict=`None`, api_version=``, reply=`featuresReply`
- Returns build level feature settings
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `oidReset` | `safeBool` | `True` | `` | `False` | `` | `` |

## `find`
- source=`src/mongo/db/query/find_command.idl`, namespace=`concatenate_with_db_or_uuid`, strict=`True`, api_version=`1`, reply=`CursorInitialReply`
- A struct representing the find command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `filter` | `object_owned_nonempty_serialize` | `False` | `` | `False` | `` | `` |
| `projection` | `object_owned_nonempty_serialize` | `False` | `` | `False` | `` | `` |
| `sort` | `object_owned_nonempty_serialize` | `False` | `` | `False` | `` | `` |
| `hint` | `indexHint` | `False` | `mongo::BSONObj()` | `False` | `` | `` |
| `collation` | `object_owned_nonempty_serialize` | `False` | `` | `False` | `` | `` |
| `skip` | `safeInt64` | `True` | `` | `False` | `` | `{"gte": 0}` |
| `limit` | `safeInt64` | `True` | `` | `False` | `` | `{"gte": 0}` |
| `batchSize` | `safeInt64` | `True` | `` | `False` | `` | `{"gte": 0}` |
| `singleBatch` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `allowDiskUse` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `min` | `object_owned_nonempty_serialize` | `False` | `` | `False` | `` | `` |
| `max` | `object_owned_nonempty_serialize` | `False` | `` | `False` | `` | `` |
| `returnKey` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `showRecordId` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `$queryOptions` | `object_owned_nonempty_serialize` | `False` | `` | `False` | `unwrappedReadPref` | `` |
| `tailable` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `oplogReplay` | `boolNoOpSerializer` | `True` | `` | `False` | `` | `` |
| `noCursorTimeout` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `awaitData` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `allowPartialResults` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `let` | `object_owned` | `True` | `` | `False` | `` | `` |
| `term` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `readOnce` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `allowSpeculativeMajorityRead` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `$_requestResumeToken` | `optionalBool` | `False` | `` | `False` | `requestResumeToken` | `` |
| `$_resumeAfter` | `object_owned_nonempty_serialize` | `False` | `mongo::BSONObj()` | `False` | `resumeAfter` | `` |
| `maxTimeMS` | `maxTimeMS` | `True` | `` | `False` | `` | `{"gte": 0}` |
| `readConcern` | `object_owned` | `True` | `` | `False` | `` | `` |
| `$_startAt` | `object_owned_nonempty_serialize` | `False` | `mongo::BSONObj()` | `False` | `startAt` | `` |
| `runtimeConstants` | `LegacyRuntimeConstants` | `True` | `` | `False` | `legacyRuntimeConstants` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `False` | `` | `` |
| `encryptionInformation` | `EncryptionInformation` | `True` | `` | `False` | `` | `` |
| `mirrored` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `sampleId` | `uuid` | `True` | `` | `False` | `` | `` |

## `findAndModify`
- source=`src/mongo/db/ops/write_ops.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`FindAndModifyCommandReply`
- Parser for the 'findAndModify' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `query` | `object_owned` | `False` | `mongo::BSONObj()` | `False` | `` | `` |
| `fields` | `object_owned` | `True` | `` | `False` | `` | `` |
| `sort` | `object_owned` | `True` | `` | `False` | `` | `` |
| `batchSize` | `int` | `True` | `` | `False` | `` | `` |
| `singleBatch` | `bool` | `True` | `` | `False` | `` | `` |
| `hint` | `indexHint` | `False` | `mongo::BSONObj()` | `False` | `` | `` |
| `collation` | `object` | `True` | `` | `False` | `` | `` |
| `arrayFilters` | `array<object>` | `True` | `` | `False` | `` | `` |
| `remove` | `safeBool` | `True` | `` | `False` | `` | `` |
| `update` | `update_modification` | `True` | `` | `False` | `` | `` |
| `upsert` | `safeBool` | `True` | `` | `False` | `` | `` |
| `new` | `safeBool` | `True` | `` | `False` | `` | `` |
| `stmtId` | `int` | `True` | `` | `False` | `` | `` |
| `bypassDocumentValidation` | `safeBool` | `True` | `` | `False` | `` | `` |
| `let` | `object` | `True` | `` | `False` | `` | `` |
| `runtimeConstants` | `LegacyRuntimeConstants` | `True` | `` | `False` | `legacyRuntimeConstants` | `` |
| `writeConcern` | `object` | `True` | `` | `False` | `` | `` |
| `encryptionInformation` | `EncryptionInformation` | `True` | `` | `False` | `` | `` |
| `bypassEmptyTsReplacement` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `sampleId` | `uuid` | `True` | `` | `False` | `` | `` |
| `$_originalQuery` | `object` | `True` | `` | `False` | `originalQuery` | `` |
| `$_originalCollation` | `object` | `True` | `` | `False` | `originalCollation` | `` |
| `$_allowShardKeyUpdatesWithoutFullShardKeyInQuery` | `optionalBool` | `False` | `` | `False` | `allowShardKeyUpdatesWithoutFullShardKeyInQuery` | `` |

## `forgetShardSplit`
- source=`src/mongo/db/serverless/shard_split_commands.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Parser for the `forgetShardSplit` command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `migrationId` | `uuid` | `False` | `` | `False` | `` | `` |

## `fsyncUnlock`
- source=`src/mongo/s/commands/cluster_fsync_unlock_cmd.idl`, namespace=`ignored`, strict=`False`, api_version=``
- The command for calling fsync unlock on all shards of a cluster.
_No IDL fields declared._

## `getChangeStreamState`
- source=`src/mongo/db/change_stream_state.idl`, namespace=`ignored`, strict=`None`, api_version=``, reply=`ChangeStreamStateParameters`
- The command to get the state of the change stream in the serverless for a particular tenant.
_No IDL fields declared._

## `getClusterParameter`
- source=`src/mongo/db/commands/cluster_server_parameter_cmds.idl`, namespace=`type`, strict=`False`, api_version=``, reply=`GetClusterParameterReply`
- Retrieves the in-memory value of the specified cluster server parameter(s)
_No IDL fields declared._

## `getCmdLineOpts`
- source=`src/mongo/db/commands/generic_servers.idl`, namespace=`ignored`, strict=`None`, api_version=``, reply=`getCmdLineOptsReply`
- Return the command line options used to launch the server
_No IDL fields declared._

## `getDatabaseVersion`
- source=`src/mongo/s/request_types/get_database_version.idl`, namespace=`type`, strict=`None`, api_version=``
- An internal command to get a shard server's cached database version
_No IDL fields declared._

## `getDefaultRWConcern`
- source=`src/mongo/db/commands/rwc_defaults_commands.idl`, namespace=`ignored`, strict=`None`, api_version=``
- Get the current read/write concern defaults being applied by this node
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `inMemory` | `bool` | `True` | `` | `False` | `` | `` |

## `getLog`
- source=`src/mongo/db/commands/generic_servers.idl`, namespace=`type`, strict=`None`, api_version=``
- Returns most recent log lines from log spooler
_No IDL fields declared._

## `getMore`
- source=`src/mongo/db/query/getmore_command.idl`, namespace=`type`, strict=`True`, api_version=`1`, reply=`CursorGetMoreReply`
- Parser for the getMore command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `collection` | `string` | `False` | `` | `False` | `` | `{"callback": "query_request_helper::validateGetMoreCollectionName"}` |
| `batchSize` | `safeInt64` | `True` | `` | `False` | `` | `{"gte": 0}` |
| `maxTimeMS` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `term` | `long` | `True` | `` | `False` | `` | `` |
| `lastKnownCommittedOpTime` | `optime` | `True` | `` | `False` | `` | `` |

## `getQueryableEncryptionCountInfo`
- source=`src/mongo/db/commands/fle2_get_count_info_command.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=``, reply=`QECountInfosReply`
- Parser for the 'getQueryableEncryptionCountInfo' command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `tokens` | `array<QECountInfoRequestTokenSet>` | `False` | `` | `False` | `` | `` |
| `queryType` | `QECountInfoQueryType` | `False` | `` | `False` | `` | `` |

## `grantPrivilegesToRole`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Grants privileges to a role
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `privileges` | `array<Privilege>` | `False` | `` | `False` | `` | `` |

## `grantRolesToRole`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Grant roles to a role
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `roles` | `array<RoleNameOrString>` | `False` | `` | `False` | `` | `` |

## `grantRolesToUser`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Grant additional roles to a user
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `roles` | `array<RoleNameOrString>` | `False` | `` | `False` | `` | `` |

## `hello`
- source=`src/mongo/db/repl/hello.idl`, namespace=`ignored`, strict=`False`, api_version=`1`, reply=`HelloCommandReply`
- Check if this server is primary for a replica set { hello: 1 }
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `forShell` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `hostInfo` | `string` | `False` | `False` | `False` | `` | `` |
| `hangUpOnStepDown` | `safeBool` | `False` | `True` | `False` | `` | `` |
| `internalClient` | `HelloInternalClientField` | `True` | `` | `False` | `` | `` |
| `client` | `ClientMetadata` | `True` | `` | `False` | `` | `` |
| `topologyVersion` | `TopologyVersion` | `True` | `` | `False` | `` | `` |
| `maxAwaitTimeMS` | `safeInt64` | `True` | `` | `False` | `` | `{"gte": 0}` |
| `helloOk` | `safeBool` | `True` | `` | `False` | `` | `` |
| `compression` | `array<string>` | `True` | `` | `False` | `` | `` |
| `saslSupportedMechs` | `{'variant': ['string', 'object_owned']}` | `True` | `` | `False` | `` | `` |
| `speculativeAuthenticate` | `object` | `True` | `` | `False` | `` | `` |
| `loadBalanced` | `bool` | `True` | `` | `False` | `` | `` |

## `hostInfo`
- source=`src/mongo/db/commands/generic_servers.idl`, namespace=`ignored`, strict=`None`, api_version=``, reply=`hostInfoReply`
- Returns information about the server's host
_No IDL fields declared._

## `httpClientRequest`
- source=`src/mongo/db/commands/http_client.idl`, namespace=`ignored`, strict=`None`, api_version=``
- Fetch an HTTP(S) resource via GET
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `uri` | `string` | `False` | `` | `False` | `` | `` |
| `timeoutSecs` | `int` | `True` | `` | `False` | `` | `{"gt": 0}` |

## `insert`
- source=`src/mongo/db/ops/write_ops.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`InsertCommandReply`
- Parser for the 'insert' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `documents` | `array<object>` | `False` | `` | `False` | `` | `` |

## `internalRenameIfOptionsAndIndexesMatch`
- source=`src/mongo/db/commands/internal_rename_if_options_and_indexes_match.idl`, namespace=`ignored`, strict=`None`, api_version=``
- An internal command that does a rename, but first checks to make sure the indexes and collection options on the destination match those given in the command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `from` | `namespacestring` | `False` | `` | `False` | `` | `` |
| `to` | `namespacestring` | `False` | `` | `False` | `` | `` |
| `collectionOptions` | `object` | `False` | `` | `False` | `` | `` |
| `indexes` | `array<object>` | `False` | `` | `False` | `` | `` |

## `invalidateUserCache`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Invalidate the user cache
_No IDL fields declared._

## `killCursors`
- source=`src/mongo/db/query/kill_cursors.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`KillCursorsCommandReply`
- Kills a specified set of cursors by ID.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `cursors` | `array<cursorId>` | `False` | `` | `False` | `cursorIds` | `` |

## `listCollections`
- source=`src/mongo/db/list_collections.idl`, namespace=`ignored`, strict=`True`, api_version=`1`, reply=`ListCollectionsReply`
- Parser for the listCollections command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `cursor` | `SimpleCursorOptions` | `True` | `` | `False` | `` | `` |
| `filter` | `object` | `True` | `` | `False` | `` | `` |
| `nameOnly` | `bool` | `False` | `False` | `False` | `` | `` |
| `authorizedCollections` | `bool` | `False` | `False` | `False` | `` | `` |
| `includePendingDrops` | `safeBool` | `True` | `` | `False` | `` | `` |

## `listDatabases`
- source=`src/mongo/db/commands/list_databases.idl`, namespace=`ignored`, strict=`True`, api_version=`1`, reply=`ListDatabasesReply`
- listDatabases Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `nameOnly` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `authorizedDatabases` | `bool` | `True` | `` | `False` | `` | `` |
| `filter` | `object` | `True` | `` | `False` | `` | `` |

## `listDatabasesForAllTenants`
- source=`src/mongo/db/commands/list_databases_for_all_tenants.idl`, namespace=`ignored`, strict=`False`, api_version=``, reply=`ListDatabasesForAllTenantsReply`
- listDatabasesForAllTenants Command: lists all databases for all tenants and can only be run if authenticated with internal __system role
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `nameOnly` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `filter` | `object` | `True` | `` | `False` | `` | `` |

## `listIndexes`
- source=`src/mongo/db/list_indexes.idl`, namespace=`concatenate_with_db_or_uuid`, strict=`True`, api_version=`1`, reply=`ListIndexesReply`
- Parser for the listIndexes command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `cursor` | `SimpleCursorOptions` | `True` | `` | `False` | `` | `` |
| `includeBuildUUIDs` | `safeBool` | `True` | `` | `False` | `` | `` |
| `includeIndexBuildInfo` | `safeBool` | `True` | `` | `False` | `` | `` |
| `isTimeseriesNamespace` | `bool` | `True` | `` | `False` | `` | `` |

## `logMessage`
- source=`src/mongo/db/commands/generic.idl`, namespace=`type`, strict=`True`, api_version=``
- Log a message on the server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `extra` | `object` | `True` | `` | `False` | `` | `` |
| `severity` | `MessageSeverity` | `False` | `kLog` | `False` | `` | `` |
| `debugLevel` | `int` | `True` | `` | `False` | `` | `{"gte": 1, "lte": 5}` |

## `logRotate`
- source=`src/mongo/db/commands/generic_servers.idl`, namespace=`type`, strict=`None`, api_version=``, reply=`OkReply`
- Rotate log file(s)
_No IDL fields declared._

## `logout`
- source=`src/mongo/db/commands/authentication_commands.idl`, namespace=`ignored`, strict=`None`, api_version=``
- Deauthenticate from the current database
_No IDL fields declared._

## `mapReduce`
- source=`src/mongo/db/commands/map_reduce.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=``
- The MapReduce command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `map` | `mapReduceJavascriptCodeType` | `False` | `` | `False` | `` | `` |
| `reduce` | `mapReduceJavascriptCodeType` | `False` | `` | `False` | `` | `` |
| `out` | `mapReduceOutOptionsType` | `False` | `` | `False` | `outOptions` | `` |
| `query` | `object` | `True` | `` | `False` | `` | `` |
| `sort` | `object` | `True` | `` | `False` | `` | `` |
| `collation` | `object` | `True` | `` | `False` | `` | `` |
| `limit` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `finalize` | `mapReduceJavascriptCodeNullPermittedType` | `True` | `` | `False` | `` | `` |
| `scope` | `mapReduceGlobalVariableScopeType` | `True` | `` | `False` | `` | `` |
| `verbose` | `bool` | `True` | `` | `False` | `` | `` |
| `bypassDocumentValidation` | `bool` | `True` | `` | `False` | `` | `` |
| `jsMode` | `bool` | `True` | `` | `False` | `` | `` |

## `mergeAllChunksOnShard`
- source=`src/mongo/s/request_types/merge_chunk_request.idl`, namespace=`type`, strict=`False`, api_version=``
- Definition of the mergeAllChunksOnShard command called on routers.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `shard` | `shard_id` | `False` | `` | `False` | `` | `{"callback": "ShardId::validate"}` |
| `maxNumberOfChunksToMerge` | `int` | `False` | `2147483647` | `False` | `` | `` |
| `maxTimeProcessingChunksMS` | `int` | `False` | `2147483647` | `False` | `` | `` |

## `moveChunk`
- source=`src/mongo/s/commands/cluster_commands.idl`, namespace=`type`, strict=`False`, api_version=``
- The public moveChunk command on mongos.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `bounds` | `array<object>` | `True` | `` | `False` | `` | `` |
| `find` | `object` | `True` | `` | `False` | `` | `` |
| `to` | `string` | `False` | `` | `False` | `` | `` |
| `forceJumbo` | `bool` | `False` | `False` | `False` | `` | `` |
| `writeConcern` | `object_owned` | `False` | `BSONObj()` | `False` | `` | `` |
| `secondaryThrottle` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `_secondaryThrottle` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `waitForDelete` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `_waitForDelete` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `moveRange`
- source=`src/mongo/s/request_types/move_range_request.idl`, namespace=`type`, strict=`False`, api_version=``
- Definition of the moveRange command called on routers.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `forceJumbo` | `bool` | `False` | `False` | `False` | `` | `` |
| `secondaryThrottle` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `ping`
- source=`src/mongo/db/commands/generic.idl`, namespace=`ignored`, strict=`True`, api_version=`1`, reply=`OkReply`
- Parser for the 'ping' command.
_No IDL fields declared._

## `prepareTransaction`
- source=`src/mongo/db/commands/txn_two_phase_commit_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Parser for the 'prepareTransaction' command.
_No IDL fields declared._

## `profile`
- source=`src/mongo/db/commands/profile.idl`, namespace=`type`, strict=`True`, api_version=``
- Parser for the 'profile' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `slowms` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `sampleRate` | `safeDouble` | `True` | `` | `False` | `` | `` |
| `filter` | `ObjectOrUnset` | `True` | `` | `False` | `` | `` |

## `recipientForgetMigration`
- source=`src/mongo/db/commands/tenant_migration_recipient_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=``, reply=`OkReply`
- Parser for the 'recipientForgetMigration' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `decision` | `MigrationDecision` | `True` | `` | `False` | `` | `` |

## `recipientSyncData`
- source=`src/mongo/db/commands/tenant_migration_recipient_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=``, reply=`recipientSyncDataResponse`
- Parser for the 'recipientSyncData' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `returnAfterReachingDonorTimestamp` | `timestamp` | `True` | `` | `False` | `` | `{"callback": "tenant_migration_util::validateTimestampNotNull"}` |
| `startMigrationDonorTimestamp` | `timestamp` | `False` | `` | `False` | `` | `{"callback": "tenant_migration_util::validateTimestampNotNull"}` |

## `recipientVoteImportedFiles`
- source=`src/mongo/db/commands/tenant_migration_recipient_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=``, reply=`OkReply`
- An internal mongod command to coordinate recipient steps of a multitenant migration.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `migrationId` | `uuid` | `False` | `` | `False` | `` | `` |
| `from` | `HostAndPort` | `False` | `` | `False` | `` | `` |
| `success` | `safeBool` | `False` | `` | `False` | `` | `` |
| `reason` | `string` | `True` | `` | `False` | `` | `` |

## `refineCollectionShardKey`
- source=`src/mongo/s/commands/refine_collection_shard_key.idl`, namespace=`type`, strict=`False`, api_version=``
- The public refineCollectionShardKey command on mongos
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `object` | `False` | `` | `False` | `` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `False` | `` | `` |
| `enforceUniquenessCheck` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `refreshSessions`
- source=`src/mongo/db/commands/sessions_commands.idl`, namespace=`type`, strict=`True`, api_version=`1`, reply=`OkReply`
- Parser for the refreshSessions command
_No IDL fields declared._

## `renameCollection`
- source=`src/mongo/db/commands/rename_collection.idl`, namespace=`type`, strict=`True`, api_version=``
- Parser for the 'renameCollection' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `to` | `namespacestring` | `False` | `` | `False` | `` | `` |
| `dropTarget` | `{'variant': ['bool', 'uuid']}` | `False` | `False` | `False` | `` | `` |
| `stayTemp` | `bool` | `False` | `False` | `False` | `` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `False` | `` | `` |

## `replSetResizeOplog`
- source=`src/mongo/db/commands/resize_oplog.idl`, namespace=`type`, strict=`True`, api_version=``
- Parser for the 'replSetResizeOplog' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `size` | `safeDouble` | `True` | `` | `False` | `` | `{"gte": 990, "lte": 1073741824}` |
| `minRetentionHours` | `safeDouble` | `True` | `` | `False` | `` | `{"gte": 0}` |

## `replSetTestEgress`
- source=`src/mongo/db/repl/repl_set_test_egress.idl`, namespace=`ignored`, strict=`None`, api_version=``
- Attempt to connect to a cluster member
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `target` | `string` | `True` | `` | `False` | `` | `` |
| `timeoutSecs` | `int` | `False` | `5` | `False` | `` | `{"gt": 0}` |

## `resetPlacementHistory`
- source=`src/mongo/s/request_types/placement_history_commands.idl`, namespace=`ignored`, strict=`False`, api_version=``
- Admin command to reinitialize the content of config.placementHistory based on the current state of the Sharding catalog.
_No IDL fields declared._

## `resetPlacementHistory`
- source=`src/mongo/s/request_types/reset_placement_history.idl`, namespace=`ignored`, strict=`False`, api_version=``
- Admin command to reinitialize the content of config.placementHistory based on the current state of the Sharding catalog.
_No IDL fields declared._

## `reshardCollection`
- source=`src/mongo/s/request_types/reshard_collection.idl`, namespace=`type`, strict=`True`, api_version=``
- The public reshardCollection command on mongos.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `object` | `False` | `` | `False` | `` | `` |
| `unique` | `bool` | `True` | `` | `False` | `` | `` |
| `numInitialChunks` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `collation` | `object` | `True` | `` | `False` | `` | `` |
| `zones` | `array<ReshardingZoneType>` | `True` | `` | `False` | `` | `` |
| `_presetReshardedChunks` | `array<ReshardedChunk>` | `True` | `` | `False` | `` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `False` | `` | `` |

## `revokePrivilegesFromRole`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Grants privileges to a role
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `privileges` | `array<Privilege>` | `False` | `` | `False` | `` | `` |

## `revokeRolesFromRole`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Revoke roles from a role
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `roles` | `array<RoleNameOrString>` | `False` | `` | `False` | `` | `` |

## `revokeRolesFromUser`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Revoke previously assigned roles from a user
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `roles` | `array<RoleNameOrString>` | `False` | `` | `False` | `` | `` |

## `rolesInfo`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``, reply=`rolesInfoReply`
- returns information about roles.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `showPrivileges` | `ParsedPrivilegeFormat` | `False` | `False` | `False` | `` | `` |
| `showBuiltinRoles` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `showAuthenticationRestrictions` | `safeBool` | `False` | `False` | `False` | `` | `` |

## `rotateCertificates`
- source=`src/mongo/db/commands/rotate_certificates.idl`, namespace=`ignored`, strict=`None`, api_version=``
- An administrative command which rotates the certificates to be used by new SSL connections
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `message` | `string` | `True` | `` | `False` | `` | `` |

## `saslContinue`
- source=`src/mongo/db/auth/sasl_commands.idl`, namespace=`ignored`, strict=`True`, api_version=`1`, reply=`SaslReply`
- Continue a SASL based authentication session
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `conversationId` | `int` | `False` | `` | `False` | `` | `` |
| `payload` | `SaslPayload` | `False` | `` | `False` | `` | `` |

## `saslStart`
- source=`src/mongo/db/auth/sasl_commands.idl`, namespace=`ignored`, strict=`True`, api_version=`1`, reply=`SaslReply`
- Begin a SASL based authentication session
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `mechanism` | `string` | `False` | `` | `False` | `` | `` |
| `autoAuthorize` | `safeBool` | `False` | `True` | `False` | `` | `` |
| `options` | `object_owned` | `True` | `` | `False` | `` | `` |
| `payload` | `SaslPayload` | `False` | `` | `False` | `` | `` |

## `setAllowMigrations`
- source=`src/mongo/s/request_types/set_allow_migrations.idl`, namespace=`type`, strict=`False`, api_version=``
- user faced setAllowMigrations command
_No IDL fields declared._

## `setChangeStreamState`
- source=`src/mongo/db/change_stream_state.idl`, namespace=`ignored`, strict=`None`, api_version=``
- The command to set the state of the change stream in the serverless for a particular tenant.
_No IDL fields declared._

## `setClusterParameter`
- source=`src/mongo/db/commands/cluster_server_parameter_cmds.idl`, namespace=`type`, strict=`False`, api_version=``
- Command to set a ClusterServerParameter
_No IDL fields declared._

## `setDefaultRWConcern`
- source=`src/mongo/db/commands/rwc_defaults_commands.idl`, namespace=`ignored`, strict=`None`, api_version=``
- Set the current read/write concern defaults (cluster-wide)
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `defaultReadConcern` | `ReadConcern` | `True` | `` | `False` | `` | `` |
| `defaultWriteConcern` | `WriteConcern` | `True` | `` | `False` | `` | `` |

## `setFeatureCompatibilityVersion`
- source=`src/mongo/db/commands/set_feature_compatibility_version.idl`, namespace=`type`, strict=`True`, api_version=``
- Parser for the 'setFeatureCompatibilityVersion' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `confirm` | `safeBool` | `True` | `` | `False` | `` | `` |
| `fromConfigServer` | `safeBool` | `True` | `` | `False` | `` | `` |
| `phase` | `SetFCVPhase` | `True` | `` | `False` | `` | `` |
| `changeTimestamp` | `timestamp` | `True` | `` | `False` | `` | `` |

## `setIndexCommitQuorum`
- source=`src/mongo/db/commands/set_index_commit_quorum.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=``
- Resets the commitQuorum for an index build.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `indexNames` | `array<string>` | `False` | `` | `False` | `` | `` |
| `commitQuorum` | `CommitQuorum` | `False` | `` | `False` | `` | `` |

## `setProfilingFilterGlobally`
- source=`src/mongo/db/commands/profile.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Parser for the 'setProfilingFilterGlobally' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `filter` | `ObjectOrUnset` | `False` | `` | `False` | `` | `` |

## `setUserWriteBlockMode`
- source=`src/mongo/db/commands/set_user_write_block_mode.idl`, namespace=`ignored`, strict=`True`, api_version=``, reply=`OkReply`
- Set whether user write blocking is enabled.
_No IDL fields declared._

## `shutdown`
- source=`src/mongo/db/commands/shutdown.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Shutdown the database
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `force` | `bool` | `False` | `False` | `False` | `` | `` |
| `timeoutSecs` | `safeInt64` | `False` | `15` | `False` | `` | `` |

## `startRecordingTraffic`
- source=`src/mongo/db/traffic_recorder.idl`, namespace=`ignored`, strict=`None`, api_version=``
- start recording Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `filename` | `string` | `False` | `` | `False` | `` | `` |
| `bufferSize` | `long` | `False` | `134217728` | `False` | `` | `` |
| `maxFileSize` | `long` | `False` | `6294967296` | `False` | `` | `` |

## `stopRecordingTraffic`
- source=`src/mongo/db/traffic_recorder.idl`, namespace=`ignored`, strict=`None`, api_version=``
- stop recording Command
_No IDL fields declared._

## `testInternalTransactions`
- source=`src/mongo/db/commands/internal_transactions_test_command.idl`, namespace=`ignored`, strict=`None`, api_version=``, reply=`TestInternalTransactionsCommandReply`
- The 'testInternalTransactions' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `useClusterClient` | `bool` | `False` | `False` | `False` | `` | `` |
| `commandInfos` | `array<TestInternalTransactionsCommandInfo>` | `False` | `` | `False` | `` | `` |

## `testReshardCloneCollection`
- source=`src/mongo/db/s/resharding_test_commands.idl`, namespace=`type`, strict=`None`, api_version=``
- The 'testReshardCloneCollection' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `atClusterTime` | `timestamp` | `False` | `` | `False` | `` | `` |
| `outputNs` | `namespacestring` | `False` | `` | `False` | `` | `` |
| `shardKey` | `object` | `False` | `` | `False` | `` | `` |
| `shardId` | `shard_id` | `False` | `` | `False` | `` | `` |
| `uuid` | `uuid` | `False` | `` | `False` | `` | `` |

## `transitionFromDedicatedConfigServer`
- source=`src/mongo/s/request_types/transition_from_dedicated_config_server.idl`, namespace=`ignored`, strict=`True`, api_version=``
- The public transitionFromDedicatedConfigServer command on mongos.
_No IDL fields declared._

## `transitionToDedicatedConfigServer`
- source=`src/mongo/s/request_types/transition_to_dedicated_config_server.idl`, namespace=`ignored`, strict=`True`, api_version=``
- mongos command for transitioning to dedicated config server
_No IDL fields declared._

## `update`
- source=`src/mongo/db/ops/write_ops.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`UpdateCommandReply`
- Parser for the 'update' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `updates` | `array<UpdateOpEntry>` | `False` | `` | `False` | `` | `` |
| `let` | `object` | `True` | `` | `False` | `` | `` |
| `runtimeConstants` | `LegacyRuntimeConstants` | `True` | `` | `False` | `legacyRuntimeConstants` | `` |

## `updateRole`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Update an existing role
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `privileges` | `array<Privilege>` | `True` | `` | `False` | `` | `` |
| `roles` | `array<RoleNameOrString>` | `True` | `` | `False` | `` | `` |
| `authenticationRestrictions` | `array<address_restriction>` | `True` | `` | `False` | `` | `` |

## `updateUser`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Modify a user
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `pwd` | `string` | `True` | `` | `False` | `` | `` |
| `customData` | `object` | `True` | `` | `False` | `` | `` |
| `roles` | `array<RoleNameOrString>` | `True` | `` | `False` | `` | `` |
| `digestPassword` | `safeBool` | `False` | `True` | `False` | `` | `` |
| `writeConcern` | `object` | `True` | `` | `False` | `` | `` |
| `authenticationRestrictions` | `array<address_restriction>` | `True` | `` | `False` | `` | `` |
| `mechanisms` | `array<string>` | `True` | `` | `False` | `` | `` |

## `usersInfo`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``, reply=`usersInfoReply`
- Returns information about users.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `showPrivileges` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `showCredentials` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `showAuthenticationRestrictions` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `showCustomData` | `safeBool` | `False` | `True` | `False` | `` | `` |
| `filter` | `object` | `True` | `` | `False` | `` | `` |

## `validateDBMetadata`
- source=`src/mongo/db/commands/validate_db_metadata.idl`, namespace=`ignored`, strict=`None`, api_version=``, reply=`ValidateDBMetadataCommandReply`
- Input request for validateDBMetadata command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `db` | `string` | `True` | `` | `False` | `` | `` |
| `collection` | `string` | `True` | `` | `False` | `` | `` |
| `apiParameters` | `APIParamsForCmd` | `False` | `` | `False` | `` | `` |

## `voteAbortIndexBuild`
- source=`src/mongo/db/commands/vote_index_build.idl`, namespace=`type`, strict=`False`, api_version=``
- An internal mongod command pertaining to cross replica set index builds. Can only be run on primary node to request that an index build, indentified by UUID, be aborted.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `hostAndPort` | `HostAndPort` | `False` | `` | `False` | `` | `` |
| `reason` | `string` | `False` | `` | `False` | `` | `` |

## `voteCommitIndexBuild`
- source=`src/mongo/db/commands/vote_index_build.idl`, namespace=`type`, strict=`False`, api_version=``
- An internal mongod command pertaining to cross replica set index builds. Can only be run on a primary node to signal that the node identified with 'hostAndPort' is ready to commit the index build identified by the UUID in the command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `hostAndPort` | `HostAndPort` | `False` | `` | `False` | `` | `` |

## `waitForFailPoint`
- source=`src/mongo/s/request_types/wait_for_fail_point.idl`, namespace=`type`, strict=`False`, api_version=``
- wait for a fail point to be entered a certain number of times
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `timesEntered` | `safeInt64` | `False` | `` | `False` | `` | `` |
| `maxTimeMS` | `safeInt64` | `False` | `` | `False` | `` | `` |
<!-- END v7_idl_commands.md -->

<!-- BEGIN v7_manual_commands.md -->
# v7 Manual / Non-IDL Command Inventory

## Summary
- Constructor-extracted command definitions: 120

| Primary | Aliases | Class | Base | File |
|---|---|---|---|---|
| `_configsvrAddShard` |  | `ConfigSvrAddShardCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_add_shard_command.cpp` |
| `_configsvrAddShardToZone` |  | `ConfigSvrAddShardToZoneCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_add_shard_to_zone_command.cpp` |
| `_configsvrCommitChunkSplit` |  | `ConfigSvrSplitChunkCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_split_chunk_command.cpp` |
| `_configsvrRemoveShard` |  | `ConfigSvrRemoveShardCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_remove_shard_command.cpp` |
| `_configsvrRemoveShardFromZone` |  | `ConfigSvrRemoveShardFromZoneCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_remove_shard_from_zone_command.cpp` |
| `_configsvrRepairShardedCollectionChunksHistory` |  | `ConfigSvrRepairShardedCollectionChunksHistoryCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_repair_sharded_collection_chunks_history_command.cpp` |
| `_configsvrRunRestore` |  | `ConfigSvrRunRestoreCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_run_restore_command.cpp` |
| `_configsvrTransitionToDedicatedConfigServer` |  | `ConfigSvrTransitionToDedicatedConfigCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_transition_to_dedicated_config_server_command.cpp` |
| `_configsvrUpdateZoneKeyRange` |  | `ConfigsvrUpdateZoneKeyRangeCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_update_zone_key_range_command.cpp` |
| `_getNextSessionMods` |  | `MigrateSessionCommand` | `BasicCommand` | `src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp` |
| `_hashBSONElement` |  | `CmdHashElt` | `BasicCommand` | `src/mongo/db/commands/hashcmd.cpp` |
| `_isSelf` |  | `IsSelfCommand` | `BasicCommand` | `src/mongo/db/commands/isself.cpp` |
| `_migrateClone` |  | `InitialCloneCommand` | `BasicCommand` | `src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp` |
| `_recvChunkAbort` |  | `RecvChunkAbortCommand` | `BasicCommand` | `src/mongo/db/s/migration_destination_manager_commands.cpp` |
| `_recvChunkCommit` |  | `RecvChunkCommitCommand` | `BasicCommand` | `src/mongo/db/s/migration_destination_manager_commands.cpp` |
| `_recvChunkReleaseCritSec` |  | `RecvChunkReleaseCritSecCommand` | `BasicCommand` | `src/mongo/db/s/migration_destination_manager_commands.cpp` |
| `_recvChunkStart` |  | `RecvChunkStartCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/migration_destination_manager_commands.cpp` |
| `_recvChunkStatus` |  | `RecvChunkStatusCommand` | `BasicCommand` | `src/mongo/db/s/migration_destination_manager_commands.cpp` |
| `_shardsvrCloneCatalogData` | `_cloneCatalogData` | `CloneCatalogDataCommand` | `BasicCommand` | `src/mongo/db/s/clone_catalog_data_command.cpp` |
| `_shardsvrMovePrimary` |  | `MovePrimaryCommand` | `BasicCommand` | `src/mongo/db/s/shardsvr_move_primary_command.cpp` |
| `_transferMods` |  | `TransferModsCommand` | `BasicCommand` | `src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp` |
| `addShard` | `addshard` | `AddShardCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_add_shard_cmd.cpp` |
| `addShardToZone` | `addshardtozone` | `AddShardToZoneCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_add_shard_to_zone_cmd.cpp` |
| `appendOplogNote` |  | `AppendOplogNoteCmd` | `BasicCommand` | `src/mongo/db/commands/oplog_note.cpp` |
| `appendOplogNote` |  | `AppendOplogNoteCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_oplog_note_cmd.cpp` |
| `applyOps` |  | `ApplyOpsCmd` | `BasicCommand` | `src/mongo/db/commands/apply_ops_cmd.cpp` |
| `buildInfo` | `buildinfo` | `CmdBuildInfo` | `BasicCommand` | `src/mongo/db/commands/dbcommands.cpp` |
| `buildInfo` | `buildinfo` | `ClusterCmdBuildInfo` | `BasicCommand` | `src/mongo/s/commands/cluster_build_info.cpp` |
| `captrunc` |  | `CapTrunc` | `BasicCommand` | `src/mongo/db/commands/test_commands.cpp` |
| `checkShardingIndex` |  | `CheckShardingIndex` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/check_sharding_index_command.cpp` |
| `cleanupOrphaned` |  | `CleanupOrphanedCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/cleanup_orphaned_cmd.cpp` |
| `cloneCollectionAsCapped` |  | `CmdCloneCollectionAsCapped` | `BasicCommand` | `src/mongo/db/commands/collection_to_capped.cpp` |
| `collStats` | `collstats` | `CollectionStats` | `BasicCommand` | `src/mongo/s/commands/cluster_coll_stats_cmd.cpp` |
| `compact` |  | `CompactCmd` | `BasicCommand` | `src/mongo/db/commands/compact.cpp` |
| `compact` |  | `CompactCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_compact_cmd.cpp` |
| `configureFailPoint` |  | `FaultInjectCmd` | `BasicCommand` | `src/mongo/db/commands/fail_point_cmd.cpp` |
| `connPoolStats` |  | `PoolStats` | `BasicCommand` | `src/mongo/db/commands/conn_pool_stats.cpp` |
| `connPoolSync` | `connpoolsync` | `PoolFlushCmd` | `BasicCommand` | `src/mongo/db/commands/conn_pool_sync.cpp` |
| `convertToCapped` |  | `CmdConvertToCapped` | `BasicCommand` | `src/mongo/db/commands/collection_to_capped.cpp` |
| `convertToCapped` |  | `ConvertToCappedCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_convert_to_capped_cmd.cpp` |
| `count` |  | `CmdCount` | `BasicCommand` | `src/mongo/db/commands/count_cmd.cpp` |
| `cpuload` |  | `CPULoadCommand` | `BasicCommand` | `src/mongo/db/commands/cpuload.cpp` |
| `cst` |  | `CstCommand` | `BasicCommand` | `src/mongo/db/commands/cst_command.cpp` |
| `currentOp` |  | `CurrentOpCommandBase` | `BasicCommand` | `src/mongo/db/commands/current_op_common.h` |
| `dbCheck` |  | `DbCheckCmd` | `BasicCommand` | `src/mongo/db/commands/dbcheck.cpp` |
| `dbHash` | `dbhash` | `DBHashCmd` | `BasicCommand` | `src/mongo/db/commands/dbhash.cpp` |
| `distinct` |  | `DistinctCommand` | `BasicCommand` | `src/mongo/db/commands/distinct.cpp` |
| `distinct` |  | `DistinctCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_distinct_cmd.cpp` |
| `emptycapped` |  | `EmptyCapped` | `BasicCommand` | `src/mongo/db/commands/test_commands.cpp` |
| `filemd5` |  | `CmdFileMD5` | `BasicCommand` | `src/mongo/db/commands/dbcommands_d.cpp` |
| `filemd5` |  | `FileMD5Cmd` | `BasicCommand` | `src/mongo/s/commands/cluster_filemd5_cmd.cpp` |
| `findAndModify` | `findandmodify` | `FindAndModifyCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_find_and_modify_cmd.h` |
| `flushRouterConfig` | `flushrouterconfig` | `FlushRouterConfigCmd` | `BasicCommand` | `src/mongo/s/commands/flush_router_config_cmd.cpp` |
| `fsync` |  | `FSyncCommand` | `BasicCommand` | `src/mongo/db/commands/fsync.cpp` |
| `fsync` |  | `FsyncCommand` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_fsync_cmd.cpp` |
| `fsyncUnlock` |  | `FSyncUnlockCommand` | `BasicCommand` | `src/mongo/db/commands/fsync.cpp` |
| `getDiagnosticData` |  | `GetDiagnosticDataCommand` | `BasicCommand` | `src/mongo/db/ftdc/ftdc_commands.cpp` |
| `getDiagnosticData` |  | `GetDiagnosticDataCommand` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_ftdc_commands.cpp` |
| `getLog` |  | `GetLogCmd` | `BasicCommand` | `src/mongo/db/commands/generic_servers.cpp` |
| `getParameter` |  | `CmdGet` | `BasicCommand` | `src/mongo/db/commands/parameters.cpp` |
| `getShardMap` |  | `CmdGetShardMap` | `BasicCommand` | `src/mongo/s/commands/get_shard_map_cmd.cpp` |
| `getShardVersion` |  | `GetShardVersion` | `BasicCommand` | `src/mongo/db/s/get_shard_version_command.cpp` |
| `getShardVersion` | `getshardversion` | `GetShardVersion` | `BasicCommand` | `src/mongo/s/commands/cluster_get_shard_version_cmd.cpp` |
| `godinsert` |  | `GodInsert` | `BasicCommand` | `src/mongo/db/commands/test_commands.cpp` |
| `isdbgrid` |  | `IsDbGridCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_is_db_grid_cmd.cpp` |
| `killAllSessions` |  | `KillAllSessionsCommand` | `BasicCommand` | `src/mongo/db/commands/kill_all_sessions_command.cpp` |
| `killAllSessionsByPattern` |  | `KillAllSessionsByPatternCommand` | `BasicCommand` | `src/mongo/db/commands/kill_all_sessions_by_pattern_command.cpp` |
| `killOp` |  | `KillOpCmdBase` | `BasicCommand` | `src/mongo/db/commands/kill_op_cmd_base.h` |
| `killSessions` |  | `KillSessionsCommand` | `BasicCommand` | `src/mongo/db/commands/kill_sessions_command.cpp` |
| `listCommands` |  | `ListCommandsCmd` | `BasicCommand` | `src/mongo/db/commands/generic.cpp` |
| `listShards` | `listshards` | `ListShardsCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_list_shards_cmd.cpp` |
| `lockInfo` |  | `CmdLockInfo` | `BasicCommand` | `src/mongo/db/commands/lock_info.cpp` |
| `makeSnapshot` |  | `CmdMakeSnapshot` | `BasicCommand` | `src/mongo/db/commands/snapshot_management.cpp` |
| `mapReduce` | `mapreduce` | `MapReduceCommandBase` | `BasicCommand` | `src/mongo/db/commands/map_reduce_command_base.h` |
| `mergeChunks` | `_shardsvrMergeChunks` | `MergeChunksCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/shardsvr_merge_chunks_command.cpp` |
| `mergeChunks` |  | `ClusterMergeChunksCommand` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_merge_chunks_cmd.cpp` |
| `movePrimary` | `moveprimary` | `MoveDatabasePrimaryCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_move_primary_cmd.cpp` |
| `multicast` |  | `ClusterMulticastCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_multicast_cmd.cpp` |
| `netstat` |  | `NetStatCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_netstat_cmd.cpp` |
| `pinHistoryReplicated` |  | `DurableHistoryReplicatedTestCmd` | `BasicCommand` | `src/mongo/db/commands/test_commands.cpp` |
| `planCacheClear` |  | `PlanCacheClearCommand` | `BasicCommand` | `src/mongo/db/commands/plan_cache_clear_command.cpp` |
| `planCacheClear` |  | `ClusterPlanCacheClearCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_plan_cache_clear_cmd.cpp` |
| `profile` |  | `ProfileCmdBase` | `BasicCommand` | `src/mongo/db/commands/profile_common.h` |
| `reIndex` |  | `CmdReIndex` | `BasicCommand` | `src/mongo/db/commands/drop_indexes.cpp` |
| `reapLogicalSessionCacheNow` |  | `ReapLogicalSessionCacheNowCommand` | `BasicCommand` | `src/mongo/db/commands/reap_logical_session_cache_now.cpp` |
| `refreshLogicalSessionCacheNow` |  | `RefreshLogicalSessionCacheNowCommand` | `BasicCommand` | `src/mongo/db/commands/refresh_logical_session_cache_now.cpp` |
| `removeShard` | `removeshard` | `RemoveShardCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_remove_shard_cmd.cpp` |
| `removeShardFromZone` | `removeshardfromzone` | `RemoveShardFromZoneCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_remove_shard_from_zone_cmd.cpp` |
| `repairShardedCollectionChunksHistory` |  | `RepairShardedCollectionChunksHistoryCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_repair_sharded_collection_chunks_history_cmd.cpp` |
| `replSetGetStatus` |  | `CmdReplSetGetStatus` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_repl_set_get_status_cmd.cpp` |
| `replSetResizeOplog` |  | `CmdReplSetResizeOplog` | `BasicCommand` | `src/mongo/db/commands/resize_oplog.cpp` |
| `serverStatus` |  | `CmdServerStatus` | `BasicCommand` | `src/mongo/db/commands/server_status_command.cpp` |
| `setCommittedSnapshot` |  | `CmdSetCommittedSnapshot` | `BasicCommand` | `src/mongo/db/commands/snapshot_management.cpp` |
| `setDefaultRWConcern` |  | `ClusterSetDefaultRWConcernCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_rwc_defaults_commands.cpp` |
| `setIndexCommitQuorum` |  | `SetIndexCommitQuorumCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_set_index_commit_quorum_cmd.cpp` |
| `setParameter` |  | `CmdSet` | `BasicCommand` | `src/mongo/db/commands/parameters.cpp` |
| `setProfilingFilterGlobally` |  | `SetProfilingFilterGloballyCmd` | `BasicCommand` | `src/mongo/db/commands/set_profiling_filter_globally_cmd.h` |
| `shardCollection` | `shardcollection` | `ShardCollectionCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_shard_collection_cmd.cpp` |
| `shardingState` |  | `ShardingStateCmd` | `BasicCommand` | `src/mongo/db/s/sharding_state_command.cpp` |
| `sleep` |  | `CmdSleep` | `BasicCommand` | `src/mongo/db/commands/sleep_command.cpp` |
| `split` |  | `SplitCollectionCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_split_cmd.cpp` |
| `splitChunk` | `_shardsvrSplitChunk` | `SplitChunkCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/shardsvr_split_chunk_command.cpp` |
| `splitVector` |  | `SplitVector` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/split_vector_command.cpp` |
| `splitVector` |  | `SplitVectorCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_split_vector_cmd.cpp` |
| `stageDebug` |  | `StageDebugCmd` | `BasicCommand` | `src/mongo/db/exec/stagedebug_cmd.cpp` |
| `startSession` |  | `StartSessionCommand` | `BasicCommand` | `src/mongo/db/commands/start_session_command.cpp` |
| `testDeprecation` |  | `TestDeprecationCmd` | `BasicCommand` | `src/mongo/db/commands/test_deprecation_command.cpp` |
| `testDeprecationInVersion2` |  | `TestDeprecationInVersion2Cmd` | `BasicCommand` | `src/mongo/db/commands/test_api_version_2_commands.cpp` |
| `testRemoval` |  | `TestRemovalCmd` | `BasicCommand` | `src/mongo/db/commands/test_api_version_2_commands.cpp` |
| `testVersion2` |  | `TestVersion2Cmd` | `BasicCommand` | `src/mongo/db/commands/test_api_version_2_commands.cpp` |
| `testVersions1And2` |  | `TestVersions1And2Cmd` | `BasicCommand` | `src/mongo/db/commands/test_api_version_2_commands.cpp` |
| `top` |  | `TopCommand` | `BasicCommand` | `src/mongo/db/commands/top_command.cpp` |
| `transitionToDedicatedConfigServer` |  | `TransitionToDedicatedConfigServerCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_transition_to_dedicated_config_server_cmd.cpp` |
| `updateZoneKeyRange` | `updatezonekeyRange` | `UpdateZoneKeyRangeCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_update_zone_key_range_cmd.cpp` |
| `validate` |  | `ValidateCmd` | `BasicCommand` | `src/mongo/db/commands/validate.cpp` |
| `validate` |  | `ValidateCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_validate_cmd.cpp` |
| `waitForOngoingChunkSplits` |  | `WaitForOngoingChunksSplitsCommand` | `BasicCommand` | `src/mongo/db/s/wait_for_ongoing_chunk_splits_command.cpp` |
| `whatsmysni` |  | `CmdWhatsMySNI` | `BasicCommand` | `src/mongo/db/commands/whats_my_sni_command.cpp` |
| `whatsmyuri` |  | `CmdWhatsMyUri` | `BasicCommand` | `src/mongo/db/commands/whats_my_uri_cmd.cpp` |
| `whatsmyuri` |  | `WhatsMyUriCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_whats_my_uri_cmd.cpp` |
<!-- END v7_manual_commands.md -->

<!-- BEGIN v7_shell_flags.md -->
# v7 Legacy Shell Flags

- Total flags/options: `37`

| Flag | Type | Short | Single | Section | Default | Hidden | Positional | Conflicts | Description |
|---|---|---|---|---|---|---:|---|---|---|
| `apiDeprecationErrors` | `Switch` | `` | `` | `` | `` | `` | `` | `` | disable all features deprecated in the MongoDB Stable API |
| `apiStrict` | `Switch` | `` | `` | `` | `` | `` | `` | `` | disable all features not included in the MongoDB Stable API |
| `apiVersion` | `String` | `` | `` | `` | `` | `` | `` | `` | set the MongoDB API version |
| `authenticationDatabase` | `String` | `` | `` | `Authentication Options` | `` | `` | `` | `` | user source (defaults to dbname) |
| `authenticationMechanism` | `String` | `` | `` | `Authentication Options` | `` | `` | `` | `` | authentication mechanism |
| `autokillop` | `Switch` | `` | `` | `` | `` | `yes` | `` | `` | autokillop |
| `crashOnInvalidBSONError` | `Switch` | `` | `` | `` | `` | `yes` | `` | `` | Crashes the shell if invalid BSON is returned from a call to the server. Must be paired with objcheck to provoke a BSON validation check. |
| `dbaddress` | `String` | `` | `` | `` | `` | `yes` | `1` | `` | dbaddress |
| `disableImplicitSessions` | `Switch` | `` | `` | `` | `` | `` | `` | `` | do not automatically create and use implicit sessions |
| `disableJavaScriptJIT` | `Switch` | `` | `` | `` | `` | `` | `` | `enableJavaScriptJIT` | disable the Javascript Just In Time compiler |
| `disableJavaScriptProtection` | `Switch` | `` | `` | `` | `` | `` | `` | `enableJavaScriptProtection` | allow automatic JavaScript function marshalling |
| `enableJavaScriptJIT` | `Switch` | `` | `` | `` | `` | `` | `` | `disableJavaScriptJIT` | enable the Javascript Just In Time compiler |
| `enableJavaScriptProtection` | `Switch` | `` | `` | `` | `` | `yes` | `` | `disableJavaScriptProtection` | disable automatic JavaScript function marshalling (defaults to true) |
| `eval` | `String` | `` | `` | `` | `` | `` | `` | `` | evaluate javascript |
| `files` | `StringVector` | `` | `` | `` | `` | `yes` | `2-` | `` | files |
| `gssapiHostName` | `String` | `` | `` | `Authentication Options` | `` | `` | `` | `` | Remote host name to use for purpose of GSSAPI/Kerberos authentication |
| `gssapiServiceName` | `String` | `` | `` | `Authentication Options` | `mongodb` | `` | `` | `` | Service name to use when authenticating using GSSAPI/Kerberos |
| `help` | `Switch` | `help` | `h` | `` | `` | `` | `` | `` | show this usage information |
| `host` | `String` | `` | `` | `` | `` | `` | `` | `` | server to connect to |
| `idleSessionTimeout` | `Int` | `` | `` | `` | `0` | `` | `` | `` | Terminate the Shell session if it's been idle for this many seconds |
| `ipv6` | `Switch` | `` | `` | `` | `` | `` | `` | `` | enable IPv6 support (disabled by default) |
| `jsHeapLimitMB` | `Int` | `` | `` | `` | `` | `` | `` | `` | set the js scope's heap size limit |
| `nodb` | `Switch` | `` | `` | `` | `` | `` | `` | `` | don't connect to mongod on startup - no 'db address' arg expected |
| `nokillop` | `Switch` | `` | `` | `` | `` | `yes` | `` | `` | nokillop |
| `noobjcheck` | `Switch` | `` | `` | `` | `` | `yes` | `` | `objcheck` | do NOT inspect client data for validity on receipt (DEFAULT) |
| `norc` | `Switch` | `` | `` | `` | `` | `` | `` | `` | will not run the ".mongorc.js" file on start up |
| `objcheck` | `Switch` | `` | `` | `` | `` | `yes` | `` | `noobjcheck` | inspect client data for validity on receipt |
| `oidcAccessToken` | `String` | `` | `` | `` | `` | `` | `` | `` | If set, the shell will pass this token to the server for any user that tries authenticating with the MONGODB-OIDC mechanism. This will bypass the device authorization grant flow. |
| `password` | `String` | `password` | `p` | `Authentication Options` | `` | `` | `` | `` | password for authentication |
| `port` | `String` | `` | `` | `` | `` | `` | `` | `` | port to connect to |
| `quiet` | `Switch` | `` | `` | `` | `` | `` | `` | `` | be less chatty |
| `retryWrites` | `Switch` | `` | `` | `` | `` | `` | `` | `` | automatically retry write operations upon transient network errors |
| `setShellParameter` | `StringMap` | `` | `` | `` | `` | `yes` | `` | `` | Set a configurable parameter |
| `shell` | `Switch` | `` | `` | `` | `` | `` | `` | `` | run the shell after executing files |
| `username` | `String` | `username` | `u` | `Authentication Options` | `` | `` | `` | `` | username for authentication |
| `verbose` | `Switch` | `` | `` | `` | `` | `` | `` | `` | increase verbosity |
| `version` | `Switch` | `` | `` | `` | `` | `` | `` | `` | show version information |
<!-- END v7_shell_flags.md -->

<!-- BEGIN v6_to_v7_shell_flag_diff.md -->
# Shell Flag Diff v6 -> v7

- Added: `1`
- Removed: `0`
- Changed: `0`

## Added

- `oidcAccessToken`
<!-- END v6_to_v7_shell_flag_diff.md -->
