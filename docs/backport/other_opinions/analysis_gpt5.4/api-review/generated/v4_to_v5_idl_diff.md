# IDL Diff v4 -> v5

- Added commands: 84
- Removed commands: 3
- Changed commands: 47

## Added commands

- `_configsvrAbortReshardCollection`
- `_configsvrCleanupReshardCollection`
- `_configsvrCommitReshardCollection`
- `_configsvrRemoveChunks`
- `_configsvrRemoveTags`
- `_configsvrRenameCollectionMetadata`
- `_configsvrReshardCollection`
- `_configsvrSetAllowMigrations`
- `_flushDatabaseCacheUpdatesWithWriteConcern`
- `_flushReshardingStateChange`
- `_flushRoutingTableCacheUpdatesWithWriteConcern`
- `_getUserCacheGeneration`
- `_mergeAuthzCollections`
- `_shardsvrAbortReshardCollection`
- `_shardsvrCleanupReshardCollection`
- `_shardsvrCommitReshardCollection`
- `_shardsvrCreateCollection`
- `_shardsvrCreateCollectionParticipant`
- `_shardsvrDropCollection`
- `_shardsvrDropCollectionIfUUIDNotMatching`
- `_shardsvrDropCollectionParticipant`
- `_shardsvrDropDatabase`
- `_shardsvrDropDatabaseParticipant`
- `_shardsvrRefineCollectionShardKey`
- `_shardsvrRenameCollection`
- `_shardsvrRenameCollectionParticipant`
- `_shardsvrRenameCollectionParticipantUnblock`
- `_shardsvrReshardCollection`
- `_shardsvrReshardingOperationTime`
- `abortReshardCollection`
- `aggregate`
- `authenticate`
- `cleanupReshardCollection`
- `collMod`
- `commitReshardCollection`
- `createIndexes`
- `createRole`
- `createUser`
- `donorAbortMigration`
- `donorForgetMigration`
- `donorStartMigration`
- `drop`
- `dropAllRolesFromDatabase`
- `dropAllUsersFromDatabase`
- `dropDatabase`
- `dropIndexes`
- `dropRole`
- `dropUser`
- `endSessions`
- `explain`
- `find`
- `findAndModify`
- `getMore`
- `grantPrivilegesToRole`
- `grantRolesToRole`
- `grantRolesToUser`
- `hello`
- `invalidateUserCache`
- `killCursors`
- `listCollections`
- `listDatabases`
- `listIndexes`
- `logout`
- `mapReduce`
- `ping`
- `recipientForgetMigration`
- `recipientSyncData`
- `refreshSessions`
- `replSetTestEgress`
- `reshardCollection`
- `revokePrivilegesFromRole`
- `revokeRolesFromRole`
- `revokeRolesFromUser`
- `rolesInfo`
- `rotateCertificates`
- `saslContinue`
- `saslStart`
- `setFeatureCompatibilityVersion`
- `shutdown`
- `testReshardCloneCollection`
- `updateRole`
- `updateUser`
- `usersInfo`
- `validateDBMetadata`

## Removed commands

- `MapReduce`
- `_configsvrCreateCollection`
- `listDatabasesCommand`

## Changed commands

### `_addShard`
- Top-level changed: `api_version`

### `_cloneCollectionOptionsFromPrimaryShard`
- Top-level changed: `api_version`

### `_configsvrBalancerCollectionStatus`
- Top-level changed: `api_version`

### `_configsvrClearJumboFlag`
- Top-level changed: `api_version`

### `_configsvrCreateDatabase`
- Top-level changed: `api_version`, `reply_type`
- Fields added: `enableSharding`, `primaryShardId`

### `_configsvrEnsureChunkVersionIsGreaterThan`
- Top-level changed: `api_version`
- Fields added: `collectionUUID`, `nss`

### `_configsvrRefineCollectionShardKey`
- Top-level changed: `api_version`
- Fields added: `isFromPrimaryShard`

### `_flushDatabaseCacheUpdates`
- Top-level changed: `api_version`

### `_flushRoutingTableCacheUpdates`
- Top-level changed: `api_version`, `description`
- Fields modified: `syncFromConfig`

### `_killOperations`
- Top-level changed: `api_version`

### `_shardsvrSetAllowMigrations`
- Top-level changed: `api_version`

### `abortTransaction`
- Top-level changed: `strict`, `api_version`, `reply_type`
- Fields added: `recoveryToken`

### `autoSplitVector`
- Top-level changed: `api_version`

### `balancerCollectionStatus`
- Top-level changed: `api_version`

### `clearJumboFlag`
- Top-level changed: `api_version`

### `cloneCatalogData`
- Top-level changed: `api_version`

### `commitTransaction`
- Top-level changed: `strict`, `api_version`, `reply_type`

### `coordinateCommitTransaction`
- Top-level changed: `api_version`

### `count`
- Top-level changed: `api_version`, `reply_type`
- Fields modified: `fields`, `skip`

### `create`
- Top-level changed: `api_version`, `reply_type`
- Fields added: `clusteredIndex`, `expireAfterSeconds`, `timeseries`
- Fields removed: `writeConcern`
- Fields modified: `autoIndexId`, `collation`, `flags`, `indexOptionDefaults`, `max`, `recordPreImages`, `size`, `storageEngine`, `temp`, `validationAction`, `validationLevel`, `viewOn`

### `delete`
- Top-level changed: `api_version`, `reply_type`
- Fields added: `let`, `runtimeConstants`

### `distinct`
- Top-level changed: `api_version`

### `dropConnections`
- Top-level changed: `api_version`

### `exampleIncrement`
- Top-level changed: `api_version`

### `exampleMinimal`
- Top-level changed: `api_version`

### `exampleVoid`
- Top-level changed: `api_version`

### `fsyncUnlock`
- Top-level changed: `api_version`

### `getDatabaseVersion`
- Top-level changed: `api_version`

### `getDefaultRWConcern`
- Top-level changed: `api_version`

### `httpClientRequest`
- Top-level changed: `api_version`

### `insert`
- Top-level changed: `api_version`, `reply_type`

### `internalRenameIfOptionsAndIndexesMatch`
- Top-level changed: `api_version`

### `logMessage`
- Top-level changed: `api_version`

### `prepareTransaction`
- Top-level changed: `api_version`

### `profile`
- Top-level changed: `api_version`

### `refineCollectionShardKey`
- Top-level changed: `api_version`

### `renameCollection`
- Top-level changed: `api_version`

### `replSetResizeOplog`
- Top-level changed: `api_version`

### `setAllowMigrations`
- Top-level changed: `strict`, `api_version`, `description`

### `setDefaultRWConcern`
- Top-level changed: `api_version`

### `setIndexCommitQuorum`
- Top-level changed: `api_version`

### `setProfilingFilterGlobally`
- Top-level changed: `api_version`

### `startRecordingTraffic`
- Top-level changed: `api_version`

### `stopRecordingTraffic`
- Top-level changed: `api_version`

### `update`
- Top-level changed: `api_version`, `reply_type`
- Fields added: `let`
- Fields modified: `runtimeConstants`

### `voteCommitIndexBuild`
- Top-level changed: `api_version`

### `waitForFailPoint`
- Top-level changed: `api_version`
