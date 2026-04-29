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