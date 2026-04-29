# v5 IDL Command Inventory

## Summary
- Command count: 131

## `_addShard`
- source=`src/mongo/db/s/add_shard_cmd.idl`, namespace=`ignored`, strict=`None`, api_version=``
- _addShard Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `shardIdentity` | `ShardIdentity` | `False` | `` | `False` | `` | `` |

## `_cloneCollectionOptionsFromPrimaryShard`
- source=`src/mongo/s/request_types/clone_collection_options_from_primary_shard.idl`, namespace=`type`, strict=`True`, api_version=``
- Internal command to create a collection on a non-primary shard with the collection options from the primary shard.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `primaryShard` | `string` | `False` | `` | `False` | `` | `` |

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
- source=`src/mongo/s/request_types/clear_jumbo_flag.idl`, namespace=`type`, strict=`False`, api_version=``
- internal clearJumboFlag command for config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `epoch` | `objectid` | `False` | `` | `False` | `` | `` |
| `minKey` | `object` | `False` | `` | `False` | `` | `` |
| `maxKey` | `object` | `False` | `` | `False` | `` | `` |

## `_configsvrCommitReshardCollection`
- source=`src/mongo/s/request_types/commit_reshard_collection.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal commitReshardCollection command on the config server.
_No IDL fields declared._

## `_configsvrCreateDatabase`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`type`, strict=`False`, api_version=``, reply=`ConfigsvrCreateDatabaseResponse`
- The internal createDatabase command on the config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `primaryShardId` | `string` | `True` | `` | `False` | `` | `` |
| `enableSharding` | `bool` | `True` | `` | `False` | `` | `` |

## `_configsvrEnsureChunkVersionIsGreaterThan`
- source=`src/mongo/s/request_types/ensure_chunk_version_is_greater_than.idl`, namespace=`ignored`, strict=`False`, api_version=``
- If a chunk matching 'requestedChunk' exists, bumps the chunk's version to one greater than the current collection version.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `minKey` | `object` | `False` | `` | `False` | `` | `` |
| `maxKey` | `object` | `False` | `` | `False` | `` | `` |
| `version` | `ChunkVersion` | `False` | `` | `False` | `` | `` |
| `collectionUUID` | `uuid` | `True` | `` | `False` | `` | `` |
| `nss` | `namespacestring` | `True` | `` | `False` | `` | `` |

## `_configsvrRefineCollectionShardKey`
- source=`src/mongo/s/request_types/refine_collection_shard_key.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal refineCollectionShardKey command on the config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `object` | `False` | `` | `False` | `` | `` |
| `epoch` | `objectid` | `False` | `` | `False` | `` | `` |
| `isFromPrimaryShard` | `bool` | `True` | `` | `False` | `` | `` |
| `enforceUniquenessCheck` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `_configsvrRemoveChunks`
- source=`src/mongo/db/s/remove_chunks.idl`, namespace=`ignored`, strict=`False`, api_version=``
- internal _configsvrRemoveChunks command for config server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `collectionUUID` | `uuid` | `False` | `` | `False` | `` | `` |

## `_configsvrRemoveTags`
- source=`src/mongo/s/request_types/remove_tags.idl`, namespace=`type`, strict=`False`, api_version=``
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
_No IDL fields declared._

## `_shardsvrDropCollectionIfUUIDNotMatching`
- source=`src/mongo/s/request_types/drop_collection_if_uuid_not_matching.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal dropCollectionIfUUIDNotMatching request.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `expectedCollectionUUID` | `uuid` | `False` | `` | `False` | `` | `` |

## `_shardsvrDropCollectionParticipant`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Parser for the _shardsvrDropCollectionParticipant command
_No IDL fields declared._

## `_shardsvrDropDatabase`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`ignored`, strict=`None`, api_version=``
- Internal command sent to the primary shard of a database to drop it.
_No IDL fields declared._

## `_shardsvrDropDatabaseParticipant`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`ignored`, strict=`False`, api_version=``
- Internal command sent to participants shards to drop a database.
_No IDL fields declared._

## `_shardsvrRefineCollectionShardKey`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Parser for the _shardsvrRefineCollectionShardKey command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `newShardKey` | `KeyPattern` | `False` | `` | `False` | `` | `` |
| `enforceUniquenessCheck` | `optionalBool` | `False` | `` | `False` | `` | `` |

## `_shardsvrRenameCollection`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal renameCollection command for a shard.
_No IDL fields declared._

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
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `object` | `False` | `` | `False` | `` | `` |
| `unique` | `bool` | `True` | `` | `False` | `` | `` |
| `numInitialChunks` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `collation` | `object` | `True` | `` | `False` | `` | `` |
| `zones` | `array<ReshardingZoneType>` | `True` | `` | `False` | `` | `` |
| `_presetReshardedChunks` | `array<ReshardedChunk>` | `True` | `` | `False` | `` | `` |

## `_shardsvrReshardingOperationTime`
- source=`src/mongo/s/request_types/resharding_operation_time.idl`, namespace=`type`, strict=`True`, api_version=``
- An internal command to query the operation time for participant shards.
_No IDL fields declared._

## `_shardsvrSetAllowMigrations`
- source=`src/mongo/s/request_types/sharded_ddl_commands.idl`, namespace=`concatenate_with_db`, strict=`False`, api_version=``
- Internal setAllowMigrations command for a shard.
_No IDL fields declared._

## `abortReshardCollection`
- source=`src/mongo/s/request_types/abort_reshard_collection.idl`, namespace=`type`, strict=`True`, api_version=``
- The public abortReshardCollection command on mongos.
_No IDL fields declared._

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
| `use44SortKeys` | `bool` | `False` | `` | `True` | `` | `` |
| `useNewUpsert` | `bool` | `False` | `` | `True` | `` | `` |
| `$_passthroughToShard` | `PassthroughToShardOptions` | `True` | `` | `True` | `passthroughToShard` | `` |

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

## `balancerCollectionStatus`
- source=`src/mongo/s/request_types/balancer_collection_status.idl`, namespace=`type`, strict=`True`, api_version=``
- Public balancerCollectionStatus command on mongos
_No IDL fields declared._

## `cleanupReshardCollection`
- source=`src/mongo/s/request_types/cleanup_reshard_collection.idl`, namespace=`type`, strict=`True`, api_version=``
- The public cleanupReshardCollection command on mongos.
_No IDL fields declared._

## `clearJumboFlag`
- source=`src/mongo/s/request_types/clear_jumbo_flag.idl`, namespace=`type`, strict=`False`, api_version=``
- clearJumboFlag command for mongos
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `bounds` | `array<object>` | `True` | `` | `False` | `` | `` |
| `find` | `object` | `True` | `` | `False` | `` | `` |

## `cloneCatalogData`
- source=`src/mongo/s/request_types/clone_catalog_data.idl`, namespace=`type`, strict=`False`, api_version=``
- The internal cloneCatalogData command on a shard
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `from` | `string` | `False` | `` | `False` | `` | `` |

## `collMod`
- source=`src/mongo/db/coll_mod.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`CollModReply`
- Specify collMod Command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `index` | `CollModIndex` | `True` | `` | `False` | `` | `` |
| `validator` | `object` | `True` | `` | `False` | `` | `` |
| `validationLevel` | `ValidationLevel` | `True` | `` | `False` | `` | `` |
| `validationAction` | `ValidationAction` | `True` | `` | `False` | `` | `` |
| `viewOn` | `string` | `True` | `` | `False` | `` | `{"callback": "create_command_validation::validateViewOnNotEmpty"}` |
| `pipeline` | `array<object>` | `True` | `` | `False` | `` | `` |
| `recordPreImages` | `safeBool` | `True` | `` | `False` | `` | `` |
| `expireAfterSeconds` | `{'variant': ['string', 'safeInt64']}` | `True` | `` | `False` | `` | `` |
| `timeseries` | `CollModTimeseries` | `True` | `` | `False` | `` | `` |
| `isTimeseriesNamespace` | `bool` | `True` | `` | `False` | `` | `` |

## `commitReshardCollection`
- source=`src/mongo/s/request_types/commit_reshard_collection.idl`, namespace=`type`, strict=`True`, api_version=``
- The public commitReshardCollection command on mongos.
_No IDL fields declared._

## `commitTransaction`
- source=`src/mongo/db/commands/txn_cmds.idl`, namespace=`ignored`, strict=`True`, api_version=`1`, reply=`OkReply`
- commitTransaction Command
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `commitTimestamp` | `timestamp` | `True` | `` | `False` | `` | `` |
| `recoveryToken` | `TxnRecoveryToken` | `True` | `` | `False` | `` | `` |

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
| `timeseries` | `TimeseriesOptions` | `True` | `` | `False` | `` | `` |
| `clusteredIndex` | `safeBool` | `True` | `` | `False` | `` | `` |
| `expireAfterSeconds` | `safeInt64` | `True` | `` | `False` | `` | `` |
| `temp` | `safeBool` | `True` | `` | `True` | `` | `` |
| `flags` | `safeInt64` | `True` | `` | `True` | `` | `` |

## `createIndexes`
- source=`src/mongo/db/create_indexes.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`CreateIndexesReply`
- Command for creating indexes on a collection
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `v` | `safeInt` | `False` | `2` | `False` | `` | `` |
| `indexes` | `array<object>` | `False` | `` | `False` | `` | `` |
| `ignoreUnknownIndexOptions` | `safeBool` | `False` | `False` | `False` | `` | `` |
| `commitQuorum` | `CommitQuorum` | `True` | `` | `False` | `` | `` |
| `isTimeseriesNamespace` | `bool` | `True` | `` | `False` | `` | `` |

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
| `tenantId` | `string` | `False` | `` | `False` | `` | `{"callback": "tenant_migration_util::validateDatabasePrefix"}` |
| `readPreference` | `readPreference` | `False` | `` | `False` | `` | `` |
| `donorCertificateForRecipient` | `TenantMigrationPEMPayload` | `True` | `` | `False` | `` | `` |
| `recipientCertificateForDonor` | `TenantMigrationPEMPayload` | `True` | `` | `False` | `` | `` |

## `drop`
- source=`src/mongo/db/drop.idl`, namespace=`concatenate_with_db`, strict=`True`, api_version=`1`, reply=`DropReply`
- Parser for the drop command
_No IDL fields declared._

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
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `index` | `{'variant': ['string', 'array<string>', 'object']}` | `False` | `` | `False` | `` | `` |
| `isTimeseriesNamespace` | `bool` | `True` | `` | `False` | `` | `` |

## `dropRole`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Drops a single role. Before deleting the role completely it must remove it from any users or roles that reference it. If any errors occur in the middle of that process it's possible to be left in a state where the role has been removed from some user/roles but otherwise still exists.
_No IDL fields declared._

## `dropUser`
- source=`src/mongo/db/commands/user_management_commands.idl`, namespace=`type`, strict=`True`, api_version=``
- Drop a single user
_No IDL fields declared._

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
| `collation` | `IDLAnyType` | `False` | `` | `True` | `` | `` |
| `use44SortKeys` | `IDLAnyType` | `False` | `` | `True` | `` | `` |
| `useNewUpsert` | `IDLAnyType` | `False` | `` | `True` | `` | `` |

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
| `_use44SortKeys` | `bool` | `True` | `` | `True` | `` | `` |
| `maxTimeMS` | `maxTimeMS` | `True` | `` | `False` | `` | `{"gte": 0}` |
| `readConcern` | `object_owned` | `True` | `` | `False` | `` | `` |
| `runtimeConstants` | `LegacyRuntimeConstants` | `True` | `` | `True` | `legacyRuntimeConstants` | `` |

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
| `bypassDocumentValidation` | `safeBool` | `True` | `` | `False` | `` | `` |
| `let` | `object` | `True` | `` | `False` | `` | `` |
| `runtimeConstants` | `LegacyRuntimeConstants` | `True` | `` | `False` | `legacyRuntimeConstants` | `` |
| `writeConcern` | `object` | `True` | `` | `False` | `` | `` |
| `bypassEmptyTsReplacement` | `optionalBool` | `False` | `` | `True` | `` | `` |

## `fsyncUnlock`
- source=`src/mongo/s/commands/cluster_fsync_unlock_cmd.idl`, namespace=`ignored`, strict=`False`, api_version=``
- The command for calling fsync unlock on all shards of a cluster.
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
| `isTimeseriesNamespace` | `bool` | `True` | `` | `False` | `` | `` |

## `logMessage`
- source=`src/mongo/db/commands/generic.idl`, namespace=`type`, strict=`True`, api_version=``
- Log a message on the server
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `extra` | `object` | `True` | `` | `False` | `` | `` |
| `severity` | `MessageSeverity` | `False` | `kLog` | `False` | `` | `` |
| `debugLevel` | `int` | `True` | `` | `False` | `` | `{"gte": 1, "lte": 5}` |

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
| `sampleRate` | `double` | `True` | `` | `False` | `` | `` |
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

## `refineCollectionShardKey`
- source=`src/mongo/s/request_types/refine_collection_shard_key.idl`, namespace=`type`, strict=`False`, api_version=``
- The public refineCollectionShardKey command on mongos
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `key` | `object` | `False` | `` | `False` | `` | `` |
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
| `dropTarget` | `bool` | `False` | `False` | `False` | `` | `` |
| `stayTemp` | `bool` | `False` | `False` | `False` | `` | `` |

## `replSetResizeOplog`
- source=`src/mongo/db/commands/resize_oplog.idl`, namespace=`type`, strict=`False`, api_version=``
- Parser for the 'replSetResizeOplog' command.
| Field | Type | Opt | Default | Unstable | C++ Name | Validator |
|---|---|---:|---|---:|---|---|
| `size` | `double` | `True` | `` | `False` | `` | `{"gte": 990, "lte": 1073741824}` |
| `minRetentionHours` | `double` | `True` | `` | `False` | `` | `{"gte": 0}` |

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
