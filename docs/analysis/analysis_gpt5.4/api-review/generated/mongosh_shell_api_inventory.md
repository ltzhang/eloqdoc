# mongosh Shell API Method Inventory

| File | Line | Method | Server Versions | API Versions | Topologies |
|---|---:|---|---|---|---|
| `database.ts` | 364 | `getMongo` |  |  |  |
| `database.ts` | 379 | `getCollectionNames` |  | `[1]` |  |
| `database.ts` | 395 | `getCollectionInfos` | `3.0.0` -> `ServerVersions.latest` | `[1]` |  |
| `database.ts` | 412 | `runCommand` |  | `[1]` |  |
| `database.ts` | 445 | `adminCommand` | `3.4.0` -> `ServerVersions.latest` | `[1]` |  |
| `database.ts` | 478 | `aggregate` |  | `[1]` |  |
| `database.ts` | 576 | `dropDatabase` |  | `[1]` |  |
| `database.ts` | 585 | `createUser` |  | `[]` |  |
| `database.ts` | 633 | `updateUser` |  |  |  |
| `database.ts` | 681 | `changeUserPassword` |  | `[]` |  |
| `database.ts` | 710 | `logout` |  | `[]` |  |
| `database.ts` | 718 | `dropUser` |  | `[]` |  |
| `database.ts` | 733 | `dropAllUsers` |  | `[]` |  |
| `database.ts` | 743 | `auth` |  |  |  |
| `database.ts` | 788 | `grantRolesToUser` |  | `[]` |  |
| `database.ts` | 808 | `revokeRolesFromUser` |  | `[]` |  |
| `database.ts` | 828 | `getUser` |  | `[]` |  |
| `database.ts` | 855 | `getUsers` |  | `[]` |  |
| `database.ts` | 863 | `createCollection` |  | `[1]` |  |
| `database.ts` | 881 | `createEncryptedCollection` |  | `[1]` |  |
| `database.ts` | 896 | `createView` |  | `[1]` |  |
| `database.ts` | 930 | `createRole` |  | `[]` |  |
| `database.ts` | 953 | `updateRole` |  | `[]` |  |
| `database.ts` | 980 | `dropRole` |  | `[]` |  |
| `database.ts` | 994 | `dropAllRoles` |  |  |  |
| `database.ts` | 1005 | `grantRolesToRole` |  | `[]` |  |
| `database.ts` | 1025 | `revokeRolesFromRole` |  | `[]` |  |
| `database.ts` | 1044 | `grantPrivilegesToRole` |  |  |  |
| `database.ts` | 1067 | `revokePrivilegesFromRole` |  | `[]` |  |
| `database.ts` | 1090 | `getRole` |  | `[]` |  |
| `database.ts` | 1117 | `getRoles` |  | `[]` |  |
| `database.ts` | 1178 | `currentOp` |  | `[]` |  |
| `database.ts` | 1189 | `killOp` |  | `[]` |  |
| `database.ts` | 1199 | `shutdownServer` |  | `[]` |  |
| `database.ts` | 1209 | `fsyncLock` |  | `[]` |  |
| `database.ts` | 1219 | `fsyncUnlock` |  | `[]` |  |
| `database.ts` | 1228 | `version` |  | `[]` |  |
| `database.ts` | 1246 | `serverBits` |  | `[]` |  |
| `database.ts` | 1264 | `isMaster` |  | `[]` |  |
| `database.ts` | 1276 | `hello` | `5.0.0` -> `ServerVersions.latest` | `[1]` |  |
| `database.ts` | 1296 | `serverBuildInfo` |  | `[]` |  |
| `database.ts` | 1305 | `serverStatus` |  | `[]` |  |
| `database.ts` | 1315 | `stats` |  | `[]` |  |
| `database.ts` | 1334 | `hostInfo` |  | `[]` |  |
| `database.ts` | 1343 | `serverCmdLineOpts` |  | `[]` |  |
| `database.ts` | 1353 | `rotateCertificates` | `5.0.0` -> `ServerVersions.latest` | `[]` |  |
| `database.ts` | 1363 | `printCollectionStats` |  | `[]` |  |
| `database.ts` | 1385 | `getProfilingStatus` |  | `[]` |  |
| `database.ts` | 1394 | `setProfilingLevel` |  | `[]` |  |
| `database.ts` | 1417 | `setLogLevel` |  | `[]` |  |
| `database.ts` | 1450 | `getLogComponents` |  | `[]` |  |
| `database.ts` | 1465 | `cloneDatabase` |  |  |  |
| `database.ts` | 1472 | `cloneCollection` |  |  |  |
| `database.ts` | 1479 | `copyDatabase` |  |  |  |
| `database.ts` | 1487 | `commandHelp` |  | `[1]` |  |
| `database.ts` | 1508 | `listCommands` |  | `[]` |  |
| `database.ts` | 1540 | `getLastErrorObj` |  | `[]` |  |
| `database.ts` | 1560 | `getLastError` |  | `[]` |  |
| `database.ts` | 1576 | `printShardingStatus` |  | `[1]` | `['Sharded']` |
| `database.ts` | 1588 | `printSecondaryReplicationInfo` |  | `[]` | `['ReplSet']` |
| `database.ts` | 1669 | `getReplicationInfo` |  | `[]` | `['ReplSet']` |
| `database.ts` | 1734 | `printReplicationInfo` |  | `[]` | `['ReplSet']` |
| `database.ts` | 1766 | `printSlaveReplicationInfo` |  |  |  |
| `database.ts` | 1774 | `setSecondaryOk` |  |  |  |
| `database.ts` | 1782 | `watch` | `3.1.0` -> `ServerVersions.latest` | `[1]` | `['ReplSet', 'Sharded']` |
| `database.ts` | 1818 | `sql` | `4.4.0` -> `ServerVersions.latest` |  |  |
| `database.ts` | 1861 | `checkMetadataConsistency` | `7.0.0` -> `ServerVersions.latest` |  | `['Sharded']` |
| `collection.ts` | 201 | `aggregate` |  | `[1]` |  |
| `collection.ts` | 278 | `bulkWrite` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 318 | `count` |  | `[]` |  |
| `collection.ts` | 345 | `countDocuments` | `4.0.3` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 372 | `deleteMany` |  | `[1]` |  |
| `collection.ts` | 406 | `deleteOne` |  | `[1]` |  |
| `collection.ts` | 447 | `distinct` |  | `[]` |  |
| `collection.ts` | 476 | `estimatedDocumentCount` | `4.0.3` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 501 | `find` |  | `[1]` |  |
| `collection.ts` | 552 | `findAndModify` |  | `[1]` |  |
| `collection.ts` | 614 | `findOne` |  | `[1]` |  |
| `collection.ts` | 639 | `renameCollection` |  | `[1]` |  |
| `collection.ts` | 684 | `findOneAndDelete` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 721 | `findOneAndReplace` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 767 | `findOneAndUpdate` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 811 | `insert` |  | `[1]` |  |
| `collection.ts` | 855 | `insertMany` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 891 | `insertOne` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 915 | `isCapped` |  | `[1]` |  |
| `collection.ts` | 947 | `remove` |  | `[1]` |  |
| `collection.ts` | 990 | `replaceOne` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 1018 | `update` |  | `[1]` |  |
| `collection.ts` | 1075 | `updateMany` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 1118 | `updateOne` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 1152 | `compactStructuredEncryptionData` |  | `[]` |  |
| `collection.ts` | 1175 | `convertToCapped` |  | `[]` |  |
| `collection.ts` | 1237 | `createIndexes` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 1263 | `createIndex` |  | `[1]` |  |
| `collection.ts` | 1298 | `ensureIndex` |  | `[1]` |  |
| `collection.ts` | 1316 | `getIndexes` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 1334 | `getIndexSpecs` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 1351 | `getIndices` |  | `[1]` |  |
| `collection.ts` | 1368 | `getIndexKeys` | `3.2.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 1387 | `dropIndexes` |  | `[1]` |  |
| `collection.ts` | 1433 | `dropIndex` |  | `[1]` |  |
| `collection.ts` | 1468 | `totalIndexSize` |  | `[]` |  |
| `collection.ts` | 1489 | `reIndex` |  | `[]` | `['Standalone']` |
| `collection.ts` | 1502 | `getDB` |  |  |  |
| `collection.ts` | 1513 | `getMongo` |  |  |  |
| `collection.ts` | 1525 | `dataSize` |  | `[]` |  |
| `collection.ts` | 1538 | `storageSize` |  | `[]` |  |
| `collection.ts` | 1551 | `totalSize` |  | `[]` |  |
| `collection.ts` | 1564 | `drop` |  | `[1]` |  |
| `collection.ts` | 1625 | `exists` |  | `[1]` |  |
| `collection.ts` | 1650 | `runCommand` |  | `[1]` |  |
| `collection.ts` | 1688 | `explain` |  | `[1]` |  |
| `collection.ts` | 1971 | `stats` |  | `[]` |  |
| `collection.ts` | 2055 | `latencyStats` |  | `[]` |  |
| `collection.ts` | 2065 | `initializeOrderedBulkOp` |  | `[1]` |  |
| `collection.ts` | 2079 | `initializeUnorderedBulkOp` |  | `[1]` |  |
| `collection.ts` | 2092 | `getPlanCache` |  | `[]` |  |
| `collection.ts` | 2102 | `mapReduce` |  | `[]` |  |
| `collection.ts` | 2142 | `validate` |  | `[]` |  |
| `collection.ts` | 2156 | `getShardVersion` |  | `[]` | `['Sharded']` |
| `collection.ts` | 2219 | `getShardDistribution` |  | `[]` | `['Sharded']` |
| `collection.ts` | 2348 | `getShardLocation` | `8.0.10` -> `ServerVersions.latest` | `[]` | `['Sharded']` |
| `collection.ts` | 2393 | `watch` | `3.1.0` -> `ServerVersions.latest` | `[1]` | `['ReplSet', 'Sharded']` |
| `collection.ts` | 2446 | `hideIndex` | `4.4.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 2454 | `unhideIndex` | `4.4.0` -> `ServerVersions.latest` | `[1]` |  |
| `collection.ts` | 2463 | `analyzeShardKey` | `7.0.0` -> `ServerVersions.latest` | `[]` | `['ReplSet', 'Sharded']` |
| `collection.ts` | 2480 | `configureQueryAnalyzer` | `7.0.0` -> `ServerVersions.latest` | `[]` | `['ReplSet', 'Sharded']` |
| `collection.ts` | 2491 | `checkMetadataConsistency` | `7.0.0` -> `ServerVersions.latest` |  | `['Sharded']` |
| `collection.ts` | 2509 | `getSearchIndexes` | `6.0.0` -> `ServerVersions.latest` | `[]` |  |
| `collection.ts` | 2544 | `createSearchIndex` | `6.0.0` -> `ServerVersions.latest` | `[]` |  |
| `collection.ts` | 2596 | `createSearchIndexes` | `6.0.0` -> `ServerVersions.latest` | `[]` |  |
| `collection.ts` | 2614 | `dropSearchIndex` | `6.0.0` -> `ServerVersions.latest` | `[]` |  |
| `collection.ts` | 2627 | `updateSearchIndex` | `6.0.0` -> `ServerVersions.latest` | `[]` |  |
| `mongo.ts` | 383 | `getDBs` |  | `[1]` |  |
| `mongo.ts` | 398 | `bulkWrite` | `8.0.0` -> `ServerVersions.latest` | `[1]` |  |
| `mongo.ts` | 431 | `getDBNames` |  | `[1]` |  |
| `mongo.ts` | 442 | `show` |  | `[1]` |  |
| `mongo.ts` | 646 | `setReadPref` |  |  |  |
| `mongo.ts` | 662 | `setReadConcern` |  |  |  |
| `mongo.ts` | 675 | `setWriteConcern` |  |  |  |
| `mongo.ts` | 752 | `startSession` |  |  | `['ReplSet']` |
| `mongo.ts` | 821 | `setSlaveOk` |  |  |  |
| `mongo.ts` | 829 | `setSecondaryOk` |  |  |  |
| `mongo.ts` | 851 | `watch` | `3.1.0` -> `ServerVersions.latest` | `[1]` | `['ReplSet', 'Sharded']` |
| `mongo.ts` | 879 | `getClientEncryption` | `4.2.0` -> `ServerVersions.latest` |  |  |
| `mongo.ts` | 896 | `getKeyVault` | `4.2.0` -> `ServerVersions.latest` |  |  |
| `mongo.ts` | 905 | `convertShardKeyToHashed` |  |  |  |