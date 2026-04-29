# IDL Diff v7 -> v8

- Added commands: 43
- Removed commands: 2
- Changed commands: 41

## Added commands

- `_configsvrCommitRefineCollectionShardKey`
- `_configsvrRemoveShard`
- `_shardsvrBeginMigrationBlockingOperation`
- `_shardsvrChangePrimary`
- `_shardsvrConvertToCapped`
- `_shardsvrConvertToCappedParticipant`
- `_shardsvrCoordinateMultiUpdate`
- `_shardsvrEndMigrationBlockingOperation`
- `_shardsvrJoinDDLCoordinators`
- `_shardsvrMovePrimary`
- `_shardsvrRecreateRangeDeletionTasks`
- `_shardsvrRecreateRangeDeletionTasksParticipant`
- `_shardsvrRunSearchIndexCommand`
- `_shardsvrUntrackUnsplittableCollection`
- `abortMoveCollection`
- `abortUnshardCollection`
- `autoCompact`
- `changePrimary`
- `cleanupStructuredEncryptionData`
- `compact`
- `createSearchIndexes`
- `createUnsplittableCollection`
- `dropSearchIndex`
- `exampleVoidAllowedOnSecondaryIfOptedIn`
- `exampleVoidAlwaysAllowedOnSecondary`
- `exampleVoidNeverAllowedOnSecondary`
- `listSearchIndexes`
- `lockInfo`
- `moveCollection`
- `movePrimary`
- `recreateRangeDeletionTasks`
- `removeQuerySettings`
- `removeShard`
- `replicateSearchIndexCommand`
- `rotateFTDC`
- `setQuerySettings`
- `startXRayLogging`
- `stopXRayLogging`
- `sysprofile`
- `transitionToShardedCluster`
- `unshardCollection`
- `untrackUnshardedCollection`
- `updateSearchIndex`

## Removed commands

- `_configsvrRenameCollectionMetadata`
- `_shardsvrDropCollectionIfUUIDNotMatching`

## Changed commands

### `_configsvrAbortReshardCollection`
- Fields added: `provenance`

### `_configsvrRefineCollectionShardKey`
- Top-level changed: `reply_type`

### `_configsvrReshardCollection`
- Fields added: `forceRedistribution`, `numSamplesPerChunk`, `provenance`, `recipientOplogBatchTaskCount`, `reshardingUUID`, `shardDistribution`

### `_configsvrSetClusterParameter`
- Fields added: `previousTime`

### `_flushDatabaseCacheUpdatesWithWriteConcern`
- Top-level changed: `reply_type`

### `_flushReshardingStateChange`
- Top-level changed: `reply_type`

### `_flushRoutingTableCacheUpdatesWithWriteConcern`
- Top-level changed: `reply_type`

### `_shardsvrAbortReshardCollection`
- Top-level changed: `reply_type`

### `_shardsvrCollModParticipant`
- Top-level changed: `reply_type`

### `_shardsvrCommitReshardCollection`
- Top-level changed: `reply_type`

### `_shardsvrDropCollectionIfUUIDNotMatchingWithWriteConcern`
- Top-level changed: `reply_type`

### `_shardsvrDropCollectionParticipant`
- Top-level changed: `reply_type`
- Fields added: `collectionUUID`, `requireCollectionEmpty`

### `_shardsvrDropDatabaseParticipant`
- Top-level changed: `reply_type`

### `_shardsvrMoveRange`
- Fields added: `collectionTimestamp`
- Fields modified: `epoch`

### `_shardsvrParticipantBlock`
- Top-level changed: `reply_type`
- Fields added: `clearFilteringMetadata`, `throwIfReasonDiffers`
- Fields removed: `allowViews`

### `_shardsvrRenameCollection`
- Top-level changed: `reply_type`

### `_shardsvrRenameCollectionParticipant`
- Top-level changed: `reply_type`
- Fields added: `newTargetCollectionUuid`

### `_shardsvrRenameCollectionParticipantUnblock`
- Top-level changed: `reply_type`

### `_shardsvrRenameIndexMetadata`
- Top-level changed: `reply_type`

### `_shardsvrReshardingOperationTime`
- Top-level changed: `description`

### `aggregate`
- Fields added: `$_isHybridSearch`, `$_requestResumeToken`, `$_resumeAfter`, `$_startAt`, `includeQueryStatsMetrics`, `querySettings`
- Fields removed: `$_generateV2ResumeTokens`

### `autoSplitVector`
- Top-level changed: `description`

### `bulkWrite`
- Fields added: `$_originalCollation`, `$_originalQuery`, `errorsOnly`, `let`

### `compactStructuredEncryptionData`
- Fields added: `anchorPaddingFactor`, `encryptionInformation`
- Fields modified: `compactionTokens`

### `count`
- Fields modified: `maxTimeMS`

### `create`
- Fields removed: `autoIndexId`, `capped`, `changeStreamPreAndPostImages`, `clusteredIndex`, `collation`, `encryptedFields`, `expireAfterSeconds`, `flags`, `idIndex`, `indexOptionDefaults`, `max`, `pipeline`, `size`, `storageEngine`, `temp`, `timeseries`, `validationAction`, `validationLevel`, `validator`, `viewOn`

### `createRole`
- Fields modified: `privileges`

### `distinct`
- Fields added: `hint`, `querySettings`

### `donorStartMigration`
- Fields removed: `donorCertificateForRecipient`, `recipientCertificateForDonor`

### `find`
- Fields added: `includeQueryStatsMetrics`, `querySettings`
- Fields modified: `$_requestResumeToken`, `$_resumeAfter`, `allowSpeculativeMajorityRead`, `maxTimeMS`, `readOnce`, `term`

### `findAndModify`
- Fields added: `isTimeseriesNamespace`

### `getClusterParameter`
- Fields added: `omitInFTDC`

### `getMore`
- Fields added: `includeQueryStatsMetrics`

### `getQueryableEncryptionCountInfo`
- Fields modified: `queryType`

### `grantPrivilegesToRole`
- Fields modified: `privileges`

### `prepareTransaction`
- Top-level changed: `reply_type`

### `recipientVoteImportedFiles`
- Top-level changed: `description`
- Fields removed: `reason`, `success`

### `reshardCollection`
- Fields added: `forceRedistribution`, `numSamplesPerChunk`, `reshardingUUID`, `shardDistribution`

### `revokePrivilegesFromRole`
- Fields modified: `privileges`

### `testReshardCloneCollection`
- Fields added: `reshardingUUID`

### `updateRole`
- Fields modified: `privileges`
