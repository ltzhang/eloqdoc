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
