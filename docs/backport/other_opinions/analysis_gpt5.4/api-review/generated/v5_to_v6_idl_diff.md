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
