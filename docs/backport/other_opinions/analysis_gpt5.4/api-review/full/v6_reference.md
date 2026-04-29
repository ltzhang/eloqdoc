# V6 Full Reference

<!-- BEGIN v6.md -->
# MongoDB v6.0 API Workbook

Representative branch: `origin/v6.0`

v6 expands the typed command surface heavily, but it does not introduce a parser-envelope break on the scale of v5 or v8.

## Primary source anchors

- `src/mongo/db/service_entry_point_common.cpp`
- `src/mongo/db/initialize_api_parameters.cpp`
- `src/mongo/db/api_parameters.idl`
- `src/mongo/shell/db.js`
- `src/mongo/shell/shell_options.idl`

## Command-entry delta from v5

Observed parse sequence in v6:

1. `initializeAPIParameters(request.body, command)`
2. store `APIParameters::fromClient(...)`
3. `CommandHelpers::uassertShouldAttemptParse(...)`
4. `getClientOperationTime(opCtx)`
5. `command->parse(opCtx, request)`
6. `CommandInvocation::set(opCtx, _invocation)`
7. later in execution, `rpc::readRequestMetadata(opCtx, request, command->requiresAuth())`

Key points:

- Stable API parsing and validation semantics remain the v5 model.
- The `readRequestMetadata()` call shape changes, but it is still not moved ahead of command parse yet.
- v6 is mostly a surface-area release, not a request-envelope redesign.

## Server command surface

Exhaustive catalogs:

- `generated/v6_idl_commands.md`
- `generated/v6_manual_commands.md`

Adjacent diffs against v5:

- `generated/v5_to_v6_idl_diff.md`
- `generated/v5_to_v6_manual_diff.md`

Counts:

- IDL commands: `165`
- Manual / non-IDL command definitions: `134`

Porting implication:

- The majority of v5 -> v6 work is command and field coverage.
- The IDL diff is the primary implementation checklist here.
- The manual diff is smaller than v4 -> v5 and is mostly internal.

## Legacy shell delta from v5

Exhaustive shell option inventory and diff:

- `generated/v6_shell_flags.md`
- `generated/v5_to_v6_shell_flag_diff.md`

Count:

- shell flags/options: `36`

Changes from v5:

- added: `crashOnInvalidBSONError`
- removed:
  - `readMode`
  - `rpcProtocols`
  - `useLegacyWriteOps`
  - `writeMode`

This is meaningful implementation-wise:

- the old explicit legacy/compatibility/command mode switches disappear from the shell CLI
- by v6, the command path is expected to be the normal path rather than a compatibility mode

The API flags remain:

- `--apiVersion`
- `--apiStrict`
- `--apiDeprecationErrors`

The descriptions in the shell option IDL change from “Versioned API” wording to “Stable API” wording.

## `mongosh` relevance for v6 servers

Current `mongosh` exposes the clearest v6-only client additions:

- `Collection.getSearchIndexes()`
- `Collection.createSearchIndex()`
- `Collection.createSearchIndexes()`
- `Collection.dropSearchIndex()`
- `Collection.updateSearchIndex()`

All are gated to `6.0+`.

These methods are backed by the service-provider layer rather than handwritten legacy-shell wrappers, which makes them easier to map to new client functionality than the older in-tree shell helpers.

See:

- `mongosh.md`
- `generated/mongosh_shell_api_inventory.md`
- `generated/mongosh_method_signatures.md`

## Porting checklist from v5

1. Apply every added/changed command and field from `generated/v5_to_v6_idl_diff.md`.
2. Apply the smaller non-IDL delta from `generated/v5_to_v6_manual_diff.md`.
3. Keep the v5 Stable API envelope unchanged.
4. Remove or stop depending on the old shell compatibility flags (`readMode`, `writeMode`, `rpcProtocols`, `useLegacyWriteOps`) if you are matching v6 CLI behavior.
5. Add `crashOnInvalidBSONError` if you are matching v6 shell options.
6. Add the v6 client-side search-index helpers if you are also matching the `mongosh` surface.
<!-- END v6.md -->

<!-- BEGIN v5_to_v6_idl_diff.md -->
# IDL Diff v5 -> v6

- Added commands: 35
- Removed commands: 1
- Changed commands: 31

## Added commands

- `_configsvrCollMod`
- `_configsvrCommitChunksMerge`
- `_configsvrCommitMovePrimary`
- `_configsvrConfigureCollectionBalancing`
- `_configsvrMoveRange`
- `_configsvrSetClusterParameter`
- `_configsvrSetUserWriteBlockMode`
- `_shardsvrCollMod`
- `_shardsvrCollModParticipant`
- `_shardsvrDropIndexes`
- `_shardsvrGetStatsForBalancing`
- `_shardsvrJoinMigrations`
- `_shardsvrMoveRange`
- `_shardsvrParticipantBlock`
- `_shardsvrSetClusterParameter`
- `_shardsvrSetUserWriteBlockMode`
- `abortShardSplit`
- `clearLog`
- `commitShardSplit`
- `compactStructuredEncryptionData`
- `configureCollectionBalancing`
- `connectionStatus`
- `enableSharding`
- `features`
- `forgetShardSplit`
- `getClusterParameter`
- `getCmdLineOpts`
- `getLog`
- `hostInfo`
- `logRotate`
- `moveRange`
- `recipientVoteImportedFiles`
- `setClusterParameter`
- `setUserWriteBlockMode`
- `testInternalTransactions`

## Removed commands

- `_cloneCollectionOptionsFromPrimaryShard`

## Changed commands

### `_configsvrCreateDatabase`
- Fields modified: `primaryShardId`

### `_configsvrEnsureChunkVersionIsGreaterThan`
- Fields modified: `collectionUUID`, `nss`, `version`

### `_configsvrRefineCollectionShardKey`
- Fields removed: `isFromPrimaryShard`

### `_shardsvrDropCollection`
- Fields added: `collectionUUID`

### `_shardsvrDropCollectionParticipant`
- Fields added: `fromMigrate`

### `_shardsvrRefineCollectionShardKey`
- Fields removed: `enforceUniquenessCheck`, `newShardKey`

### `_shardsvrRenameCollection`
- Fields added: `allowEncryptedCollectionRename`

### `_shardsvrReshardCollection`
- Fields removed: `_presetReshardedChunks`, `collation`, `key`, `numInitialChunks`, `unique`, `zones`

### `aggregate`
- Fields added: `$_generateV2ResumeTokens`, `encryptionInformation`
- Fields removed: `use44SortKeys`, `useNewUpsert`

### `autoSplitVector`
- Fields added: `limit`

### `clearJumboFlag`
- Top-level changed: `reply_type`, `description`

### `collMod`
- Fields removed: `expireAfterSeconds`, `index`, `isTimeseriesNamespace`, `pipeline`, `recordPreImages`, `timeseries`, `validationAction`, `validationLevel`, `validator`, `viewOn`

### `count`
- Fields added: `encryptionInformation`

### `create`
- Fields added: `changeStreamPreAndPostImages`, `encryptedFields`
- Fields modified: `clusteredIndex`

### `createIndexes`
- Fields added: `collectionUUID`
- Fields modified: `ignoreUnknownIndexOptions`

### `createUser`
- Fields added: `$tenant`

### `delete`
- Fields modified: `runtimeConstants`

### `donorStartMigration`
- Fields added: `protocol`
- Fields modified: `tenantId`

### `drop`
- Fields added: `collectionUUID`

### `dropIndexes`
- Fields removed: `index`, `isTimeseriesNamespace`

### `explain`
- Fields removed: `collation`, `use44SortKeys`, `useNewUpsert`

### `find`
- Fields added: `collectionUUID`, `encryptionInformation`
- Fields removed: `_use44SortKeys`

### `findAndModify`
- Fields added: `encryptionInformation`, `stmtId`

### `getMore`
- Fields modified: `batchSize`, `lastKnownCommittedOpTime`, `maxTimeMS`, `term`

### `listIndexes`
- Fields added: `includeIndexBuildInfo`

### `profile`
- Fields modified: `sampleRate`

### `refineCollectionShardKey`
- Fields added: `collectionUUID`

### `renameCollection`
- Fields added: `collectionUUID`
- Fields modified: `dropTarget`

### `replSetResizeOplog`
- Fields modified: `minRetentionHours`, `size`

### `reshardCollection`
- Fields added: `collectionUUID`

### `usersInfo`
- Fields added: `showCustomData`
<!-- END v5_to_v6_idl_diff.md -->

<!-- BEGIN v5_to_v6_manual_diff.md -->
# Manual Command Diff v5 -> v6

- Added primary names: 2
- Removed primary names: 17

## Added primary names

- `_configsvrRunRestore`
- `_recvChunkReleaseCritSec`

## Removed primary names

- `_configsvrCommitChunkMerge`
- `_configsvrCommitChunksMerge`
- `_configsvrCommitMovePrimary`
- `_configsvrDropCollection`
- `_configsvrDropDatabase`
- `_configsvrEnableSharding`
- `_configsvrMovePrimary`
- `_configsvrShardCollection`
- `_shardsvrShardCollection`
- `clearLog`
- `connectionStatus`
- `enableSharding`
- `features`
- `getCmdLineOpts`
- `hostInfo`
- `logRotate`
- `sbe`
<!-- END v5_to_v6_manual_diff.md -->

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

<!-- BEGIN v6_idl_commands.md -->
# v6 IDL Command Inventory

## Summary
- Command count: 165

## `_addShard`
- source=`src/mongo/db/s/add_shard_cmd.idl`, namespace=`ignored`, strict=`None`, api_version=``
- _addShard Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `shardIdentity` | `ShardIdentity` | `False` | `` | `False` | `` | `` |

## `_configsvrAbortReshardCollection`
- source=`src/mongo/s/request_types/abort_reshard_collection.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal abortReshardCollection command on the config server.
_No IDL fields declared._

## `_configsvrBalancerCollectionStatus`
- source=`src/mongo/s/request_types/balancer_collection_status.idl`, namespace=`type`, strict=`True`, api_version=``
- Internal balancerCollectionStatus command on the config server
_No IDL fields declared._

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

## `_configsvrCommitChunksMerge`
- source=`src/mongo/s/request_types/merge_chunk_request.idl`, namespace=`type`, strict=`False`, api_version=``, reply=`ConfigSvrMergeResponse`
- The internal _configsvrCommitChunksMerge command on the config server.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `shard` | `shard_id` | `False` | `` | `False` | `` | `` |
| `collUUID` | `my_uuid` | `False` | `` | `False` | `collectionUUID` | `` |
| `chunkRange` | `chunk_range` | `False` | `` | `False` | `chunkRange` | `` |
| `validAfter` | `timestamp` | `True` | `` | `False` | `` | `` |
| `epoch` | `objectid` | `True` | `` | `False` | `` | `` |
| `timestamp` | `timestamp` | `True` | `` | `False` | `` | `` |

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
- source=`src/mongo/s/request_types/configure_collection_balancing.idl`, namespace=`type`, strict=`True`, api_version=``
- Internal configureCollectionBalancing command on the config server
_No IDL fields declared._

## `_configsvrCreateDatabase`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`type`, strict=`False`, api_version=``, reply=`ConfigsvrCreateDatabaseResponse`
- The internal createDatabase command on the config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `primaryShardId` | `shard_id` | `True` | `` | `False` | `` | `` |
| `enableSharding` | `bool` | `True` | `` | `False` | `` | `` |

## `_configsvrEnsureChunkVersionIsGreaterThan`
- source=`src/mongo/s/request_types/ensure_chunk_version_is_greater_than.idl`, namespace=`ignored`, strict=`False`, api_version=``
- If a chunk matching 'requestedChunk' exists, bumps the chunk's version to one greater than the current collection version.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `minKey` | `object` | `False` | `` | `False` | `` | `` |
| `maxKey` | `object` | `False` | `` | `False` | `` | `` |
| `version` | `ChunkVersionArrayWronglyEncodedAsBSONObjFormat` | `False` | `` | `False` | `` | `` |
| `collectionUUID` | `uuid` | `False` | `` | `False` | `` | `` |
| `nss` | `namespacestring` | `False` | `` | `False` | `` | `` |

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
- source=`src/mongo/db/commands/kill_operations.idl`, namespace=`ignored`, strict=`True`, api_version=``
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

## `_shardsvrAbortReshardCollection`
- source=`src/mongo/s/request_types/abort_reshard_collection.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal command used by resharding coordinator to abort participants.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `userCanceled` | `bool` | `False` | `` | `False` | `` | `` |

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

## `_shardsvrDropCollectionParticipant`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Parser for the _shardsvrDropCollectionParticipant command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `fromMigrate` | `bool` | `True` | `` | `False` | `` | `` |

## `_shardsvrDropDatabase`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`ignored`, strict=`None`, api_version=``
- Internal command sent to the primary shard of a database to drop it.
_No IDL fields declared._

## `_shardsvrDropDatabaseParticipant`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`ignored`, strict=`False`, api_version=``
- Internal command sent to participants shards to drop a database.
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

## `_shardsvrJoinMigrations`
- source=`src/mongo/s/request_types/shardsvr_join_migrations_request.idl`, namespace=`ignored`, strict=`False`, api_version=``
- Command to synch the caller on the completion of any chunk migration activity performed by the shard (as either donor or recipient)
_No IDL fields declared._

## `_shardsvrMoveRange`
- source=`src/mongo/s/request_types/move_range_request.idl`, namespace=`type`, strict=`False`, api_version=``
- Definition of the moveRange command called from the config server on shards.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `epoch` | `objectid` | `False` | `mongo::OID()` | `False` | `` | `` |
| `fromShard` | `shard_id` | `False` | `` | `False` | `` | `{"callback": "validateShardId"}` |
| `maxChunkSizeBytes` | `safeInt64` | `False` | `` | `False` | `` | `` |
| `forceJumbo` | `ForceJumbo` | `False` | `kDoNotForce` | `False` | `` | `` |
| `secondaryThrottle` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `_shardsvrParticipantBlock`
- source=`src/mongo/db/s/participant_block.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal command sent to shards to block CRUD operations
_No IDL fields declared._

## `_shardsvrRefineCollectionShardKey`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Parser for the _shardsvrRefineCollectionShardKey command
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
| `explain` | `explainVerbosity` | `True` | `` | `True` | `` | `` |
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
| `$queryOptions` | `object_owned` | `True` | `` | `True` | `unwrappedReadPref` | `` |
| `$_requestReshardingResumeToken` | `optionalBool` | `False` | `` | `True` | `requestReshardingResumeToken` | `` |
| `exchange` | `ExchangeSpec` | `True` | `` | `False` | `` | `` |
| `runtimeConstants` | `LegacyRuntimeConstants` | `True` | `` | `True` | `legacyRuntimeConstants` | `` |
| `isMapReduceCommand` | `optionalBool` | `False` | `` | `True` | `` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `True` | `` | `` |
| `$_passthroughToShard` | `PassthroughToShardOptions` | `True` | `` | `True` | `passthroughToShard` | `` |
| `$_generateV2ResumeTokens` | `optionalBool` | `False` | `` | `False` | `generateV2ResumeTokens` | `` |
| `encryptionInformation` | `EncryptionInformation` | `True` | `` | `True` | `` | `` |

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
| `tenantIds` | `array<string>` | `False` | `` | `False` | `` | `{"callback": "tenant_migration_util::validateDatabasePrefix"}` |

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
| `fields` | `object` | `False` | `` | `True` | `` | `` |
| `readConcern` | `object` | `True` | `` | `False` | `` | `` |
| `maxTimeMS` | `maxTimeMS` | `True` | `` | `False` | `` | `` |
| `$queryOptions` | `object` | `True` | `` | `False` | `queryOptions` | `` |
| `encryptionInformation` | `EncryptionInformation` | `True` | `` | `True` | `` | `` |

## `create`
- source=`src/mongo/db/commands/create.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`CreateCommandReply`
- Parser for the 'create' Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `capped` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `autoIndexId` | `safeBool` | `True` | `` | `True` | `` | `` |
| `idIndex` | `object` | `True` | `` | `False` | `` | `` |
| `size` | `safeInt64` | `True` | `` | `True` | `` | `{"gte": 1, "lte": 1125899906842624}` |
| `max` | `safeInt64` | `True` | `` | `True` | `` | `{"lt": 2147483648}` |
| `storageEngine` | `object` | `True` | `` | `True` | `` | `` |
| `validator` | `object` | `True` | `` | `False` | `` | `` |
| `validationLevel` | `ValidationLevel` | `True` | `` | `False` | `` | `` |
| `validationAction` | `ValidationAction` | `True` | `` | `False` | `` | `` |
| `indexOptionDefaults` | `IndexOptionDefaults` | `True` | `` | `True` | `` | `` |
| `viewOn` | `string` | `True` | `` | `False` | `` | `{"callback": "create_command_validation::validateViewOnNotEmpty"}` |
| `pipeline` | `array<object>` | `True` | `` | `False` | `` | `` |
| `collation` | `Collation` | `True` | `` | `False` | `` | `` |
| `recordPreImages` | `safeBool` | `True` | `` | `True` | `` | `` |
| `changeStreamPreAndPostImages` | `ChangeStreamPreAndPostImagesOptions` | `True` | `` | `True` | `` | `` |
| `timeseries` | `TimeseriesOptions` | `True` | `` | `False` | `` | `` |
| `clusteredIndex` | `{'variant': ['safeBool', 'ClusteredIndexSpec']}` | `True` | `` | `False` | `` | `` |
| `expireAfterSeconds` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `encryptedFields` | `EncryptedFieldConfig` | `True` | `` | `False` | `` | `` |
| `temp` | `safeBool` | `True` | `` | `True` | `` | `` |
| `flags` | `safeInt64` | `True` | `` | `True` | `` | `` |

## `createIndexes`
- source=`src/mongo/db/create_indexes.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`CreateIndexesReply`
- Command for creating indexes on a collection
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `v` | `safeInt` | `False` | `2` | `False` | `` | `` |
| `indexes` | `array<object>` | `False` | `` | `False` | `` | `` |
| `ignoreUnknownIndexOptions` | `safeBool` | `False` | `False` | `True` | `` | `` |
| `commitQuorum` | `CommitQuorum` | `True` | `` | `False` | `` | `` |
| `isTimeseriesNamespace` | `bool` | `True` | `` | `False` | `` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `True` | `` | `` |

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
| `$tenant` | `tenant_id` | `True` | `` | `False` | `tenantOverride` | `` |

## `delete`
- source=`src/mongo/db/ops/write_ops.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`DeleteCommandReply`
- Parser for the 'delete' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `deletes` | `array<DeleteOpEntry>` | `False` | `` | `False` | `` | `` |
| `let` | `object` | `True` | `` | `False` | `` | `` |
| `runtimeConstants` | `LegacyRuntimeConstants` | `True` | `` | `True` | `legacyRuntimeConstants` | `` |

## `distinct`
- source=`src/mongo/db/query/distinct_command.idl`, namespace=`concatenate_with_db_or_uuid`, strict=`True`, api_version=``
- Parser for the 'distinct' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `string` | `False` | `` | `False` | `` | `` |
| `query` | `object` | `True` | `` | `False` | `` | `` |
| `collation` | `object` | `True` | `` | `False` | `` | `` |

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
| `tenantId` | `string` | `False` | `""` | `False` | `` | `{"callback": "tenant_migration_util::validateDatabasePrefix"}` |
| `readPreference` | `readPreference` | `False` | `` | `False` | `` | `` |
| `donorCertificateForRecipient` | `TenantMigrationPEMPayload` | `True` | `` | `False` | `` | `` |
| `recipientCertificateForDonor` | `TenantMigrationPEMPayload` | `True` | `` | `False` | `` | `` |
| `protocol` | `MigrationProtocol` | `True` | `` | `False` | `` | `{"callback": "tenant_migration_util::validateProtocolFCVCompatibility"}` |

## `drop`
- source=`src/mongo/db/drop.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`DropReply`
- Parser for the drop command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `collectionUUID` | `uuid` | `True` | `` | `True` | `` | `` |

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
| `ntoreturn` | `safeInt64` | `True` | `` | `True` | `` | `{"gte": 0}` |
| `singleBatch` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `allowDiskUse` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `min` | `object_owned_nonempty_serialize` | `False` | `` | `True` | `` | `` |
| `max` | `object_owned_nonempty_serialize` | `False` | `` | `True` | `` | `` |
| `returnKey` | `optionalBool` | `False` | `` | `True` | `` | `` |
| `showRecordId` | `optionalBool` | `False` | `` | `True` | `` | `` |
| `$queryOptions` | `object_owned_nonempty_serialize` | `False` | `` | `True` | `unwrappedReadPref` | `` |
| `tailable` | `optionalBool` | `False` | `` | `True` | `` | `` |
| `oplogReplay` | `boolNoOpSerializer` | `True` | `` | `True` | `` | `` |
| `noCursorTimeout` | `optionalBool` | `False` | `` | `True` | `` | `` |
| `awaitData` | `optionalBool` | `False` | `` | `True` | `` | `` |
| `allowPartialResults` | `optionalBool` | `False` | `` | `False` | `` | `` |
| `let` | `object_owned` | `True` | `` | `False` | `` | `` |
| `options` | `object_owned` | `True` | `` | `True` | `` | `` |
| `term` | `safeInt64` | `True` | `` | `True` | `` | `` |
| `readOnce` | `optionalBool` | `False` | `` | `True` | `` | `` |
| `allowSpeculativeMajorityRead` | `optionalBool` | `False` | `` | `True` | `` | `` |
| `$_requestResumeToken` | `optionalBool` | `False` | `` | `True` | `requestResumeToken` | `` |
| `$_resumeAfter` | `object_owned_nonempty_serialize` | `False` | `mongo::BSONObj()` | `True` | `resumeAfter` | `` |
| `maxTimeMS` | `maxTimeMS` | `True` | `` | `False` | `` | `{"gte": 0}` |
| `readConcern` | `object_owned` | `True` | `` | `False` | `` | `` |
| `runtimeConstants` | `LegacyRuntimeConstants` | `True` | `` | `True` | `legacyRuntimeConstants` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `True` | `` | `` |
| `encryptionInformation` | `EncryptionInformation` | `True` | `` | `True` | `` | `` |

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
| `encryptionInformation` | `EncryptionInformation` | `True` | `` | `True` | `` | `` |
| `bypassEmptyTsReplacement` | `optionalBool` | `False` | `` | `True` | `` | `` |

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
| `term` | `long` | `True` | `` | `True` | `` | `` |
| `lastKnownCommittedOpTime` | `optime` | `True` | `` | `True` | `` | `` |

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
| `includePendingDrops` | `safeBool` | `True` | `` | `True` | `` | `` |

## `listDatabases`
- source=`src/mongo/db/commands/list_databases.idl`, namespace=`ignored`, strict=`True`, api_version=`1`, reply=`ListDatabasesReply`
- listDatabases Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `nameOnly` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `authorizedDatabases` | `bool` | `True` | `` | `False` | `` | `` |
| `filter` | `object` | `True` | `` | `False` | `` | `` |

## `listIndexes`
- source=`src/mongo/db/list_indexes.idl`, namespace=`concatenate_with_db_or_uuid`, strict=`True`, api_version=`1`, reply=`ListIndexesReply`
- Parser for the listIndexes command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `cursor` | `SimpleCursorOptions` | `True` | `` | `False` | `` | `` |
| `includeBuildUUIDs` | `safeBool` | `True` | `` | `True` | `` | `` |
| `includeIndexBuildInfo` | `safeBool` | `True` | `` | `True` | `` | `` |
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
- source=`src/mongo/db/commands/profile.idl`, namespace=`type`, strict=`False`, api_version=``
- Parser for the 'profile' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `slowms` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `sampleRate` | `safeDouble` | `True` | `` | `False` | `` | `` |
| `filter` | `ObjectOrUnset` | `True` | `` | `False` | `` | `` |

## `recipientForgetMigration`
- source=`src/mongo/db/commands/tenant_migration_recipient_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=``
- Parser for the 'recipientForgetMigration' command.
_No IDL fields declared._

## `recipientSyncData`
- source=`src/mongo/db/commands/tenant_migration_recipient_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=``
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
- source=`src/mongo/db/commands/resize_oplog.idl`, namespace=`type`, strict=`False`, api_version=``
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
| `downgradeOnDiskChanges` | `safeBool` | `True` | `` | `False` | `` | `` |
| `fromConfigServer` | `safeBool` | `True` | `` | `False` | `` | `` |
| `phase` | `SetFCVPhase` | `True` | `` | `False` | `` | `` |
| `changeTimestamp` | `timestamp` | `True` | `` | `False` | `` | `` |

## `setIndexCommitQuorum`
- source=`src/mongo/db/commands/set_index_commit_quorum.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
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

## `voteCommitIndexBuild`
- source=`src/mongo/db/commands/vote_commit_index_build.idl`, namespace=`type`, strict=`False`, api_version=``
- An internal mongod command pertaining to cross replica set index builds
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
<!-- END v6_idl_commands.md -->

<!-- BEGIN v6_manual_commands.md -->
# v6 Manual / Non-IDL Command Inventory

## Summary
- Constructor-extracted command definitions: 134

| Primary | Aliases | Class | Base | File |
|---|---|---|---|---|
| `_configsvrAddShard` |  | `ConfigSvrAddShardCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_add_shard_command.cpp` |
| `_configsvrAddShardToZone` |  | `ConfigSvrAddShardToZoneCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_add_shard_to_zone_command.cpp` |
| `_configsvrCommitChunkMigration` |  | `ConfigSvrCommitChunkMigrationCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_commit_chunk_migration_command.cpp` |
| `_configsvrCommitChunkSplit` |  | `ConfigSvrSplitChunkCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_split_chunk_command.cpp` |
| `_configsvrMoveChunk` |  | `ConfigSvrMoveChunkCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_move_chunk_command.cpp` |
| `_configsvrRemoveShard` |  | `ConfigSvrRemoveShardCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_remove_shard_command.cpp` |
| `_configsvrRemoveShardFromZone` |  | `ConfigSvrRemoveShardFromZoneCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_remove_shard_from_zone_command.cpp` |
| `_configsvrRepairShardedCollectionChunksHistory` |  | `ConfigSvrRepairShardedCollectionChunksHistoryCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_repair_sharded_collection_chunks_history_command.cpp` |
| `_configsvrRunRestore` |  | `ConfigSvrRunRestoreCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_run_restore_command.cpp` |
| `_configsvrUpdateZoneKeyRange` |  | `ConfigsvrUpdateZoneKeyRangeCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_update_zone_key_range_command.cpp` |
| `_getNextSessionMods` |  | `MigrateSessionCommand` | `BasicCommand` | `src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp` |
| `_hashBSONElement` |  | `CmdHashElt` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/hashcmd.cpp` |
| `_isSelf` |  | `IsSelfCommand` | `BasicCommand` | `src/mongo/db/commands/isself.cpp` |
| `_migrateClone` |  | `InitialCloneCommand` | `BasicCommand` | `src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp` |
| `_recvChunkAbort` |  | `RecvChunkAbortCommand` | `BasicCommand` | `src/mongo/db/s/migration_destination_manager_legacy_commands.cpp` |
| `_recvChunkCommit` |  | `RecvChunkCommitCommand` | `BasicCommand` | `src/mongo/db/s/migration_destination_manager_legacy_commands.cpp` |
| `_recvChunkReleaseCritSec` |  | `RecvChunkReleaseCritSecCommand` | `BasicCommand` | `src/mongo/db/s/migration_destination_manager_legacy_commands.cpp` |
| `_recvChunkStart` |  | `RecvChunkStartCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/migration_destination_manager_legacy_commands.cpp` |
| `_recvChunkStatus` |  | `RecvChunkStatusCommand` | `BasicCommand` | `src/mongo/db/s/migration_destination_manager_legacy_commands.cpp` |
| `_shardsvrCloneCatalogData` | `_cloneCatalogData` | `CloneCatalogDataCommand` | `BasicCommand` | `src/mongo/db/s/clone_catalog_data_command.cpp` |
| `_shardsvrMovePrimary` |  | `MovePrimaryCommand` | `BasicCommand` | `src/mongo/db/s/shardsvr_move_primary_command.cpp` |
| `_transferMods` |  | `TransferModsCommand` | `BasicCommand` | `src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp` |
| `addShard` | `addshard` | `AddShardCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_add_shard_cmd.cpp` |
| `addShardToZone` | `addshardtozone` | `AddShardToZoneCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_add_shard_to_zone_cmd.cpp` |
| `appendOplogNote` |  | `AppendOplogNoteCmd` | `BasicCommand` | `src/mongo/db/commands/oplog_note.cpp` |
| `appendOplogNote` |  | `AppendOplogNoteCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_oplog_note_cmd.cpp` |
| `applyOps` |  | `ApplyOpsCmd` | `BasicCommand` | `src/mongo/db/commands/apply_ops_cmd.cpp` |
| `availableQueryOptions` | `availablequeryoptions` | `AvailableQueryOptions` | `BasicCommand` | `src/mongo/db/commands/dbcommands_d.cpp` |
| `availableQueryOptions` | `availablequeryoptions` | `AvailableQueryOptions` | `BasicCommand` | `src/mongo/s/commands/cluster_available_query_options_cmd.cpp` |
| `buildInfo` | `buildinfo` | `CmdBuildInfo` | `BasicCommand` | `src/mongo/db/commands/dbcommands.cpp` |
| `buildInfo` | `buildinfo` | `ClusterCmdBuildInfo` | `BasicCommand` | `src/mongo/s/commands/cluster_build_info.cpp` |
| `captrunc` |  | `CapTrunc` | `BasicCommand` | `src/mongo/db/commands/test_commands.cpp` |
| `checkShardingIndex` |  | `CheckShardingIndex` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/check_sharding_index_command.cpp` |
| `cleanupOrphaned` |  | `CleanupOrphanedCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/cleanup_orphaned_cmd.cpp` |
| `cloneCollectionAsCapped` |  | `CmdCloneCollectionAsCapped` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/collection_to_capped.cpp` |
| `collStats` | `collstats` | `CollectionStats` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/dbcommands.cpp` |
| `collStats` | `collstats` | `CollectionStats` | `BasicCommand` | `src/mongo/s/commands/cluster_coll_stats_cmd.cpp` |
| `compact` |  | `CompactCmd` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/compact.cpp` |
| `compact` |  | `CompactCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_compact_cmd.cpp` |
| `configureFailPoint` |  | `FaultInjectCmd` | `BasicCommand` | `src/mongo/db/commands/fail_point_cmd.cpp` |
| `connPoolStats` |  | `PoolStats` | `BasicCommand` | `src/mongo/db/commands/conn_pool_stats.cpp` |
| `connPoolSync` | `connpoolsync` | `PoolFlushCmd` | `BasicCommand` | `src/mongo/db/commands/conn_pool_sync.cpp` |
| `convertToCapped` |  | `CmdConvertToCapped` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/collection_to_capped.cpp` |
| `convertToCapped` |  | `ConvertToCappedCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_convert_to_capped_cmd.cpp` |
| `count` |  | `CmdCount` | `BasicCommand` | `src/mongo/db/commands/count_cmd.cpp` |
| `count` |  | `ClusterCountCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_count_cmd.cpp` |
| `cpuload` |  | `CPULoadCommand` | `BasicCommand` | `src/mongo/db/commands/cpuload.cpp` |
| `cst` |  | `CstCommand` | `BasicCommand` | `src/mongo/db/commands/cst_command.cpp` |
| `currentOp` |  | `CurrentOpCommandBase` | `BasicCommand` | `src/mongo/db/commands/current_op_common.h` |
| `dataSize` | `datasize` | `CmdDatasize` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/dbcommands.cpp` |
| `dataSize` | `datasize` | `DataSizeCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_data_size_cmd.cpp` |
| `dbCheck` |  | `DbCheckCmd` | `BasicCommand` | `src/mongo/db/commands/dbcheck.cpp` |
| `dbHash` | `dbhash` | `DBHashCmd` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/dbhash.cpp` |
| `dbStats` | `dbstats` | `DBStats` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/dbcommands.cpp` |
| `dbStats` | `dbstats` | `DBStatsCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_db_stats_cmd.cpp` |
| `distinct` |  | `DistinctCommand` | `BasicCommand` | `src/mongo/db/commands/distinct.cpp` |
| `distinct` |  | `DistinctCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_distinct_cmd.cpp` |
| `emptycapped` |  | `EmptyCapped` | `BasicCommand` | `src/mongo/db/commands/test_commands.cpp` |
| `filemd5` |  | `CmdFileMD5` | `BasicCommand` | `src/mongo/db/commands/dbcommands_d.cpp` |
| `filemd5` |  | `FileMD5Cmd` | `BasicCommand` | `src/mongo/s/commands/cluster_filemd5_cmd.cpp` |
| `findAndModify` | `findandmodify` | `FindAndModifyCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_find_and_modify_cmd.cpp` |
| `flushRouterConfig` | `flushrouterconfig` | `FlushRouterConfigCmd` | `BasicCommand` | `src/mongo/s/commands/flush_router_config_cmd.cpp` |
| `fsync` |  | `FSyncCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/fsync.cpp` |
| `fsync` |  | `FsyncCommand` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_fsync_cmd.cpp` |
| `fsyncUnlock` |  | `FSyncUnlockCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/fsync.cpp` |
| `getDiagnosticData` |  | `GetDiagnosticDataCommand` | `BasicCommand` | `src/mongo/db/ftdc/ftdc_commands.cpp` |
| `getDiagnosticData` |  | `GetDiagnosticDataCommand` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_ftdc_commands.cpp` |
| `getLastError` | `getlasterror` | `CmdGetLastError` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/get_last_error.cpp` |
| `getLastError` | `getlasterror` | `GetLastErrorCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_get_last_error_cmd.cpp` |
| `getLog` |  | `GetLogCmd` | `BasicCommand` | `src/mongo/db/commands/generic_servers.cpp` |
| `getParameter` |  | `CmdGet` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/parameters.cpp` |
| `getShardMap` |  | `CmdGetShardMap` | `BasicCommand` | `src/mongo/s/commands/get_shard_map_cmd.cpp` |
| `getShardVersion` |  | `GetShardVersion` | `BasicCommand` | `src/mongo/db/s/get_shard_version_command.cpp` |
| `getShardVersion` | `getshardversion` | `GetShardVersion` | `BasicCommand` | `src/mongo/s/commands/cluster_get_shard_version_cmd.cpp` |
| `getnonce` |  | `CmdGetNonce` | `BasicCommand` | `src/mongo/db/commands/authentication_commands.cpp` |
| `godinsert` |  | `GodInsert` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/test_commands.cpp` |
| `isdbgrid` |  | `IsDbGridCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_is_db_grid_cmd.cpp` |
| `killAllSessions` |  | `KillAllSessionsCommand` | `BasicCommand` | `src/mongo/db/commands/kill_all_sessions_command.cpp` |
| `killAllSessionsByPattern` |  | `KillAllSessionsByPatternCommand` | `BasicCommand` | `src/mongo/db/commands/kill_all_sessions_by_pattern_command.cpp` |
| `killOp` |  | `KillOpCmdBase` | `BasicCommand` | `src/mongo/db/commands/kill_op_cmd_base.h` |
| `killSessions` |  | `KillSessionsCommand` | `BasicCommand` | `src/mongo/db/commands/kill_sessions_command.cpp` |
| `listCommands` |  | `ListCommandsCmd` | `BasicCommand` | `src/mongo/db/commands/generic.cpp` |
| `listShards` | `listshards` | `ListShardsCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_list_shards_cmd.cpp` |
| `lockInfo` |  | `CmdLockInfo` | `BasicCommand` | `src/mongo/db/commands/lock_info.cpp` |
| `makeSnapshot` |  | `CmdMakeSnapshot` | `BasicCommand` | `src/mongo/db/commands/snapshot_management.cpp` |
| `mapReduce` | `mapreduce` | `MapReduceCommandBase` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/map_reduce_command_base.h` |
| `mergeChunks` | `_shardsvrMergeChunks` | `MergeChunksCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/shardsvr_merge_chunks_command.cpp` |
| `mergeChunks` |  | `ClusterMergeChunksCommand` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_merge_chunks_cmd.cpp` |
| `moveChunk` | `movechunk` | `MoveChunkCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_move_chunk_cmd.cpp` |
| `movePrimary` | `moveprimary` | `MoveDatabasePrimaryCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_move_primary_cmd.cpp` |
| `multicast` |  | `ClusterMulticastCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_multicast_cmd.cpp` |
| `netstat` |  | `NetStatCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_netstat_cmd.cpp` |
| `pinHistoryReplicated` |  | `DurableHistoryReplicatedTestCmd` | `BasicCommand` | `src/mongo/db/commands/test_commands.cpp` |
| `planCacheClear` |  | `PlanCacheClearCommand` | `BasicCommand` | `src/mongo/db/commands/plan_cache_clear_command.cpp` |
| `planCacheClear` |  | `ClusterPlanCacheClearCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_plan_cache_clear_cmd.cpp` |
| `profile` |  | `ProfileCmdBase` | `BasicCommand` | `src/mongo/db/commands/profile_common.h` |
| `reIndex` |  | `CmdReIndex` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/drop_indexes.cpp` |
| `reapLogicalSessionCacheNow` |  | `ReapLogicalSessionCacheNowCommand` | `BasicCommand` | `src/mongo/db/commands/reap_logical_session_cache_now.cpp` |
| `refreshLogicalSessionCacheNow` |  | `RefreshLogicalSessionCacheNowCommand` | `BasicCommand` | `src/mongo/db/commands/refresh_logical_session_cache_now.cpp` |
| `removeShard` | `removeshard` | `RemoveShardCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_remove_shard_cmd.cpp` |
| `removeShardFromZone` | `removeshardfromzone` | `RemoveShardFromZoneCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_remove_shard_from_zone_cmd.cpp` |
| `renameCollection` |  | `CmdRenameCollection` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/rename_collection_cmd.cpp` |
| `repairShardedCollectionChunksHistory` |  | `RepairShardedCollectionChunksHistoryCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_repair_sharded_collection_chunks_history_cmd.cpp` |
| `replSetGetStatus` |  | `CmdReplSetGetStatus` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_repl_set_get_status_cmd.cpp` |
| `replSetResizeOplog` |  | `CmdReplSetResizeOplog` | `BasicCommand` | `src/mongo/db/commands/resize_oplog.cpp` |
| `serverStatus` |  | `CmdServerStatus` | `BasicCommand` | `src/mongo/db/commands/server_status_command.cpp` |
| `setCommittedSnapshot` |  | `CmdSetCommittedSnapshot` | `BasicCommand` | `src/mongo/db/commands/snapshot_management.cpp` |
| `setDefaultRWConcern` |  | `ClusterSetDefaultRWConcernCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_rwc_defaults_commands.cpp` |
| `setIndexCommitQuorum` |  | `SetIndexCommitQuorumCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_set_index_commit_quorum_cmd.cpp` |
| `setParameter` |  | `CmdSet` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/parameters.cpp` |
| `setProfilingFilterGlobally` |  | `SetProfilingFilterGloballyCmd` | `BasicCommand` | `src/mongo/db/commands/set_profiling_filter_globally_cmd.h` |
| `setShardVersion` |  | `SetShardVersion` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/set_shard_version_command.cpp` |
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
| `updateZoneKeyRange` | `updatezonekeyRange` | `UpdateZoneKeyRangeCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_update_zone_key_range_cmd.cpp` |
| `validate` |  | `ValidateCmd` | `BasicCommand` | `src/mongo/db/commands/validate.cpp` |
| `validate` |  | `ValidateCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_validate_cmd.cpp` |
| `waitForOngoingChunkSplits` |  | `WaitForOngoingChunksSplitsCommand` | `BasicCommand` | `src/mongo/db/s/wait_for_ongoing_chunk_splits_command.cpp` |
| `whatsmysni` |  | `CmdWhatsMySNI` | `BasicCommand` | `src/mongo/db/commands/whats_my_sni_command.cpp` |
| `whatsmyuri` |  | `CmdWhatsMyUri` | `BasicCommand` | `src/mongo/db/commands/whats_my_uri_cmd.cpp` |
| `whatsmyuri` |  | `WhatsMyUriCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_whats_my_uri_cmd.cpp` |
<!-- END v6_manual_commands.md -->

<!-- BEGIN v6_shell_flags.md -->
# v6 Legacy Shell Flags

- Total flags/options: `36`

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
| `password` | `String` | `password` | `p` | `Authentication Options` | `` | `` | `` | `` | password for authentication |
| `port` | `String` | `` | `` | `` | `` | `` | `` | `` | port to connect to |
| `quiet` | `Switch` | `` | `` | `` | `` | `` | `` | `` | be less chatty |
| `retryWrites` | `Switch` | `` | `` | `` | `` | `` | `` | `` | automatically retry write operations upon transient network errors |
| `setShellParameter` | `StringMap` | `` | `` | `` | `` | `yes` | `` | `` | Set a configurable parameter |
| `shell` | `Switch` | `` | `` | `` | `` | `` | `` | `` | run the shell after executing files |
| `username` | `String` | `username` | `u` | `Authentication Options` | `` | `` | `` | `` | username for authentication |
| `verbose` | `Switch` | `` | `` | `` | `` | `` | `` | `` | increase verbosity |
| `version` | `Switch` | `` | `` | `` | `` | `` | `` | `` | show version information |
<!-- END v6_shell_flags.md -->

<!-- BEGIN v5_to_v6_shell_flag_diff.md -->
# Shell Flag Diff v5 -> v6

- Added: `1`
- Removed: `4`
- Changed: `2`

## Added

- `crashOnInvalidBSONError`

## Removed

- `readMode`
- `rpcProtocols`
- `useLegacyWriteOps`
- `writeMode`

## Changed

- `apiDeprecationErrors`: `description`
- `apiStrict`: `description`
<!-- END v5_to_v6_shell_flag_diff.md -->
