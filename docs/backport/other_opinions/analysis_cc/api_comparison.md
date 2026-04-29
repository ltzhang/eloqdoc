# MongoDB API Evolution: Cross-Version Comparison (v4.0 → v8.0)

**Scope:** Data model, indexing, query, aggregation, encryption, and auth APIs only.  
Sharding, replication, and transaction internals excluded.

**Legend:** ✓ = supported | ✗ = not supported | ⚠ = deprecated / experimental | — = N/A

---

## 1. Index Types

| Index Type | Wire String | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|---|---|
| B-Tree (default) | `""` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 2D geospatial | `"2d"` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 2DSphere geospatial | `"2dsphere"` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Text full-text search | `"text"` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Hashed | `"hashed"` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| GeoHaystack | `"geoHaystack"` | ✓ | ✓ | ⚠depr | ✗removed | ✗ | ✗ | ✗ |
| Wildcard | `"wildcard"` | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| 2DSphere Bucket | `"2dsphere_bucket"` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| Column Store | `"columnstore"` | ✗ | ✗ | ✗ | ✗ | ⚠exp | ✓ | ⚠depr |
| Encrypted Range | `"queryable_encrypted_range"` | ✗ | ✗ | ✗ | ✗ | ✗ | ✓internal | ✓internal |

### Index Options Per Type

| Option | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|---|
| `unique` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `sparse` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `background` | ✓ | ✓ | ⚠ignored | ✗removed | ✗ | ✗ | ✗ |
| `expireAfterSeconds` (TTL) | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `partialFilterExpression` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `collation` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `wildcardProjection` | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Compound wildcard | ✗ | ✗ | ✗ | ✗ | ⚠exp | ✓ | ✓ |
| `commitQuorum` (createIndexes) | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ |

---

## 2. Collection Types

| Collection Type | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|---|
| Standard | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Capped | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| View | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Time-series | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| Clustered (by _id) | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| Encrypted (FLE2 fields) | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |

---

## 3. `create` Command Options

| Field | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|---|
| `capped` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `size` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `max` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `autoIndexId` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `idIndex` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `storageEngine` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `validator` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `validationLevel` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `validationAction` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `collation` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `viewOn` / `pipeline` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `indexOptionDefaults` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `timeseries` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `expireAfterSeconds` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `clusteredIndex` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `changeStreamPreAndPostImages` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `encryptedFields` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `recordIdsReplicated` | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ |

---

## 4. Aggregation Pipeline Stages

### Core Stages (stable across all versions)
| Stage | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|---|
| `$match` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$project` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$addFields` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$group` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$sort` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$limit` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$skip` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$unwind` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$lookup` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$graphLookup` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$out` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$replaceRoot` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$redact` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$sample` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$count` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$sortByCount` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$bucket` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$bucketAuto` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$facet` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$geoNear` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$indexStats` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$collStats` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$currentOp` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$changeStream` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |

### Stages Added After 4.0

| Stage | Introduced In | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|---|---|
| `$merge` | 4.2 | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$planCacheStats` | 4.2 | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$unionWith` | 4.4 | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `$setWindowFields` | 5.0 | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `$densify` | 5.3/6.0 | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `$fill` | 5.3/6.0 | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `$listCatalog` | 6.0 | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `$queryStats` | 6.0 | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `$shardedDataDistribution` | 6.0 | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `$changeStreamSplitLargeEvent` | 6.0 | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `$listSampledQueries` | 7.0 | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ |
| `$vectorSearch` | 8.0 | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ |
| `$rankFusion` | 8.0 | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ |
| `$scoreFusion` | 8.0 | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ |
| `$listSearchIndexes` | 8.0 | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ |
| `$querySettings` | 8.0 | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ |
| `$search` (Atlas Search) | 8.0 | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ |
| `$searchMeta` | 8.0 | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ |
| `$listClusterCatalog` | 8.0 | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ |

---

## 5. `$group` Accumulators

| Accumulator | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 | Notes |
|---|---|---|---|---|---|---|---|---|
| `$sum` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| `$avg` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| `$min` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| `$max` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| `$first` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | As expression: 4.4+ |
| `$last` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | As expression: 4.4+ |
| `$push` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| `$addToSet` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| `$stdDevPop` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| `$stdDevSamp` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| `$mergeObjects` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | Added in 3.6 |
| `$accumulator` (custom JS) | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ | Requires JS |
| `$count` (in group) | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ | — |
| `$minN` / `$maxN` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | — |
| `$firstN` / `$lastN` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | — |
| `$top` / `$bottom` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | — |
| `$topN` / `$bottomN` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | — |
| `$percentile` | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | Approximate |
| `$median` | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | Approximate |

### Window-Specific Operators (`$setWindowFields` only)

| Operator | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|---|
| `$rank` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `$denseRank` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `$documentNumber` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `$expMovingAvg` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `$covariancePop` / `$covarianceSamp` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `$integral` / `$derivative` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `$shift` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `$locf` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `$percentile` (window) | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ |
| `$median` (window) | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ |

---

## 6. Expression Functions

| Expression | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 | Notes |
|---|---|---|---|---|---|---|---|---|
| All arithmetic (`$add`, `$subtract`, etc.) | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| All string (`$concat`, `$toLower`, etc.) | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| All date (`$dateToString`, etc.) | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| Trigonometric (`$sin`, `$cos`, etc.) | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | 14 trig fns |
| `$round` / `$trunc` | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| `$function` (custom JS) | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ | Requires JS |
| `$first` / `$last` as array expr | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| `$getField` / `$setField` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ | — |

---

## 7. Update Command Features

| Feature | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|---|
| Standard update operators | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `arrayFilters` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Aggregation pipeline form (`u: [...]`) | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Update time-series `metaField` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| Upsert into time-series | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ |
| `bulkWrite` (cross-collection) | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ |

---

## 8. Security and Encryption

| Feature | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|---|
| SCRAM-SHA-1 | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| SCRAM-SHA-256 | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| MONGODB-X509 | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| GSSAPI/Kerberos | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| LDAP (PLAIN) | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| MONGODB-OIDC | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ |
| CSFLE (client-side encryption) | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| FLE2 equality | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| FLE2 range | ✗ | ✗ | ✗ | ✗ | ⚠preview | ✓ | ✓ |
| Encryption at rest (WiredTiger) | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| TLS certificate rotation | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ | ✓ |
| JSON Schema validation | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |

---

## 9. Commands Removed (from v4.0 baseline)

| Command | Removed In | Replacement |
|---|---|---|
| `eval` | 4.2 | None (server-side JS removed for security) |
| `clone` | 4.2 | `mongodump` + `mongorestore` |
| `copydb` | 4.2 | Same |
| `geoNear` (cmd) | 4.0 (formally) | `$geoNear` aggregation stage |
| `group` | 4.2 | `$group` aggregation stage |
| `parallelCollectionScan` | 4.2 | Normal `find` / `aggregate` |
| `do_txn` (internal) | 4.2 | New transaction internals |
| `reIndex` | 6.0 | `dropIndexes` + `createIndexes` |
| `getLastError` | 7.0 | Write concern on the operation |

---

## 10. Commands Added Post-4.0 (Data Model Focus)

| Command | Added In | Description |
|---|---|---|
| `dropConnections` | 4.2 | Force-close connections |
| `voteCommitIndexBuild` | 4.2 | Index build coordination |
| `setIndexCommitQuorum` | 4.4 | Change quorum on in-progress build |
| `rotateCertificates` | 4.4 | Live TLS cert reload |
| `validateDBMetadata` | 5.0 | Validate metadata vs API version |
| `compactStructuredEncryptionData` | 6.0 | FLE2 state compaction |
| `getClusterParameter` | 6.0 | Read cluster-wide parameters |
| `setClusterParameter` | 6.0 | Write cluster-wide parameters |
| `setUserWriteBlockMode` | 6.0 | Block all user writes |
| `getFLECount` | 7.0 | FLE2 state collection counts |
| `bulkWrite` | 7.0 | Cross-collection batch operations |
| `analyzeShardKey` | 7.0 | Collection field distribution |
| `setChangeStreamState` | 7.0 | Enable/disable change streams |
| `createSearchIndex` | 8.0 | Create Atlas/vector search index |
| `dropSearchIndex` | 8.0 | Drop search index |
| `updateSearchIndex` | 8.0 | Update search index definition |
| `setQuerySettings` | 8.0 | Persistent query plan hints |
| `removeQuerySettings` | 8.0 | Remove query plan hints |
| `cleanupStructuredEncryptionData` | 8.0 | FLE2 state cleanup |
| `autoCompact` | 8.0 | Enable auto-compaction |

---

## 11. Versioned API

| Feature | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|---|
| `apiVersion` request field | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `apiStrict` request field | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |
| `apiDeprecationErrors` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ | ✓ |

---

## 12. Change Streams Evolution

| Feature | 4.0 | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|---|
| Single collection watch | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Database-level watch | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Deployment-level watch | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `fullDocument: updateLookup` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `fullDocument: required` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `fullDocumentBeforeChange` (pre-image) | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| DDL events (`showExpandedEvents`) | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `$changeStreamSplitLargeEvent` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |

---

## 13. Time-Series Collections Evolution

| Feature | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|
| Basic insert | ✓ | ✓ | ✓ | ✓ |
| `timeField`, `metaField` | ✓ | ✓ | ✓ | ✓ |
| Granularity (`seconds/minutes/hours`) | ✓ | ✓ | ✓ | ✓ |
| TTL (`expireAfterSeconds`) | ✓ | ✓ | ✓ | ✓ |
| Secondary indexes | ✗ | ✓ | ✓ | ✓ |
| `2dsphere` geospatial index | ✗ | ✓ | ✓ | ✓ |
| Updates (metaField) | ✗ | ✓ | ✓ | ✓ |
| Deletes | ✗ | ✓ | ✓ | ✓ |
| Upserts | ✗ | ✗ | ✗ | ✓ |
| Sharding | ✗ | ✗ | ✓ | ✓ |
| Extended timestamp range | ✗ | ✓ | ✓ | ✓ |
| `bucketMaxSpanSeconds` | ✓ | ✓ | ✓ | ✓ |

---

## 14. FLE / Encryption Evolution

| Feature | 4.2 | 4.4 | 5.0 | 6.0 | 7.0 | 8.0 |
|---|---|---|---|---|---|---|
| CSFLE schema (`encrypt` in $jsonSchema) | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Encrypted BSON BinData subtype 6 | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| FLE2 equality on encrypted fields | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| FLE2 range (preview) | ✗ | ✗ | ✗ | ✓ | — | — |
| FLE2 range (stable) | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ |
| `encryptedFields` in `create` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `compactStructuredEncryptionData` | ✗ | ✗ | ✗ | ✓ | ✓ | ✓ |
| `getFLECount` | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ |
| `cleanupStructuredEncryptionData` | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ |
| `INDEX_ENCRYPTED_RANGE` index type | ✗ | ✗ | ✗ | ✗ | ✓ | ✓ |
