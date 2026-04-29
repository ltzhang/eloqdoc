# mongosh Shell API Signatures

- Total public methods captured: `93`

| File | Line | Method | Signature | Decorators |
|---|---:|---|---|---|
| `database.ts` | 99 | `constructor` | `constructor(mongo: Mongo<M>, name: StringKey<M>, session?: Session)` |  |
| `database.ts` | 364 | `getMongo` | `getMongo(): Mongo<M>` | @returnType('Mongo') |
| `database.ts` | 368 | `getName` | `getName(): StringKey<M>` |  |
| `database.ts` | 379 | `getCollectionNames` | `async getCollectionNames(): Promise<StringKey<D>[]>` | @returnsPromise, @apiVersions([1]) |
| `database.ts` | 445 | `adminCommand` | `async adminCommand(cmd: string \| Document): Promise<Document>` | @returnsPromise, @serverVersions(['3.4.0', ServerVersions.latest]), @apiVersions([1]) |
| `database.ts` | 478 | `aggregate` | `async aggregate(...args: unknown[]): Promise<AggregationCursor \| Document>` | @returnsPromise, @returnType('AggregationCursor'), @apiVersions([1]) |
| `database.ts` | 576 | `dropDatabase` | `async dropDatabase(writeConcern?: WriteConcern): Promise<Document>` | @returnsPromise, @apiVersions([1]) |
| `database.ts` | 710 | `logout` | `async logout(): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 733 | `dropAllUsers` | `async dropAllUsers(writeConcern?: WriteConcern): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 855 | `getUsers` | `async getUsers(options: Document = {}): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 994 | `dropAllRoles` | `async dropAllRoles(writeConcern?: WriteConcern): Promise<Document>` | @returnsPromise |
| `database.ts` | 1117 | `getRoles` | `async getRoles(options: Document = {}): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1178 | `currentOp` | `async currentOp(opts: Document \| boolean = {}): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1189 | `killOp` | `async killOp(opId: number \| string): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1199 | `shutdownServer` | `async shutdownServer(opts: Document = {}): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1209 | `fsyncLock` | `async fsyncLock(): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1219 | `fsyncUnlock` | `async fsyncUnlock(): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1228 | `version` | `async version(): Promise<string>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1246 | `serverBits` | `async serverBits(): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1264 | `isMaster` | `async isMaster(): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1276 | `hello` | `async hello(): Promise<Document>` | @returnsPromise, @apiVersions([1]), @serverVersions(['5.0.0', ServerVersions.latest]) |
| `database.ts` | 1296 | `serverBuildInfo` | `async serverBuildInfo(): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1305 | `serverStatus` | `async serverStatus(opts = {}): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1315 | `stats` | `async stats(scaleOrOptions: number \| Document = 1): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1334 | `hostInfo` | `async hostInfo(): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1343 | `serverCmdLineOpts` | `async serverCmdLineOpts(): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1353 | `rotateCertificates` | `async rotateCertificates(message?: string): Promise<Document>` | @returnsPromise, @serverVersions(['5.0.0', ServerVersions.latest]), @apiVersions([]) |
| `database.ts` | 1363 | `printCollectionStats` | `async printCollectionStats(scale = 1): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1385 | `getProfilingStatus` | `async getProfilingStatus(): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1450 | `getLogComponents` | `async getLogComponents(): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1465 | `cloneDatabase` | `cloneDatabase(): void` | @deprecated |
| `database.ts` | 1472 | `cloneCollection` | `cloneCollection(): void` | @deprecated |
| `database.ts` | 1479 | `copyDatabase` | `copyDatabase(): void` | @deprecated |
| `database.ts` | 1487 | `commandHelp` | `async commandHelp(name: string): Promise<Document>` | @returnsPromise, @apiVersions([1]) |
| `database.ts` | 1508 | `listCommands` | `async listCommands(): Promise<CommandResult>` | @returnsPromise, @apiVersions([]) |
| `database.ts` | 1576 | `printShardingStatus` | `async printShardingStatus(verbose = false): Promise<CommandResult>` | @returnsPromise, @topologies(['Sharded']), @apiVersions([1]) |
| `database.ts` | 1588 | `printSecondaryReplicationInfo` | `async printSecondaryReplicationInfo(): Promise<CommandResult>` | @returnsPromise, @topologies(['ReplSet']), @apiVersions([]) |
| `database.ts` | 1669 | `getReplicationInfo` | `async getReplicationInfo(): Promise<Document>` | @returnsPromise, @topologies(['ReplSet']), @apiVersions([]) |
| `database.ts` | 1734 | `printReplicationInfo` | `async printReplicationInfo(): Promise<CommandResult>` | @returnsPromise, @apiVersions([]), @topologies(['ReplSet']) |
| `database.ts` | 1766 | `printSlaveReplicationInfo` | `printSlaveReplicationInfo(): never` | @deprecated |
| `database.ts` | 1774 | `setSecondaryOk` | `async setSecondaryOk(): Promise<void>` | @deprecated, @returnsPromise |
| `collection.ts` | 201 | `aggregate` | `async aggregate(...args: unknown[]): Promise<AggregationCursor \| Document>` | @returnsPromise, @returnType('AggregationCursor'), @apiVersions([1]) |
| `collection.ts` | 318 | `count` | `async count(query = {}, options: CountOptions = {}): Promise<number>` | @returnsPromise, @deprecated, @serverVersions([ServerVersions.earliest, '4.0.0']), @apiVersions([]) |
| `collection.ts` | 915 | `isCapped` | `async isCapped(): Promise<boolean>` | @returnsPromise, @apiVersions([1]) |
| `collection.ts` | 1152 | `compactStructuredEncryptionData` | `async compactStructuredEncryptionData(): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `collection.ts` | 1175 | `convertToCapped` | `async convertToCapped(size: number): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `collection.ts` | 1316 | `getIndexes` | `async getIndexes(): Promise<Document[]>` | @returnsPromise, @serverVersions(['3.2.0', ServerVersions.latest]), @apiVersions([1]) |
| `collection.ts` | 1334 | `getIndexSpecs` | `async getIndexSpecs(): Promise<Document[]>` | @returnsPromise, @serverVersions(['3.2.0', ServerVersions.latest]), @apiVersions([1]) |
| `collection.ts` | 1351 | `getIndices` | `async getIndices(): Promise<Document[]>` | @returnsPromise, @apiVersions([1]) |
| `collection.ts` | 1368 | `getIndexKeys` | `async getIndexKeys(): Promise<Document[]>` | @returnsPromise, @serverVersions(['3.2.0', ServerVersions.latest]), @apiVersions([1]) |
| `collection.ts` | 1433 | `dropIndex` | `async dropIndex(index: string \| Document): Promise<Document>` | @returnsPromise, @apiVersions([1]) |
| `collection.ts` | 1468 | `totalIndexSize` | `async totalIndexSize(...args: any[]): Promise<number>` | @returnsPromise, @apiVersions([]) |
| `collection.ts` | 1489 | `reIndex` | `async reIndex(): Promise<Document>` | @returnsPromise, @deprecated, @topologies(['Standalone']), @apiVersions([]) |
| `collection.ts` | 1502 | `getDB` | `getDB(): DatabaseWithSchema<M, D>` | @returnType('DatabaseWithSchema') |
| `collection.ts` | 1513 | `getMongo` | `getMongo(): Mongo<M>` | @returnType('Mongo') |
| `collection.ts` | 1525 | `dataSize` | `async dataSize(): Promise<number>` | @returnsPromise, @apiVersions([]) |
| `collection.ts` | 1538 | `storageSize` | `async storageSize(): Promise<number>` | @returnsPromise, @apiVersions([]) |
| `collection.ts` | 1551 | `totalSize` | `async totalSize(): Promise<number>` | @returnsPromise, @apiVersions([]) |
| `collection.ts` | 1564 | `drop` | `async drop(options: DropCollectionOptions = {}): Promise<boolean>` | @returnsPromise, @apiVersions([1]) |
| `collection.ts` | 1625 | `exists` | `async exists(): Promise<Document>` | @returnsPromise, @apiVersions([1]) |
| `collection.ts` | 1638 | `getFullName` | `getFullName(): string` |  |
| `collection.ts` | 1643 | `getName` | `getName(): N` |  |
| `collection.ts` | 1688 | `explain` | `explain(verbosity: ExplainVerbosityLike = 'queryPlanner'): Explainable` | @returnType('Explainable'), @apiVersions([1]) |
| `collection.ts` | 1971 | `stats` | `async stats(originalOptions: Document \| number = {}): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `collection.ts` | 2055 | `latencyStats` | `async latencyStats(options: Document = {}): Promise<Document[]>` | @returnsPromise, @apiVersions([]) |
| `collection.ts` | 2065 | `initializeOrderedBulkOp` | `async initializeOrderedBulkOp(): Promise<Bulk>` | @returnsPromise, @returnType('Bulk'), @apiVersions([1]) |
| `collection.ts` | 2079 | `initializeUnorderedBulkOp` | `async initializeUnorderedBulkOp(): Promise<Bulk>` | @returnsPromise, @returnType('Bulk'), @apiVersions([1]) |
| `collection.ts` | 2092 | `getPlanCache` | `getPlanCache(): PlanCache` | @returnType('PlanCache'), @apiVersions([]) |
| `collection.ts` | 2142 | `validate` | `async validate(options: boolean \| Document = false): Promise<Document>` | @returnsPromise, @apiVersions([]) |
| `collection.ts` | 2156 | `getShardVersion` | `async getShardVersion(): Promise<Document>` | @returnsPromise, @topologies(['Sharded']), @apiVersions([]) |
| `collection.ts` | 2348 | `getShardLocation` | `async getShardLocation(): Promise<` | @returnsPromise, @topologies(['Sharded']), @apiVersions([]), @serverVersions(['8.0.10', ServerVersions.latest]) |
| `collection.ts` | 2446 | `hideIndex` | `async hideIndex(index: string \| Document): Promise<Document>` | @serverVersions(['4.4.0', ServerVersions.latest]), @returnsPromise, @apiVersions([1]) |
| `collection.ts` | 2454 | `unhideIndex` | `async unhideIndex(index: string \| Document): Promise<Document>` | @serverVersions(['4.4.0', ServerVersions.latest]), @returnsPromise, @apiVersions([1]) |
| `collection.ts` | 2480 | `configureQueryAnalyzer` | `async configureQueryAnalyzer(options: Document): Promise<Document>` | @serverVersions(['7.0.0', ServerVersions.latest]), @returnsPromise, @topologies(['ReplSet', 'Sharded']), @apiVersions([]) |
| `collection.ts` | 2614 | `dropSearchIndex` | `async dropSearchIndex(indexName: string): Promise<void>` | @serverVersions(['6.0.0', ServerVersions.latest]), @returnsPromise, @apiVersions([]) |
| `mongo.ts` | 221 | `connect` | `async connect(username?: string, password?: string): Promise<void>` |  |
| `mongo.ts` | 311 | `getURI` | `getURI(): string` |  |
| `mongo.ts` | 315 | `use` | `use(db: StringKey<M>): string` |  |
| `mongo.ts` | 383 | `getDBs` | `async getDBs(options: ListDatabasesOptions = {}): Promise<` | @returnsPromise, @apiVersions([1]) |
| `mongo.ts` | 589 | `close` | `async close(): Promise<void>` |  |
| `mongo.ts` | 608 | `getReadPrefMode` | `getReadPrefMode(): ReadPreferenceMode` |  |
| `mongo.ts` | 612 | `getReadPrefTagSet` | `getReadPrefTagSet(): Record<string, string>[] \| undefined` |  |
| `mongo.ts` | 616 | `getReadPref` | `getReadPref(): ReadPreference` |  |
| `mongo.ts` | 628 | `getReadConcern` | `getReadConcern(): string \| undefined` |  |
| `mongo.ts` | 637 | `getWriteConcern` | `getWriteConcern(): WriteConcern \| undefined` |  |
| `mongo.ts` | 662 | `setReadConcern` | `async setReadConcern(level: ReadConcernLevel): Promise<void>` | @returnsPromise |
| `mongo.ts` | 752 | `startSession` | `startSession(options: Document = {}): Session` | @topologies(['ReplSet']) |
| `mongo.ts` | 804 | `setCausalConsistency` | `setCausalConsistency(): void` |  |
| `mongo.ts` | 812 | `isCausalConsistency` | `isCausalConsistency(): void` |  |
| `mongo.ts` | 821 | `setSlaveOk` | `setSlaveOk(): void` | @deprecated |
| `mongo.ts` | 829 | `setSecondaryOk` | `async setSecondaryOk(): Promise<void>` | @deprecated, @returnsPromise |
| `mongo.ts` | 879 | `getClientEncryption` | `getClientEncryption(): ClientEncryption` | @platforms(['CLI']), @serverVersions(['4.2.0', ServerVersions.latest]), @returnType('ClientEncryption') |
| `mongo.ts` | 896 | `getKeyVault` | `async getKeyVault(): Promise<KeyVault>` | @platforms(['CLI']), @serverVersions(['4.2.0', ServerVersions.latest]), @returnType('KeyVault'), @returnsPromise |
