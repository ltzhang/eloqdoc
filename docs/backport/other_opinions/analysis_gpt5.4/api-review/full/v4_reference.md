# V4 Full Reference

<!-- BEGIN v4.md -->
# MongoDB v4.4 API Workbook

Representative branch: `origin/v4.4`

This is the baseline implementation model that later versions evolve from.

## Primary source anchors

- `src/mongo/db/service_entry_point_common.cpp`
- `src/mongo/db/commands/*.idl`
- `src/mongo/shell/db.js`
- `src/mongo/shell/shell_options.idl`
- `src/mongo/shell/shell_options.cpp`

## Command-entry pipeline

The main command path is `execCommandDatabase(...)` in `service_entry_point_common.cpp`.

Observed order in v4.4:

1. `CommandHelpers::uassertShouldAttemptParse(opCtx, command, request)`
2. `getClientOperationTime(opCtx)`
3. `command->parse(opCtx, request)`
4. `CommandInvocation::set(opCtx, invocation)`
5. `CurOp::setCommand_inlock(command)`
6. for `hello` / `isMaster`, preload `ClientMetadata`
7. `rpc::readRequestMetadata(opCtx, request.body, command->requiresAuth())`
8. `rpc::TrackingMetadata::initWithOperName(...)`
9. `initializeOperationSessionInfo(...)`
10. generic top-level field scan and duplicate-key detection
11. help fast-path
12. authorization check
13. primary / secondary / maintenance-state checks
14. parse `maxTimeMS` and `maxTimeMSOpOnly`
15. extract and validate read concern
16. initialize sharding routing versions from the request body
17. `behaviors.waitForReadConcern(...)`
18. `runCommandImpl(...)`

Important baseline properties:

- No Stable API envelope exists yet.
- No `APIParameters` initialization exists yet.
- Parsing happens before request metadata and session info initialization.
- The generic top-level fields recognized by the service-entry path include:
  - `maxTimeMS`
  - `maxTimeMSOpOnly`
  - `allowImplicitCollectionCreation`
  - `help`
  - `comment`
- Duplicate top-level keys are rejected before execution.

## Server command surface

Exhaustive catalogs:

- `generated/v4_idl_commands.md`
- `generated/v4_manual_commands.md`

Counts:

- IDL commands: `50`
- Manual / non-IDL command definitions: `215`

What this means for implementation:

- v4 still has a relatively small IDL-described surface.
- A large amount of command registration still comes from manual `BasicCommand` / related subclasses.
- When later versions “add” commands, many of those additions are real APIs, but some are commands that become newly expressed in IDL rather than newly invented semantics.

## Legacy shell surface

Exhaustive shell option inventory:

- `generated/v4_shell_flags.md`

Count:

- shell flags/options: `36`

Important shell behavior in v4:

- No `--apiVersion`
- No `--apiStrict`
- No `--apiDeprecationErrors`
- `DB.prototype.hello()` exists and directly sends `hello`
- `DB.prototype.listCommands()` prints `slaveOk`

The v4 shell therefore has a `hello` helper, but no Stable API machinery around it.

## Porting implications

If you are implementing a later API on top of v4 code, these are the things v4 does not give you:

- no Stable API request fields
- no `RunCommandImpl`-style `HandleRequest::_parseCommand()` phase
- no `secondaryOk` / `secondaryOverrideOk` terminology in shell output
- no shell CLI support for API-versioned operation
- no standalone `mongosh` client in this repo

Use v4 as the baseline for:

- pre-versioned command parsing
- pre-`CommonRequestArgs` request handling
- pre-`mongosh` helper modeling
<!-- END v4.md -->

<!-- BEGIN v4_idl_commands.md -->
# v4 IDL Command Inventory

## Summary
- Command count: 50

## `MapReduce`
- source=`src/mongo/db/commands/map_reduce.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`None`
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

## `_addShard`
- source=`src/mongo/db/s/add_shard_cmd.idl`, namespace=`ignored`, strict=`None`, api_version=`None`
- _addShard Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `shardIdentity` | `ShardIdentity` | `False` | `` | `False` | `` | `` |

## `_cloneCollectionOptionsFromPrimaryShard`
- source=`src/mongo/s/request_types/clone_collection_options_from_primary_shard.idl`, namespace=`type`, strict=`True`, api_version=`None`
- Internal command to create a collection on a non-primary shard with the collection options from the primary shard.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `primaryShard` | `string` | `False` | `` | `False` | `` | `` |

## `_configsvrBalancerCollectionStatus`
- source=`src/mongo/s/request_types/balancer_collection_status.idl`, namespace=`type`, strict=`True`, api_version=`None`
- Internal balancerCollectionStatus command on the config server
_No IDL fields declared._

## `_configsvrClearJumboFlag`
- source=`src/mongo/s/request_types/clear_jumbo_flag.idl`, namespace=`type`, strict=`False`, api_version=`None`
- internal clearJumboFlag command for config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `epoch` | `objectid` | `False` | `` | `False` | `` | `` |
| `minKey` | `object` | `False` | `` | `False` | `` | `` |
| `maxKey` | `object` | `False` | `` | `False` | `` | `` |

## `_configsvrCreateCollection`
- source=`src/mongo/s/request_types/create_collection.idl`, namespace=`type`, strict=`False`, api_version=`None`
- The internal createCollection command on the config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `options` | `object` | `True` | `` | `False` | `` | `` |

## `_configsvrCreateDatabase`
- source=`src/mongo/s/request_types/create_database.idl`, namespace=`type`, strict=`False`, api_version=`None`
- The internal createDatabase command on the config server
_No IDL fields declared._

## `_configsvrEnsureChunkVersionIsGreaterThan`
- source=`src/mongo/s/request_types/ensure_chunk_version_is_greater_than.idl`, namespace=`ignored`, strict=`False`, api_version=`None`
- If a chunk matching 'requestedChunk' exists, bumps the chunk's version to one greater than the current collection version.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `minKey` | `object` | `False` | `` | `False` | `` | `` |
| `maxKey` | `object` | `False` | `` | `False` | `` | `` |
| `version` | `ChunkVersion` | `False` | `` | `False` | `` | `` |

## `_configsvrRefineCollectionShardKey`
- source=`src/mongo/s/request_types/refine_collection_shard_key.idl`, namespace=`type`, strict=`False`, api_version=`None`
- The internal refineCollectionShardKey command on the config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `object` | `False` | `` | `False` | `` | `` |
| `epoch` | `objectid` | `False` | `` | `False` | `` | `` |
| `enforceUniquenessCheck` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `_flushDatabaseCacheUpdates`
- source=`src/mongo/s/request_types/flush_database_cache_updates.idl`, namespace=`type`, strict=`True`, api_version=`None`
- An internal command to wait for the last routing table cache refresh for a particular database to be persisted to disk
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `syncFromConfig` | `bool` | `False` | `True` | `False` | `` | `` |

## `_flushRoutingTableCacheUpdates`
- source=`src/mongo/s/request_types/flush_routing_table_cache_updates.idl`, namespace=`type`, strict=`True`, api_version=`None`
- An internal command to wait for the last routing table cache refresh for a particular namespace to be persisted to disk
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `syncFromConfig` | `bool` | `False` | `True` | `False` | `` | `` |

## `_killOperations`
- source=`src/mongo/db/commands/kill_operations.idl`, namespace=`ignored`, strict=`True`, api_version=`None`
- Interrupt a list of operations on a remote server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `operationKeys` | `array<uuid>` | `False` | `` | `False` | `` | `` |

## `_shardsvrSetAllowMigrations`
- source=`src/mongo/s/request_types/set_allow_migrations.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=`None`
- Internal setAllowMigrations command for a shard.
_No IDL fields declared._

## `abortTransaction`
- source=`src/mongo/db/commands/txn_cmds.idl`, namespace=`ignored`, strict=`None`, api_version=`None`
- abortTransaction Command
_No IDL fields declared._

## `autoSplitVector`
- source=`src/mongo/s/request_types/auto_split_vector.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=`None`
- Internal autoSplitVector command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `keyPattern` | `object_owned` | `False` | `` | `False` | `` | `` |
| `min` | `object_owned` | `False` | `` | `False` | `` | `` |
| `max` | `object_owned` | `False` | `` | `False` | `` | `` |
| `maxChunkSizeBytes` | `safeInt64` | `False` | `` | `False` | `` | `` |

## `balancerCollectionStatus`
- source=`src/mongo/s/request_types/balancer_collection_status.idl`, namespace=`type`, strict=`True`, api_version=`None`
- Public balancerCollectionStatus command on mongos
_No IDL fields declared._

## `clearJumboFlag`
- source=`src/mongo/s/request_types/clear_jumbo_flag.idl`, namespace=`type`, strict=`False`, api_version=`None`
- clearJumboFlag command for mongos
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `bounds` | `array<object>` | `True` | `` | `False` | `` | `` |
| `find` | `object` | `True` | `` | `False` | `` | `` |

## `cloneCatalogData`
- source=`src/mongo/s/request_types/clone_catalog_data.idl`, namespace=`type`, strict=`False`, api_version=`None`
- The internal cloneCatalogData command on a shard
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `from` | `string` | `False` | `` | `False` | `` | `` |

## `commitTransaction`
- source=`src/mongo/db/commands/txn_cmds.idl`, namespace=`ignored`, strict=`None`, api_version=`None`
- commitTransaction Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `commitTimestamp` | `timestamp` | `True` | `` | `False` | `` | `` |
| `recoveryToken` | `TxnRecoveryToken` | `True` | `` | `False` | `` | `` |

## `coordinateCommitTransaction`
- source=`src/mongo/db/commands/txn_two_phase_commit_cmds.idl`, namespace=`ignored`, strict=`False`, api_version=`None`
- Parser for the 'coordinateCommitTransaction' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `participants` | `array<CommitParticipant>` | `False` | `` | `False` | `` | `` |

## `count`
- source=`src/mongo/db/query/count_command.idl`, namespace=`concatenate_with_db_or_uuid`, strict=`True`, api_version=`None`
- Parser for the 'count' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `query` | `object` | `False` | `BSONObj()` | `False` | `` | `` |
| `limit` | `countLimit` | `True` | `` | `False` | `` | `` |
| `skip` | `countSkip` | `True` | `` | `False` | `` | `` |
| `hint` | `indexHint` | `False` | `BSONObj()` | `False` | `` | `` |
| `collation` | `object` | `True` | `` | `False` | `` | `` |
| `fields` | `object` | `False` | `` | `False` | `` | `` |
| `readConcern` | `object` | `True` | `` | `False` | `` | `` |
| `maxTimeMS` | `maxTimeMS` | `True` | `` | `False` | `` | `` |
| `$queryOptions` | `object` | `True` | `` | `False` | `queryOptions` | `` |

## `create`
- source=`src/mongo/db/commands/create.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`None`
- Parser for the 'create' Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `capped` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `autoIndexId` | `safeBool` | `True` | `` | `False` | `` | `` |
| `idIndex` | `object` | `True` | `` | `False` | `` | `` |
| `size` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `max` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `storageEngine` | `object` | `True` | `` | `False` | `` | `` |
| `validator` | `object` | `True` | `` | `False` | `` | `` |
| `validationLevel` | `string` | `False` | `"strict"` | `False` | `` | `` |
| `validationAction` | `string` | `False` | `"error"` | `False` | `` | `` |
| `indexOptionDefaults` | `object` | `True` | `` | `False` | `` | `` |
| `viewOn` | `string` | `True` | `` | `False` | `` | `` |
| `pipeline` | `array<object>` | `True` | `` | `False` | `` | `` |
| `collation` | `object` | `True` | `` | `False` | `` | `` |
| `writeConcern` | `object` | `True` | `` | `False` | `` | `` |
| `recordPreImages` | `safeBool` | `True` | `` | `False` | `` | `` |
| `temp` | `safeBool` | `True` | `` | `False` | `` | `` |
| `flags` | `safeInt64` | `True` | `` | `False` | `` | `` |

## `delete`
- source=`src/mongo/db/ops/write_ops.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`None`
- Parser for the 'delete' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `deletes` | `array<DeleteOpEntry>` | `False` | `` | `False` | `` | `` |

## `distinct`
- source=`src/mongo/db/query/distinct_command.idl`, namespace=`concatenate_with_db_or_uuid`, strict=`True`, api_version=`None`
- Parser for the 'distinct' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `string` | `False` | `` | `False` | `` | `` |
| `query` | `object` | `True` | `` | `False` | `` | `` |
| `collation` | `object` | `True` | `` | `False` | `` | `` |

## `dropConnections`
- source=`src/mongo/db/commands/drop_connections.idl`, namespace=`ignored`, strict=`None`, api_version=`None`
- An administrative command which takes a list of host and ports and drops pooled connections to them
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `hostAndPort` | `array<HostAndPort>` | `False` | `` | `False` | `` | `` |

## `exampleIncrement`
- source=`src/mongo/db/commands_test_example.idl`, namespace=`concatenate_with_db`, strict=`None`, api_version=`None`
- increment an integer (TypedCommand example)
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `i` | `None` | `False` | `` | `False` | `` | `` |

## `exampleMinimal`
- source=`src/mongo/db/commands_test_example.idl`, namespace=`concatenate_with_db`, strict=`None`, api_version=`None`
- like exampleIncrement, but use MinimalInvocationBase
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `i` | `None` | `False` | `` | `False` | `` | `` |

## `exampleVoid`
- source=`src/mongo/db/commands_test_example.idl`, namespace=`concatenate_with_db`, strict=`None`, api_version=`None`
- no return, just side effects
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `i` | `None` | `False` | `` | `False` | `` | `` |

## `fsyncUnlock`
- source=`src/mongo/s/commands/cluster_fsync_unlock_cmd.idl`, namespace=`ignored`, strict=`False`, api_version=`None`
- The command for calling fsync unlock on all shards of a cluster.
_No IDL fields declared._

## `getDatabaseVersion`
- source=`src/mongo/s/request_types/get_database_version.idl`, namespace=`type`, strict=`None`, api_version=`None`
- An internal command to get a shard server's cached database version
_No IDL fields declared._

## `getDefaultRWConcern`
- source=`src/mongo/db/commands/rwc_defaults_commands.idl`, namespace=`ignored`, strict=`None`, api_version=`None`
- Get the current read/write concern defaults being applied by this node
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `inMemory` | `bool` | `True` | `` | `False` | `` | `` |

## `httpClientRequest`
- source=`src/mongo/db/commands/http_client.idl`, namespace=`ignored`, strict=`None`, api_version=`None`
- Fetch an HTTP(S) resource via GET
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `uri` | `string` | `False` | `` | `False` | `` | `` |
| `timeoutSecs` | `int` | `True` | `` | `False` | `` | `{"gt": 0}` |

## `insert`
- source=`src/mongo/db/ops/write_ops.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`None`
- Parser for the 'insert' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `documents` | `array<object>` | `False` | `` | `False` | `` | `` |

## `internalRenameIfOptionsAndIndexesMatch`
- source=`src/mongo/db/commands/internal_rename_if_options_and_indexes_match.idl`, namespace=`ignored`, strict=`None`, api_version=`None`
- An internal command that does a rename, but first checks to make sure the indexes and collection options on the destination match those given in the command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `from` | `namespacestring` | `False` | `` | `False` | `` | `` |
| `to` | `namespacestring` | `False` | `` | `False` | `` | `` |
| `collectionOptions` | `object` | `False` | `` | `False` | `` | `` |
| `indexes` | `array<object>` | `False` | `` | `False` | `` | `` |

## `listDatabasesCommand`
- source=`src/mongo/db/commands/list_databases.idl`, namespace=`ignored`, strict=`None`, api_version=`None`
- listDatabases Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `nameOnly` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `authorizedDatabases` | `bool` | `True` | `` | `False` | `` | `` |
| `filter` | `object` | `True` | `` | `False` | `` | `` |

## `logMessage`
- source=`src/mongo/db/commands/generic.idl`, namespace=`type`, strict=`True`, api_version=`None`
- Log a message on the server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `extra` | `object` | `True` | `` | `False` | `` | `` |
| `severity` | `MessageSeverity` | `False` | `kLog` | `False` | `` | `` |
| `debugLevel` | `int` | `True` | `` | `False` | `` | `{"gte": 1, "lte": 5}` |

## `prepareTransaction`
- source=`src/mongo/db/commands/txn_two_phase_commit_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=`None`
- Parser for the 'prepareTransaction' command.
_No IDL fields declared._

## `profile`
- source=`src/mongo/db/commands/profile.idl`, namespace=`type`, strict=`False`, api_version=`None`
- Parser for the 'profile' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `slowms` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `sampleRate` | `double` | `True` | `` | `False` | `` | `` |
| `filter` | `ObjectOrUnset` | `True` | `` | `False` | `` | `` |

## `refineCollectionShardKey`
- source=`src/mongo/s/request_types/refine_collection_shard_key.idl`, namespace=`type`, strict=`False`, api_version=`None`
- The public refineCollectionShardKey command on mongos
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `object` | `False` | `` | `False` | `` | `` |
| `enforceUniquenessCheck` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `renameCollection`
- source=`src/mongo/db/commands/rename_collection.idl`, namespace=`type`, strict=`True`, api_version=`None`
- Parser for the 'renameCollection' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `to` | `namespacestring` | `False` | `` | `False` | `` | `` |
| `dropTarget` | `bool` | `False` | `False` | `False` | `` | `` |
| `stayTemp` | `bool` | `False` | `False` | `False` | `` | `` |

## `replSetResizeOplog`
- source=`src/mongo/db/commands/resize_oplog.idl`, namespace=`type`, strict=`False`, api_version=`None`
- Parser for the 'replSetResizeOplog' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `size` | `double` | `True` | `` | `False` | `` | `{"gte": 990, "lte": 1073741824}` |
| `minRetentionHours` | `double` | `True` | `` | `False` | `` | `{"gte": 0}` |

## `setAllowMigrations`
- source=`src/mongo/s/request_types/set_allow_migrations.idl`, namespace=`type`, strict=`True`, api_version=`None`
- User faced setAllowMigrations command.
_No IDL fields declared._

## `setDefaultRWConcern`
- source=`src/mongo/db/commands/rwc_defaults_commands.idl`, namespace=`ignored`, strict=`None`, api_version=`None`
- Set the current read/write concern defaults (cluster-wide)
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `defaultReadConcern` | `ReadConcern` | `True` | `` | `False` | `` | `` |
| `defaultWriteConcern` | `WriteConcern` | `True` | `` | `False` | `` | `` |

## `setIndexCommitQuorum`
- source=`src/mongo/db/commands/set_index_commit_quorum.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=`None`
- Resets the commitQuorum for an index build.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `indexNames` | `array<string>` | `False` | `` | `False` | `` | `` |
| `commitQuorum` | `CommitQuorum` | `False` | `` | `False` | `` | `` |

## `setProfilingFilterGlobally`
- source=`src/mongo/db/commands/profile.idl`, namespace=`ignored`, strict=`True`, api_version=`None`
- Parser for the 'setProfilingFilterGlobally' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `filter` | `ObjectOrUnset` | `False` | `` | `False` | `` | `` |

## `startRecordingTraffic`
- source=`src/mongo/db/traffic_recorder.idl`, namespace=`ignored`, strict=`None`, api_version=`None`
- start recording Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `filename` | `string` | `False` | `` | `False` | `` | `` |
| `bufferSize` | `long` | `False` | `134217728` | `False` | `` | `` |
| `maxFileSize` | `long` | `False` | `6294967296` | `False` | `` | `` |

## `stopRecordingTraffic`
- source=`src/mongo/db/traffic_recorder.idl`, namespace=`ignored`, strict=`None`, api_version=`None`
- stop recording Command
_No IDL fields declared._

## `update`
- source=`src/mongo/db/ops/write_ops.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`None`
- Parser for the 'update' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `updates` | `array<UpdateOpEntry>` | `False` | `` | `False` | `` | `` |
| `runtimeConstants` | `RuntimeConstants` | `True` | `` | `False` | `` | `` |

## `voteCommitIndexBuild`
- source=`src/mongo/db/commands/vote_commit_index_build.idl`, namespace=`type`, strict=`False`, api_version=`None`
- An internal mongod command pertaining to cross replica set index builds
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `hostAndPort` | `HostAndPort` | `False` | `` | `False` | `` | `` |

## `waitForFailPoint`
- source=`src/mongo/s/request_types/wait_for_fail_point.idl`, namespace=`type`, strict=`False`, api_version=`None`
- wait for a fail point to be entered a certain number of times
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `timesEntered` | `safeInt64` | `False` | `` | `False` | `` | `` |
| `maxTimeMS` | `safeInt64` | `False` | `` | `False` | `` | `` |
<!-- END v4_idl_commands.md -->

<!-- BEGIN v4_manual_commands.md -->
# v4 Manual / Non-IDL Command Inventory

## Summary
- Constructor-extracted command definitions: 215

| Primary | Aliases | Class | Base | File |
|---|---|---|---|---|
| `_configsvrAddShard` |  | `ConfigSvrAddShardCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_add_shard_command.cpp` |
| `_configsvrAddShardToZone` |  | `ConfigSvrAddShardToZoneCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_add_shard_to_zone_command.cpp` |
| `_configsvrCommitChunkMerge` |  | `ConfigSvrMergeChunkCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_merge_chunk_command.cpp` |
| `_configsvrCommitChunkMigration` |  | `ConfigSvrCommitChunkMigrationCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_commit_chunk_migration_command.cpp` |
| `_configsvrCommitChunkSplit` |  | `ConfigSvrSplitChunkCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_split_chunk_command.cpp` |
| `_configsvrCommitChunksMerge` |  | `ConfigSvrMergeChunksCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_merge_chunks_command.cpp` |
| `_configsvrCommitMovePrimary` |  | `ConfigSvrCommitMovePrimaryCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_commit_move_primary_command.cpp` |
| `_configsvrDropCollection` |  | `ConfigSvrDropCollectionCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_drop_collection_command.cpp` |
| `_configsvrDropDatabase` |  | `ConfigSvrDropDatabaseCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_drop_database_command.cpp` |
| `_configsvrEnableSharding` |  | `ConfigSvrEnableShardingCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_enable_sharding_command.cpp` |
| `_configsvrMoveChunk` |  | `ConfigSvrMoveChunkCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_move_chunk_command.cpp` |
| `_configsvrMovePrimary` |  | `ConfigSvrMovePrimaryCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_move_primary_command.cpp` |
| `_configsvrRemoveShard` |  | `ConfigSvrRemoveShardCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_remove_shard_command.cpp` |
| `_configsvrRemoveShardFromZone` |  | `ConfigSvrRemoveShardFromZoneCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_remove_shard_from_zone_command.cpp` |
| `_configsvrRepairShardedCollectionChunksHistory` |  | `ConfigSvrRepairShardedCollectionChunksHistoryCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_repair_sharded_collection_chunks_history_command.cpp` |
| `_configsvrShardCollection` |  | `ConfigSvrShardCollectionCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_shard_collection_command.cpp` |
| `_configsvrUpdateZoneKeyRange` |  | `ConfigsvrUpdateZoneKeyRangeCommand` | `BasicCommand` | `src/mongo/db/s/config/configsvr_update_zone_key_range_command.cpp` |
| `_getNextSessionMods` |  | `MigrateSessionCommand` | `BasicCommand` | `src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp` |
| `_getUserCacheGeneration` |  | `CmdGetCacheGeneration` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `_hashBSONElement` |  | `CmdHashElt` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/hashcmd.cpp` |
| `_isSelf` |  | `IsSelfCommand` | `BasicCommand` | `src/mongo/db/commands/isself.cpp` |
| `_mergeAuthzCollections` |  | `CmdMergeAuthzCollections` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `_mergeAuthzCollections` |  | `CmdMergeAuthzCollections` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `_migrateClone` |  | `InitialCloneCommand` | `BasicCommand` | `src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp` |
| `_recvChunkAbort` |  | `RecvChunkAbortCommand` | `BasicCommand` | `src/mongo/db/s/migration_destination_manager_legacy_commands.cpp` |
| `_recvChunkCommit` |  | `RecvChunkCommitCommand` | `BasicCommand` | `src/mongo/db/s/migration_destination_manager_legacy_commands.cpp` |
| `_recvChunkStart` |  | `RecvChunkStartCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/migration_destination_manager_legacy_commands.cpp` |
| `_recvChunkStatus` |  | `RecvChunkStatusCommand` | `BasicCommand` | `src/mongo/db/s/migration_destination_manager_legacy_commands.cpp` |
| `_shardsvrCloneCatalogData` | `_cloneCatalogData` | `CloneCatalogDataCommand` | `BasicCommand` | `src/mongo/db/s/clone_catalog_data_command.cpp` |
| `_shardsvrMovePrimary` | `_movePrimary` | `MovePrimaryCommand` | `BasicCommand` | `src/mongo/db/s/move_primary_command.cpp` |
| `_shardsvrShardCollection` |  | `ShardsvrShardCollectionCommand` | `BasicCommand` | `src/mongo/db/s/shardsvr_shard_collection.cpp` |
| `_transferMods` |  | `TransferModsCommand` | `BasicCommand` | `src/mongo/db/s/migration_chunk_cloner_source_legacy_commands.cpp` |
| `abortTransaction` |  | `CmdAbortTxn` | `BasicCommand` | `src/mongo/db/commands/txn_cmds.cpp` |
| `abortTransaction` |  | `ClusterAbortTransactionCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_abort_transaction_cmd.cpp` |
| `addShard` | `addshard` | `AddShardCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_add_shard_cmd.cpp` |
| `addShardToZone` | `addshardtozone` | `AddShardToZoneCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_add_shard_to_zone_cmd.cpp` |
| `appendOplogNote` |  | `AppendOplogNoteCmd` | `BasicCommand` | `src/mongo/db/commands/oplog_note.cpp` |
| `appendOplogNote` |  | `AppendOplogNoteCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_oplog_note_cmd.cpp` |
| `applyOps` |  | `ApplyOpsCmd` | `BasicCommand` | `src/mongo/db/commands/apply_ops_cmd.cpp` |
| `authenticate` |  | `CmdAuthenticate` | `BasicCommand` | `src/mongo/db/commands/authentication_commands.cpp` |
| `availableQueryOptions` | `availablequeryoptions` | `AvailableQueryOptions` | `BasicCommand` | `src/mongo/db/commands/dbcommands_d.cpp` |
| `availableQueryOptions` | `availablequeryoptions` | `AvailableQueryOptions` | `BasicCommand` | `src/mongo/s/commands/cluster_available_query_options_cmd.cpp` |
| `buildInfo` | `buildinfo` | `CmdBuildInfo` | `BasicCommand` | `src/mongo/db/commands/dbcommands.cpp` |
| `buildInfo` | `buildinfo` | `ClusterCmdBuildInfo` | `BasicCommand` | `src/mongo/s/commands/cluster_build_info.cpp` |
| `captrunc` |  | `CapTrunc` | `BasicCommand` | `src/mongo/db/commands/test_commands.cpp` |
| `checkShardingIndex` |  | `CheckShardingIndex` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/check_sharding_index_command.cpp` |
| `cleanupOrphaned` |  | `CleanupOrphanedCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/cleanup_orphaned_cmd.cpp` |
| `clearLog` |  | `ClearLogCmd` | `BasicCommand` | `src/mongo/db/commands/generic_servers.cpp` |
| `cloneCollectionAsCapped` |  | `CmdCloneCollectionAsCapped` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/collection_to_capped.cpp` |
| `collMod` |  | `CollectionModCommand` | `BasicCommand` | `src/mongo/db/commands/dbcommands.cpp` |
| `collMod` |  | `CollectionModCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_collection_mod_cmd.cpp` |
| `collStats` | `collstats` | `CollectionStats` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/dbcommands.cpp` |
| `collStats` | `collstats` | `CollectionStats` | `BasicCommand` | `src/mongo/s/commands/cluster_coll_stats_cmd.cpp` |
| `commitTransaction` |  | `CmdCommitTxn` | `BasicCommand` | `src/mongo/db/commands/txn_cmds.cpp` |
| `commitTransaction` |  | `ClusterCommitTransactionCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_commit_transaction_cmd.cpp` |
| `compact` |  | `CompactCmd` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/compact.cpp` |
| `compact` |  | `CompactCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_compact_cmd.cpp` |
| `configureFailPoint` |  | `FaultInjectCmd` | `BasicCommand` | `src/mongo/db/commands/fail_point_cmd.cpp` |
| `connPoolStats` |  | `PoolStats` | `BasicCommand` | `src/mongo/db/commands/conn_pool_stats.cpp` |
| `connPoolSync` | `connpoolsync` | `PoolFlushCmd` | `BasicCommand` | `src/mongo/db/commands/conn_pool_sync.cpp` |
| `connectionStatus` |  | `CmdConnectionStatus` | `BasicCommand` | `src/mongo/db/commands/connection_status.cpp` |
| `convertToCapped` |  | `CmdConvertToCapped` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/collection_to_capped.cpp` |
| `convertToCapped` |  | `ConvertToCappedCmd` | `BasicCommand` | `src/mongo/s/commands/commands_public.cpp` |
| `count` |  | `CmdCount` | `BasicCommand` | `src/mongo/db/commands/count_cmd.cpp` |
| `count` |  | `ClusterCountCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_count_cmd.cpp` |
| `cpuload` |  | `CPULoadCommand` | `BasicCommand` | `src/mongo/db/commands/cpuload.cpp` |
| `create` |  | `CmdCreate` | `BasicCommand` | `src/mongo/db/commands/dbcommands.cpp` |
| `create` |  | `CreateCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_create_cmd.cpp` |
| `createIndexes` |  | `CreateIndexesCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_create_indexes_cmd.cpp` |
| `createRole` |  | `CmdCreateRole` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `createRole` |  | `CmdCreateRole` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `createUser` |  | `CmdCreateUser` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `createUser` |  | `CmdCreateUser` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `currentOp` |  | `CurrentOpCommandBase` | `BasicCommand` | `src/mongo/db/commands/current_op_common.h` |
| `dataSize` | `datasize` | `CmdDatasize` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/dbcommands.cpp` |
| `dataSize` | `datasize` | `DataSizeCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_data_size_cmd.cpp` |
| `dbCheck` |  | `DbCheckCmd` | `BasicCommand` | `src/mongo/db/commands/dbcheck.cpp` |
| `dbHash` | `dbhash` | `DBHashCmd` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/dbhash.cpp` |
| `dbStats` | `dbstats` | `DBStats` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/dbcommands.cpp` |
| `dbStats` | `dbstats` | `DBStatsCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_db_stats_cmd.cpp` |
| `distinct` |  | `DistinctCommand` | `BasicCommand` | `src/mongo/db/commands/distinct.cpp` |
| `distinct` |  | `DistinctCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_distinct_cmd.cpp` |
| `drop` |  | `CmdDrop` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/dbcommands.cpp` |
| `drop` |  | `DropCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_drop_cmd.cpp` |
| `dropAllRolesFromDatabase` |  | `CmdDropAllRolesFromDatabase` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `dropAllRolesFromDatabase` |  | `CmdDropAllRolesFromDatabase` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `dropAllUsersFromDatabase` |  | `CmdDropAllUsersFromDatabase` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `dropAllUsersFromDatabase` |  | `CmdDropAllUsersFromDatabase` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `dropDatabase` |  | `CmdDropDatabase` | `BasicCommand` | `src/mongo/db/commands/dbcommands.cpp` |
| `dropDatabase` |  | `DropDatabaseCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_drop_database_cmd.cpp` |
| `dropIndexes` | `deleteIndexes` | `CmdDropIndexes` | `BasicCommand` | `src/mongo/db/commands/drop_indexes.cpp` |
| `dropIndexes` | `deleteIndexes` | `DropIndexesCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_drop_indexes_cmd.cpp` |
| `dropRole` |  | `CmdDropRole` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `dropRole` |  | `CmdDropRole` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `dropUser` |  | `CmdDropUser` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `dropUser` |  | `CmdDropUser` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `emptycapped` |  | `EmptyCapped` | `BasicCommand` | `src/mongo/db/commands/test_commands.cpp` |
| `enableSharding` | `enablesharding` | `EnableShardingCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_enable_sharding_cmd.cpp` |
| `endSessions` |  | `EndSessionsCommand` | `BasicCommand` | `src/mongo/db/commands/end_sessions_command.cpp` |
| `features` |  | `FeaturesCmd` | `BasicCommand` | `src/mongo/db/commands/generic_servers.cpp` |
| `filemd5` |  | `CmdFileMD5` | `BasicCommand` | `src/mongo/db/commands/dbcommands_d.cpp` |
| `filemd5` |  | `FileMD5Cmd` | `BasicCommand` | `src/mongo/s/commands/cluster_filemd5_cmd.cpp` |
| `findAndModify` | `findandmodify` | `CmdFindAndModify` | `BasicCommand` | `src/mongo/db/commands/find_and_modify.cpp` |
| `findAndModify` | `findandmodify` | `FindAndModifyCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_find_and_modify_cmd.cpp` |
| `flushRouterConfig` | `flushrouterconfig` | `FlushRouterConfigCmd` | `BasicCommand` | `src/mongo/s/commands/flush_router_config_cmd.cpp` |
| `fsync` |  | `FSyncCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/fsync.cpp` |
| `fsync` |  | `FsyncCommand` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_fsync_cmd.cpp` |
| `fsyncUnlock` |  | `FSyncUnlockCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/fsync.cpp` |
| `geoSearch` |  | `GeoHaystackSearchCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/haystack.cpp` |
| `getCmdLineOpts` |  | `CmdGetCmdLineOpts` | `BasicCommand` | `src/mongo/db/commands/generic_servers.cpp` |
| `getDiagnosticData` |  | `GetDiagnosticDataCommand` | `BasicCommand` | `src/mongo/db/ftdc/ftdc_commands.cpp` |
| `getDiagnosticData` |  | `GetDiagnosticDataCommand` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_ftdc_commands.cpp` |
| `getLastError` | `getlasterror` | `CmdGetLastError` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/get_last_error.cpp` |
| `getLastError` | `getlasterror` | `GetLastErrorCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_get_last_error_cmd.cpp` |
| `getLog` |  | `GetLogCmd` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/generic_servers.cpp` |
| `getParameter` |  | `CmdGet` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/parameters.cpp` |
| `getShardMap` |  | `CmdGetShardMap` | `BasicCommand` | `src/mongo/s/commands/get_shard_map_cmd.cpp` |
| `getShardVersion` |  | `GetShardVersion` | `BasicCommand` | `src/mongo/db/s/get_shard_version_command.cpp` |
| `getShardVersion` | `getshardversion` | `GetShardVersion` | `BasicCommand` | `src/mongo/s/commands/cluster_get_shard_version_cmd.cpp` |
| `getnonce` |  | `CmdGetNonce` | `BasicCommand` | `src/mongo/db/commands/authentication_commands.cpp` |
| `godinsert` |  | `GodInsert` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/test_commands.cpp` |
| `grantPrivilegesToRole` |  | `CmdGrantPrivilegesToRole` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `grantPrivilegesToRole` |  | `CmdGrantPrivilegesToRole` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `grantRolesToRole` |  | `CmdGrantRolesToRole` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `grantRolesToRole` |  | `CmdGrantRolesToRole` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `grantRolesToUser` |  | `CmdGrantRolesToUser` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `grantRolesToUser` |  | `CmdGrantRolesToUser` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `hostInfo` |  | `HostInfoCmd` | `BasicCommand` | `src/mongo/db/commands/generic_servers.cpp` |
| `invalidateUserCache` |  | `CmdInvalidateUserCache` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `invalidateUserCache` |  | `CmdInvalidateUserCache` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `isdbgrid` |  | `IsDbGridCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_is_db_grid_cmd.cpp` |
| `killAllSessions` |  | `KillAllSessionsCommand` | `BasicCommand` | `src/mongo/db/commands/kill_all_sessions_command.cpp` |
| `killAllSessionsByPattern` |  | `KillAllSessionsByPatternCommand` | `BasicCommand` | `src/mongo/db/commands/kill_all_sessions_by_pattern_command.cpp` |
| `killCursors` |  | `KillCursorsCmdBase` | `BasicCommand` | `src/mongo/db/commands/killcursors_common.h` |
| `killOp` |  | `KillOpCmdBase` | `BasicCommand` | `src/mongo/db/commands/kill_op_cmd_base.h` |
| `killSessions` |  | `KillSessionsCommand` | `BasicCommand` | `src/mongo/db/commands/kill_sessions_command.cpp` |
| `listCollections` |  | `CmdListCollections` | `BasicCommand` | `src/mongo/db/commands/list_collections.cpp` |
| `listCollections` |  | `CmdListCollections` | `BasicCommand` | `src/mongo/s/commands/commands_public.cpp` |
| `listCommands` |  | `ListCommandsCmd` | `BasicCommand` | `src/mongo/db/commands/generic.cpp` |
| `listDatabases` |  | `CmdListDatabases` | `BasicCommand` | `src/mongo/db/commands/list_databases.cpp` |
| `listDatabases` | `listdatabases` | `ListDatabasesCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_list_databases_cmd.cpp` |
| `listIndexes` |  | `CmdListIndexes` | `BasicCommand` | `src/mongo/db/commands/list_indexes.cpp` |
| `listIndexes` |  | `CmdListIndexes` | `BasicCommand` | `src/mongo/s/commands/commands_public.cpp` |
| `listShards` | `listshards` | `ListShardsCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_list_shards_cmd.cpp` |
| `lockInfo` |  | `CmdLockInfo` | `BasicCommand` | `src/mongo/db/commands/lock_info.cpp` |
| `logRotate` |  | `LogRotateCmd` | `BasicCommand` | `src/mongo/db/commands/generic_servers.cpp` |
| `logout` |  | `CmdLogout` | `BasicCommand` | `src/mongo/db/commands/authentication_commands.cpp` |
| `makeSnapshot` |  | `CmdMakeSnapshot` | `BasicCommand` | `src/mongo/db/commands/snapshot_management.cpp` |
| `mapReduce` | `mapreduce` | `MapReduceCommandBase` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/map_reduce_command_base.h` |
| `mapreduce.shardedfinish` |  | `MapReduceFinishCommand` | `BasicCommand` | `src/mongo/db/commands/map_reduce_finish_command.cpp` |
| `mergeChunks` |  | `MergeChunksCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/merge_chunks_command.cpp` |
| `mergeChunks` |  | `ClusterMergeChunksCommand` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_merge_chunks_cmd.cpp` |
| `moveChunk` |  | `MoveChunkCommand` | `BasicCommand` | `src/mongo/db/s/move_chunk_command.cpp` |
| `moveChunk` | `movechunk` | `MoveChunkCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_move_chunk_cmd.cpp` |
| `movePrimary` | `moveprimary` | `MoveDatabasePrimaryCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_move_primary_cmd.cpp` |
| `multicast` |  | `MulticastCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_multicast.cpp` |
| `netstat` |  | `NetStatCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_netstat_cmd.cpp` |
| `ping` |  | `PingCommand` | `BasicCommand` | `src/mongo/db/commands/generic.cpp` |
| `planCacheClear` |  | `PlanCacheClearCommand` | `BasicCommand` | `src/mongo/db/commands/plan_cache_clear_command.cpp` |
| `planCacheClear` |  | `ClusterPlanCacheClearCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_plan_cache_clear_cmd.cpp` |
| `profile` |  | `ProfileCmdBase` | `BasicCommand` | `src/mongo/db/commands/profile_common.h` |
| `reIndex` |  | `CmdReIndex` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/drop_indexes.cpp` |
| `reapLogicalSessionCacheNow` |  | `ReapLogicalSessionCacheNowCommand` | `BasicCommand` | `src/mongo/db/commands/reap_logical_session_cache_now.cpp` |
| `refreshLogicalSessionCacheNow` |  | `RefreshLogicalSessionCacheNowCommand` | `BasicCommand` | `src/mongo/db/commands/refresh_logical_session_cache_now.cpp` |
| `refreshSessions` |  | `RefreshSessionsCommand` | `BasicCommand` | `src/mongo/db/commands/refresh_sessions_command.cpp` |
| `removeShard` | `removeshard` | `RemoveShardCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_remove_shard_cmd.cpp` |
| `removeShardFromZone` | `removeshardfromzone` | `RemoveShardFromZoneCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_remove_shard_from_zone_cmd.cpp` |
| `renameCollection` |  | `CmdRenameCollection` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/rename_collection_cmd.cpp` |
| `renameCollection` |  | `RenameCollectionCmd` | `BasicCommand` | `src/mongo/s/commands/commands_public.cpp` |
| `repairShardedCollectionChunksHistory` |  | `RepairShardedCollectionChunksHistoryCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_repair_sharded_collection_chunks_history_cmd.cpp` |
| `replSetGetStatus` |  | `CmdReplSetGetStatus` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_repl_set_get_status_cmd.cpp` |
| `replSetResizeOplog` |  | `CmdReplSetResizeOplog` | `BasicCommand` | `src/mongo/db/commands/resize_oplog.cpp` |
| `resetError` | `reseterror` | `CmdResetErrorDeprecated` | `BasicCommand` | `src/mongo/db/commands/get_last_error.cpp` |
| `resetError` | `reseterror` | `CmdShardingResetErrorDeprecated` | `BasicCommand` | `src/mongo/s/commands/cluster_reset_error_cmd.cpp` |
| `revokePrivilegesFromRole` |  | `CmdRevokePrivilegesFromRole` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `revokePrivilegesFromRole` |  | `CmdRevokePrivilegesFromRole` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `revokeRolesFromRole` |  | `CmdRevokeRolesFromRole` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `revokeRolesFromRole` |  | `CmdRevokeRolesFromRole` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `revokeRolesFromUser` |  | `CmdRevokeRolesFromUser` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `revokeRolesFromUser` |  | `CmdRevokeRolesFromUser` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `rolesInfo` |  | `CmdRolesInfo` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `rolesInfo` |  | `CmdRolesInfo` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `serverStatus` |  | `CmdServerStatus` | `BasicCommand` | `src/mongo/db/commands/server_status_command.cpp` |
| `setCommittedSnapshot` |  | `CmdSetCommittedSnapshot` | `BasicCommand` | `src/mongo/db/commands/snapshot_management.cpp` |
| `setDefaultRWConcern` |  | `ClusterSetDefaultRWConcernCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_rwc_defaults_commands.cpp` |
| `setFeatureCompatibilityVersion` |  | `SetFeatureCompatibilityVersionCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_set_feature_compatibility_version_cmd.cpp` |
| `setIndexCommitQuorum` |  | `SetIndexCommitQuorumCommand` | `BasicCommand` | `src/mongo/s/commands/cluster_set_index_commit_quorum_cmd.cpp` |
| `setParameter` |  | `CmdSet` | `ErrmsgCommandDeprecated` | `src/mongo/db/commands/parameters.cpp` |
| `setProfilingFilterGlobally` |  | `SetProfilingFilterGloballyCmd` | `BasicCommand` | `src/mongo/db/commands/set_profiling_filter_globally_cmd.h` |
| `setShardVersion` |  | `SetShardVersion` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/set_shard_version_command.cpp` |
| `shardCollection` | `shardcollection` | `ShardCollectionCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_shard_collection_cmd.cpp` |
| `shardConnPoolStats` |  | `ShardedPoolStatsDeprecated` | `BasicCommand` | `src/mongo/db/commands/conn_pool_stats.cpp` |
| `shardingState` |  | `ShardingStateCmd` | `BasicCommand` | `src/mongo/db/s/sharding_state_command.cpp` |
| `shutdown` |  | `CmdShutdown` | `BasicCommand` | `src/mongo/db/commands/shutdown.h` |
| `sleep` |  | `CmdSleep` | `BasicCommand` | `src/mongo/db/commands/sleep_command.cpp` |
| `split` |  | `SplitCollectionCmd` | `ErrmsgCommandDeprecated` | `src/mongo/s/commands/cluster_split_cmd.cpp` |
| `splitChunk` |  | `SplitChunkCommand` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/split_chunk_command.cpp` |
| `splitVector` |  | `SplitVector` | `ErrmsgCommandDeprecated` | `src/mongo/db/s/split_vector_command.cpp` |
| `stageDebug` |  | `StageDebugCmd` | `BasicCommand` | `src/mongo/db/exec/stagedebug_cmd.cpp` |
| `startSession` |  | `StartSessionCommand` | `BasicCommand` | `src/mongo/db/commands/start_session_command.cpp` |
| `top` |  | `TopCommand` | `BasicCommand` | `src/mongo/db/commands/top_command.cpp` |
| `unsetSharding` |  | `UnsetShardingCommandDeprecated` | `BasicCommand` | `src/mongo/db/s/unset_sharding_command.cpp` |
| `updateRole` |  | `CmdUpdateRole` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `updateRole` |  | `CmdUpdateRole` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `updateUser` |  | `CmdUpdateUser` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `updateUser` |  | `CmdUpdateUser` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `updateZoneKeyRange` | `updatezonekeyRange` | `UpdateZoneKeyRangeCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_update_zone_key_range_cmd.cpp` |
| `usersInfo` |  | `CmdUsersInfo` | `BasicCommand` | `src/mongo/db/commands/user_management_commands.cpp` |
| `usersInfo` |  | `CmdUsersInfo` | `BasicCommand` | `src/mongo/s/commands/cluster_user_management_commands.cpp` |
| `validate` |  | `ValidateCmd` | `BasicCommand` | `src/mongo/db/commands/validate.cpp` |
| `validate` |  | `ValidateCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_validate_cmd.cpp` |
| `waitForOngoingChunkSplits` |  | `WaitForOngoingChunksSplitsCommand` | `BasicCommand` | `src/mongo/db/s/wait_for_ongoing_chunk_splits_command.cpp` |
| `whatsmysni` |  | `CmdWhatsMySNI` | `BasicCommand` | `src/mongo/db/commands/whats_my_sni_command.cpp` |
| `whatsmyuri` |  | `CmdWhatsMyUri` | `BasicCommand` | `src/mongo/db/commands/whats_my_uri_cmd.cpp` |
| `whatsmyuri` |  | `WhatsMyUriCmd` | `BasicCommand` | `src/mongo/s/commands/cluster_whats_my_uri_cmd.cpp` |
<!-- END v4_manual_commands.md -->

<!-- BEGIN v4_shell_flags.md -->
# v4 Legacy Shell Flags

- Total flags/options: `36`

| Flag | Type | Short | Single | Section | Default | Hidden | Positional | Conflicts | Description |
|---|---|---|---|---|---|---:|---|---|---|
| `authenticationDatabase` | `String` | `` | `` | `Authentication Options` | `` | `` | `` | `` | user source (defaults to dbname) |
| `authenticationMechanism` | `String` | `` | `` | `Authentication Options` | `` | `` | `` | `` | authentication mechanism |
| `autokillop` | `Switch` | `` | `` | `` | `` | `yes` | `` | `` | autokillop |
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
| `readMode` | `String` | `` | `` | `` | `` | `yes` | `` | `` | mode to determine how .find() querues are done: commands, compatibility, legacy |
| `retryWrites` | `Switch` | `` | `` | `` | `` | `` | `` | `` | automatically retry write operations upon transient network errors |
| `rpcProtocols` | `String` | `` | `` | `` | `` | `yes` | `` | `` |  none, opQueryOnly, opMsgOnly, all |
| `setShellParameter` | `StringMap` | `` | `` | `` | `` | `yes` | `` | `` | Set a configurable parameter |
| `shell` | `Switch` | `` | `` | `` | `` | `` | `` | `` | run the shell after executing files |
| `useLegacyWriteOps` | `Switch` | `` | `` | `` | `` | `yes` | `` | `` | use legacy write ops instead of write commands |
| `username` | `String` | `username` | `u` | `Authentication Options` | `` | `` | `` | `` | username for authentication |
| `verbose` | `Switch` | `` | `` | `` | `` | `` | `` | `` | increase verbosity |
| `version` | `Switch` | `` | `` | `` | `` | `` | `` | `` | show version information |
| `writeMode` | `String` | `` | `` | `` | `` | `yes` | `` | `` | mode to determine how writes are done: commands, compatibility, legacy |
<!-- END v4_shell_flags.md -->
