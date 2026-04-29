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