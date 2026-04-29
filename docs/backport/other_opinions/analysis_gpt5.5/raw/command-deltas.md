# Version command deltas


## v4.0 -> v5.0

### Added commands

- `APIVersion1CommandRuntimeName`
- `APIVersion1CommandRuntimeName2`
- `AccessCheckComplexPrivilegeCommandName`
- `AccessCheckNoneCommandName`
- `AccessCheckSimpleAccessCheckCommandName`
- `AccessCheckSimplePrivilegeCommandName`
- `BasicConcatenateWithDbCommand`
- `BasicConcatenateWithDbOrUUIDCommand`
- `BasicIgnoredCommand`
- `BasicNamespaceConstGetterCommand`
- `CommandTypeArrayObjectCommand`
- `CommandTypeArrayStructCommand`
- `CommandTypeStringCommand`
- `CommandTypeStructCommand`
- `CommandTypeStructValidatorCommand`
- `CommandTypeVariantCommand`
- `CommandTypeVariantStructCommand`
- `CommandWithAnyTypeOwnedMember`
- `CommandWithOkReply`
- `CommandWithReplyType`
- `CommandWithValueTypeMember`
- `DocSequenceCommand`
- `DocSequenceCommandNonStrict`
- `KnownFieldCommand`
- `NewCommandName`
- `_addShard`
- `_cloneCollectionOptionsFromPrimaryShard`
- `_configsvrAbortReshardCollection`
- `_configsvrBalancerCollectionStatus`
- `_configsvrCleanupReshardCollection`
- `_configsvrClearJumboFlag`
- `_configsvrCommitReshardCollection`
- `_configsvrCreateDatabase`
- `_configsvrEnsureChunkVersionIsGreaterThan`
- `_configsvrRefineCollectionShardKey`
- `_configsvrRemoveChunks`
- `_configsvrRemoveTags`
- `_configsvrRenameCollectionMetadata`
- `_configsvrReshardCollection`
- `_configsvrSetAllowMigrations`
- `_flushDatabaseCacheUpdates`
- `_flushDatabaseCacheUpdatesWithWriteConcern`
- `_flushReshardingStateChange`
- `_flushRoutingTableCacheUpdates`
- `_flushRoutingTableCacheUpdatesWithWriteConcern`
- `_killOperations`
- `_shardsvrAbortReshardCollection`
- `_shardsvrCleanupReshardCollection`
- `_shardsvrCloneCatalogData`
- `_shardsvrCommitReshardCollection`
- `_shardsvrCreateCollection`
- `_shardsvrCreateCollectionParticipant`
- `_shardsvrDropCollection`
- `_shardsvrDropCollectionIfUUIDNotMatching`
- `_shardsvrDropCollectionParticipant`
- `_shardsvrDropDatabase`
- `_shardsvrDropDatabaseParticipant`
- `_shardsvrMovePrimary`
- `_shardsvrRefineCollectionShardKey`
- `_shardsvrRenameCollection`
- `_shardsvrRenameCollectionParticipant`
- `_shardsvrRenameCollectionParticipantUnblock`
- `_shardsvrReshardCollection`
- `_shardsvrReshardingOperationTime`
- `_shardsvrSetAllowMigrations`
- `_underscore_command`
- `abortReshardCollection`
- `autoSplitVector`
- `balancerCollectionStatus`
- `buildinfo`
- `chained_command_type_mixed`
- `cleanupReshardCollection`
- `clearJumboFlag`
- `cloneCatalogData`
- `commitReshardCollection`
- `coordinateCommitTransaction`
- `createIndexes`
- `cst`
- `donorAbortMigration`
- `donorForgetMigration`
- `donorStartMigration`
- `dropConnections`
- `exampleIncrement`
- `exampleMinimal`
- `exampleVoid`
- `fsyncUnlock`
- `getDatabaseVersion`
- `getDefaultRWConcern`
- `hello`
- `httpClientRequest`
- `int_array_type_command`
- `int_type_command`
- `internalRenameIfOptionsAndIndexesMatch`
- `pinHistoryReplicated`
- `recipientForgetMigration`
- `recipientSyncData`
- `refineCollectionShardKey`
- `replSetTestEgress`
- `reshardCollection`
- `rotateCertificates`
- `saslContinue`
- `saslStart`
- `sbe`
- `setAllowMigrations`
- `setDefaultRWConcern`
- `setIndexCommitQuorum`
- `setProfilingFilterGlobally`
- `splitVector`
- `startRecordingTraffic`
- `stopRecordingTraffic`
- `testDeprecation`
- `testDeprecationInVersion2`
- `testRemoval`
- `testReshardCloneCollection`
- `testVersion2`
- `testVersions1And2`
- `validateDBMetadata`
- `validated_command`
- `voteCommitIndexBuild`
- `waitForFailPoint`
- `waitForOngoingChunkSplits`
- `whatsmysni`

### Removed commands

- `ReplSetRequestVotes`
- `_cloneCatalogData`
- `_movePrimary`
- `balancerStart`
- `balancerStop`
- `clone`
- `copydb`
- `doTxn`
- `eval`
- `geoNear`
- `getFreeMonitoringStatus`
- `getPrevError`
- `group`
- `invalidateViewCatalog`
- `journalLatencyTest`
- `mapreduce.shardedfinish`
- `parallelCollectionScan`
- `planCacheListPlans`
- `planCacheListQueryShapes`
- `refreshSessionsInternal`
- `repairCursor`
- `replSetElect`
- `replSetFresh`
- `resetError`
- `restartCatalog`
- `setFreeMonitoring`
- `shardConnPoolStats`
- `unsetSharding`

### Added data-relevant commands

- `createIndexes`

### Removed data-relevant commands


## v5.0 -> v6.0

### Added commands

- `_configsvrCollMod`
- `_configsvrConfigureCollectionBalancing`
- `_configsvrMoveRange`
- `_configsvrRunRestore`
- `_configsvrSetClusterParameter`
- `_configsvrSetUserWriteBlockMode`
- `_recvChunkReleaseCritSec`
- `_shardsvrCollMod`
- `_shardsvrCollModParticipant`
- `_shardsvrCompactStructuredEncryptionData`
- `_shardsvrDropIndexes`
- `_shardsvrGetStatsForBalancing`
- `_shardsvrJoinMigrations`
- `_shardsvrMoveRange`
- `_shardsvrParticipantBlock`
- `_shardsvrSetClusterParameter`
- `_shardsvrSetUserWriteBlockMode`
- `abortShardSplit`
- `commitShardSplit`
- `compactStructuredEncryptionData`
- `configureCollectionBalancing`
- `enableSharding`
- `forgetShardSplit`
- `getClusterParameter`
- `getLog`
- `moveRange`
- `recipientVoteImportedFiles`
- `setClusterParameter`
- `setUserWriteBlockMode`
- `testInternalTransactions`
- `user`

### Removed commands

- `_cloneCollectionOptionsFromPrimaryShard`
- `_configsvrCommitChunkMerge`
- `_configsvrDropCollection`
- `_configsvrDropDatabase`
- `_configsvrEnableSharding`
- `_configsvrMovePrimary`
- `_configsvrShardCollection`
- `_shardsvrShardCollection`
- `moveChunk`
- `sbe`

### Added data-relevant commands

- `compactStructuredEncryptionData`

### Removed data-relevant commands


## v6.0 -> v7.0

### Added commands

- `CommandTypeNamespaceCommand`
- `CommandWithBypassAndNamespaceStruct`
- `CommandWithNamespaceMember`
- `CommandWithNamespaceStruct`
- `DocSequenceCommandArrayVariant`
- `_clusterQueryWithoutShardKey`
- `_clusterWriteWithoutShardKey`
- `_configsvrCheckClusterMetadataConsistency`
- `_configsvrCheckMetadataConsistency`
- `_configsvrCommitIndex`
- `_configsvrCommitMergeAllChunksOnShard`
- `_configsvrDropIndexCatalogEntry`
- `_configsvrGetHistoricalPlacement`
- `_configsvrResetPlacementHistory`
- `_configsvrTransitionFromDedicatedConfigServer`
- `_configsvrTransitionToDedicatedConfigServer`
- `_hashBSONElement`
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
- `cloneCollectionAsCapped`
- `configureQueryAnalyzer`
- `dbHash`
- `dbStats`
- `findandmodify`
- `fsync`
- `getChangeStreamState`
- `getParameter`
- `getQueryableEncryptionCountInfo`
- `godinsert`
- `listDatabasesForAllTenants`
- `mergeAllChunksOnShard`
- `reIndex`
- `resetPlacementHistory`
- `setChangeStreamState`
- `setParameter`
- `transitionFromDedicatedConfigServer`
- `transitionToDedicatedConfigServer`
- `unauthorizedCmd`
- `voteAbortIndexBuild`

### Removed commands

- `_configsvrMoveChunk`
- `admin`
- `availableQueryOptions`
- `getLastError`
- `getnonce`
- `user`

### Added data-relevant commands

- `analyze`
- `bulkWrite`
- `dbStats`
- `getQueryableEncryptionCountInfo`

### Removed data-relevant commands


## v7.0 -> v8.0

### Added commands

- `_configsvrCommitRefineCollectionShardKey`
- `_dropConnectionsToMongot`
- `_mongotConnPoolStats`
- `_shardsvrBeginMigrationBlockingOperation`
- `_shardsvrChangePrimary`
- `_shardsvrCleanupStructuredEncryptionData`
- `_shardsvrConvertToCapped`
- `_shardsvrConvertToCappedParticipant`
- `_shardsvrCoordinateMultiUpdate`
- `_shardsvrEndMigrationBlockingOperation`
- `_shardsvrJoinDDLCoordinators`
- `_shardsvrRecreateRangeDeletionTasks`
- `_shardsvrRecreateRangeDeletionTasksParticipant`
- `_shardsvrRunSearchIndexCommand`
- `_shardsvrUntrackUnsplittableCollection`
- `abortMoveCollection`
- `abortUnshardCollection`
- `autoCompact`
- `changePrimary`
- `cleanupStructuredEncryptionData`
- `createSearchIndexes`
- `createUnsplittableCollection`
- `dropSearchIndex`
- `exampleVoidAllowedOnSecondaryIfOptedIn`
- `exampleVoidAlwaysAllowedOnSecondary`
- `exampleVoidNeverAllowedOnSecondary`
- `getShardingReady`
- `listSearchIndexes`
- `moveCollection`
- `planShardedSearch`
- `recreateRangeDeletionTasks`
- `removeQuerySettings`
- `replicateSearchIndexCommand`
- `rotateFTDC`
- `search`
- `setQuerySettings`
- `startXRayLogging`
- `stopXRayLogging`
- `sysprofile`
- `testCommandFeatureFlaggedOnLatestFCV`
- `timeseriesCatalogBucketParamsChanged`
- `transitionToShardedCluster`
- `unshardCollection`
- `untrackUnshardedCollection`
- `updateSearchIndex`
- `vectorSearch`

### Removed commands

- `_configsvrRenameCollectionMetadata`
- `_shardsvrDropCollectionIfUUIDNotMatching`
- `cst`
- `throwsStatus`
- `unauthorizedCmd`
- `waitForOngoingChunkSplits`

### Added data-relevant commands

- `cleanupStructuredEncryptionData`
- `createSearchIndexes`
- `dropSearchIndex`
- `listSearchIndexes`
- `removeQuerySettings`
- `setQuerySettings`
- `updateSearchIndex`

### Removed data-relevant commands

