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
