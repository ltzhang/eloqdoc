# MongoDB 4.0 → 8.0 Data API Evolution (Gap Table)

This is a **gap-focused** delta. It lists features added in 5.0 / 6.0 / 7.0 / 8.0 that EloqDoc 4.0.3 does NOT yet have. Features already present in EloqDoc are not listed here — see [`baseline.md`](./baseline.md) for those.

Every row links to a task spec under `tasks/` or to [`tier-4-deferred.md`](./tier-4-deferred.md) for explicitly out-of-scope items.

**Tier legend:** **1** = Simple · **2** = Needs Effort & Design · **3** = Hard w/o Significant Refactor · **4** = Deferred

---

## Commands

| Feature | Version | Tier | Spec |
| ------- | ------- | ---- | ---- |
| Stable API envelope (`apiVersion`, `apiStrict`, `apiDeprecationErrors`) | 5.0 | 1 | [`stable-api-envelope.md`](./tasks/stable-api-envelope.md) |
| `hello` handshake (alias of `isMaster`) | 5.0 | 1 | [`hello-command.md`](./tasks/hello-command.md) |
| `rotateCertificates` | 4.4/5.0 | 1 | [`rotate-certificates.md`](./tasks/rotate-certificates.md) |
| Common command field passthrough (`comment`, `collectionUUID`-as-ignored) | 5.0+ | 1 | [`common-command-fields.md`](./tasks/common-command-fields.md) |
| `commitQuorum` parsing on `createIndexes` (parser-only / primary-only) | 4.4/5.0 | 1→2 | [`commitquorum-parsing.md`](./tasks/commitquorum-parsing.md) |
| `bulkWrite` command (cross-namespace writes) | 7.0 | 2 | [`bulkwrite-command.md`](./tasks/bulkwrite-command.md) |
| `bulkWrite` enhancements (`let`, `errorsOnly`) | 8.0 | 2 | [`bulkwrite-command.md`](./tasks/bulkwrite-command.md) |
| `analyze` command (collection statistics / histograms) | 7.0 | 2 | [`analyze-command.md`](./tasks/analyze-command.md) |
| `setQuerySettings` / `removeQuerySettings` | 8.0 | 3 | [`query-settings.md`](./tasks/query-settings.md) |
| `compactStructuredEncryptionData` (FLE2) | 6.0/8.0 | 3 | [`queryable-encryption-fle2.md`](./tasks/queryable-encryption-fle2.md) |
| `cleanupStructuredEncryptionData` (FLE2) | 8.0 | 3 | [`queryable-encryption-fle2.md`](./tasks/queryable-encryption-fle2.md) |
| `getQueryableEncryptionCountInfo` | 7.0 | 3 | [`queryable-encryption-fle2.md`](./tasks/queryable-encryption-fle2.md) |
| Search index commands (`createSearchIndexes`, `dropSearchIndex`, `updateSearchIndex`, `listSearchIndexes`) | 8.0 | 3 | [`search-vector-search.md`](./tasks/search-vector-search.md) |
| Replication coordination commands (`voteCommitIndexBuild`, `setIndexCommitQuorum`, etc.) | various | 4 | [`tier-4-deferred.md`](./tier-4-deferred.md) |

## Aggregation stages

| Feature | Version | Tier | Spec |
| ------- | ------- | ---- | ---- |
| `$set` (alias of `$addFields`) | 5.0 | 1 | [`stage-aliases-set-unset-replacewith.md`](./tasks/stage-aliases-set-unset-replacewith.md) |
| `$unset` (alias for projection) | 5.0 | 1 | [`stage-aliases-set-unset-replacewith.md`](./tasks/stage-aliases-set-unset-replacewith.md) |
| `$replaceWith` (alias of `$replaceRoot`) | 5.0 | 1 | [`stage-aliases-set-unset-replacewith.md`](./tasks/stage-aliases-set-unset-replacewith.md) |
| `$documents` (literal source) | 6.0 | 1 | [`document-source-documents.md`](./tasks/document-source-documents.md) |
| `$planCacheStats` | 4.2/5.0 | 1 | [`document-source-plan-cache-stats.md`](./tasks/document-source-plan-cache-stats.md) |
| `$unionWith` (nested pipeline union) | 4.4/5.0 | 2 | [`document-source-unionwith.md`](./tasks/document-source-unionwith.md) |
| `$merge` (write from pipeline) | 4.2/5.0 | 2 | [`document-source-merge.md`](./tasks/document-source-merge.md) |
| `$fill` (forward-fill / linear interp) | 6.0 | 2 | [`document-source-fill.md`](./tasks/document-source-fill.md) |
| `$densify` (gap-fill date/numeric) | 6.0 | 2 | [`document-source-densify.md`](./tasks/document-source-densify.md) |
| `$setWindowFields` (window functions) | 5.0/7.0 | 2 | [`setwindowfields.md`](./tasks/setwindowfields.md) |
| `$listCatalog` | 6.0 | 2 | [`document-source-listcatalog.md`](./tasks/document-source-listcatalog.md) |
| `$queryStats` | 6.0 | 3 | [`query-settings.md`](./tasks/query-settings.md) (subsystem dep) |
| `$listSampledQueries` | 7.0 | 4 | [`tier-4-deferred.md`](./tier-4-deferred.md) |
| `$search`, `$searchBeta`, `$searchMeta` | 8.0 | 3 | [`search-vector-search.md`](./tasks/search-vector-search.md) |
| `$vectorSearch` | 8.0 | 3 | [`search-vector-search.md`](./tasks/search-vector-search.md) |
| `$rankFusion`, `$scoreFusion` | 8.0 | 3 | [`search-vector-search.md`](./tasks/search-vector-search.md) |
| `$listSearchIndexes` (stage form) | 8.0 | 3 | [`search-vector-search.md`](./tasks/search-vector-search.md) |
| Internal time-series stages (`$_internalUnpackBucket`, etc.) | 5.0+ | 4 | [`tier-4-deferred.md`](./tier-4-deferred.md) (covered by time-series spec) |

## Aggregation expressions

| Feature | Version | Tier | Spec |
| ------- | ------- | ---- | ---- |
| Trig functions (`$sin`, `$cos`, `$tan`, `$asin`, `$acos`, `$atan`, `$atan2`, `$sinh`, `$cosh`, `$tanh`, `$asinh`, `$acosh`, `$atanh`, `$degrees`, `$radians`) | 5.0 | 1 | [`pure-expressions-batch.md`](./tasks/pure-expressions-batch.md) |
| Regex expressions (`$regexFind`, `$regexMatch`, `$regexFindAll`, `$replaceOne`, `$replaceAll`) | 5.0 | 1 | [`pure-expressions-batch.md`](./tasks/pure-expressions-batch.md) |
| String trim (`$trim`, `$ltrim`, `$rtrim`) | 5.0 | 1 | [`pure-expressions-batch.md`](./tasks/pure-expressions-batch.md) |
| `$round`, `$trunc` (rounding flavor) | 5.0 | 1 | [`pure-expressions-batch.md`](./tasks/pure-expressions-batch.md) |
| Conversions (`$convert`, `$toBool`, `$toDate`, `$toString`, `$toObjectId`, `$toInt`, `$toLong`, `$toDecimal`, `$toDouble`) | 5.0 | 1 | [`pure-expressions-batch.md`](./tasks/pure-expressions-batch.md) |
| `$function` (custom JS expression) | 4.4/5.0 | 1 | [`pure-expressions-batch.md`](./tasks/pure-expressions-batch.md) |
| `$rand`, `$binarySize`, `$bsonSize`, `$toHashedIndexKey` | 5.0 | 1 | [`pure-expressions-batch.md`](./tasks/pure-expressions-batch.md) |
| Date arithmetic (`$dateAdd`, `$dateDiff`, `$dateSubtract`, `$dateTrunc`) | 6.0 | 1 | [`date-arithmetic-expressions.md`](./tasks/date-arithmetic-expressions.md) |
| Field access (`$getField`, `$setField`, `$unsetField`) | 7.0 | 1 | [`getfield-setfield-unsetfield.md`](./tasks/getfield-setfield-unsetfield.md) |
| `$sortArray` | 7.0 | 1 | [`sortarray-firstn-lastn-expressions.md`](./tasks/sortarray-firstn-lastn-expressions.md) |
| N-value expressions (`$firstN`, `$lastN`, `$minN`, `$maxN` as expressions) | 7.0 | 1 | [`sortarray-firstn-lastn-expressions.md`](./tasks/sortarray-firstn-lastn-expressions.md) |
| Bit ops (`$bitAnd`, `$bitOr`, `$bitXor`, `$bitNot`) | 8.0 | 1 | [`pure-expressions-batch.md`](./tasks/pure-expressions-batch.md) |
| `$median`, `$percentile` (as expressions) | 8.0 | 2 | [`percentile-median-accumulators.md`](./tasks/percentile-median-accumulators.md) |

## Accumulators

| Feature | Version | Tier | Spec |
| ------- | ------- | ---- | ---- |
| `$count` (group-context) | 5.0 | 1 | [`accumulator-count.md`](./tasks/accumulator-count.md) |
| `$accumulator` (custom JS) | 4.4/5.0 | 1 | [`pure-expressions-batch.md`](./tasks/pure-expressions-batch.md) |
| N-value family (`$minN`, `$maxN`, `$firstN`, `$lastN`, `$top`, `$topN`, `$bottom`, `$bottomN`) | 5.0/6.0 | 2 | [`accumulator-n-value-family.md`](./tasks/accumulator-n-value-family.md) |
| `$percentile`, `$median` (t-digest) | 7.0 | 2 | [`percentile-median-accumulators.md`](./tasks/percentile-median-accumulators.md) |
| Window operators (`$rank`, `$denseRank`, `$documentNumber`, `$expMovingAvg`, `$covariancePop`, `$covarianceSamp`, `$integral`, `$derivative`, `$shift`, `$locf`) | 5.0/6.0 | 2 | [`window-operators.md`](./tasks/window-operators.md) |

## Index types & options

| Feature | Version | Tier | Spec |
| ------- | ------- | ---- | ---- |
| Hidden indexes (`hidden: true`) | 5.0 | 2 | [`hidden-indexes.md`](./tasks/hidden-indexes.md) |
| Wildcard indexes (`{ "$**": 1 }` + `wildcardProjection`) | 4.2 catch-up | 2 | [`wildcard-indexes.md`](./tasks/wildcard-indexes.md) |
| `commitQuorum` option (parser stub) | 4.4/5.0 | 1 | [`commitquorum-parsing.md`](./tasks/commitquorum-parsing.md) |
| `returnOnStart` two-phase build | 7.0 | 4 | [`tier-4-deferred.md`](./tier-4-deferred.md) (replication-coupled) |
| `prepareUnique` workflow | 8.0 | 2 | [`prepareunique-workflow.md`](./tasks/prepareunique-workflow.md) |
| Compound wildcard indexes | 6.0/7.0 | 3 | [`compound-wildcard-indexes.md`](./tasks/compound-wildcard-indexes.md) |
| Columnstore indexes | 6.0 (deprecated 8.0) | 4 | [`tier-4-deferred.md`](./tier-4-deferred.md) |
| 2DSphere bucket indexes (internal to TS) | 6.0 | 4 | covered by [`time-series-collections.md`](./tasks/time-series-collections.md) |
| `clustered: true` index option | 8.0 | 3 | [`clustered-collections.md`](./tasks/clustered-collections.md) |

## Collection options

| Feature | Version | Tier | Spec |
| ------- | ------- | ---- | ---- |
| `timeseries: {timeField, metaField, granularity}` | 5.0 | 2/3 | [`time-series-collections.md`](./tasks/time-series-collections.md) |
| `clusteredIndex` option | 5.0/6.0 | 3 | [`clustered-collections.md`](./tasks/clustered-collections.md) |
| `recordPreImages` (5.0) / `changeStreamPreAndPostImages` (6.0) | 5.0/6.0 | 2 | [`pre-post-images-collection-option.md`](./tasks/pre-post-images-collection-option.md) |
| `encryptedFields` (FLE2 metadata) | 6.0 | 3 | [`queryable-encryption-fle2.md`](./tasks/queryable-encryption-fle2.md) |
| `recordIdsReplicated` | 8.0 | 4 | [`tier-4-deferred.md`](./tier-4-deferred.md) (replication-coupled) |

## Query / write command flags

| Feature | Version | Tier | Spec |
| ------- | ------- | ---- | ---- |
| `find.let`, `update.let`, `delete.let`, `aggregate.let` | 5.0 | 2 | [`let-runtime-constants.md`](./tasks/let-runtime-constants.md) |
| `runtimeConstants` (find/update/delete) | 5.0 | 2 | [`let-runtime-constants.md`](./tasks/let-runtime-constants.md) |
| `update.hint`, `delete.hint` | 5.0 | 2 | [`update-delete-hint.md`](./tasks/update-delete-hint.md) |
| `find.allowDiskUse` | 5.0 | 2 | [`allowdiskuse-find.md`](./tasks/allowdiskuse-find.md) |
| `distinct.hint` | 8.0 | 1 | [`distinct-hint.md`](./tasks/distinct-hint.md) |
| Pipeline-form update (`update.u: [{$set:…}]`) | 4.2/5.0 | 2 | [`pipeline-update-syntax.md`](./tasks/pipeline-update-syntax.md) |
| `find/aggregate/distinct.querySettings` | 8.0 | 3 | [`query-settings.md`](./tasks/query-settings.md) |
| `sampleId`, `mirrored`, query-analyzer flags | 7.0 | 4 | [`tier-4-deferred.md`](./tier-4-deferred.md) |
| `isTimeseriesNamespace` internal flag | 5.0+ | 4 | covered by [`time-series-collections.md`](./tasks/time-series-collections.md) |
| `encryptionInformation` field | 6.0+ | 3 | [`queryable-encryption-fle2.md`](./tasks/queryable-encryption-fle2.md) |

## Security / authentication

| Feature | Version | Tier | Spec |
| ------- | ------- | ---- | ---- |
| Stable API enforcement (full strict mode) | 5.0 | 3 | [`stable-api-envelope.md`](./tasks/stable-api-envelope.md) (escalation path) |
| OIDC authentication (`MONGODB-OIDC`) | 7.0 | 3 | [`oidc-auth.md`](./tasks/oidc-auth.md) |
| FLE2 / Queryable Encryption (full crypto subsystem) | 6.0 | 3 | [`queryable-encryption-fle2.md`](./tasks/queryable-encryption-fle2.md) |

## Mongosh-visible client surface

| Feature | Version | Tier | Spec |
| ------- | ------- | ---- | ---- |
| `db.hello()` shell helper | 5.0 | 1 | covered by [`hello-command.md`](./tasks/hello-command.md) |
| `db.collection.hideIndex()`, `unhideIndex()` | 5.0 | 1 | covered by [`hidden-indexes.md`](./tasks/hidden-indexes.md) |
| `db.rotateCertificates()` | 5.0 | 1 | covered by [`rotate-certificates.md`](./tasks/rotate-certificates.md) |
| `db.getMongo().bulkWrite()` (client-level) | 8.0 | 2 | covered by [`bulkwrite-command.md`](./tasks/bulkwrite-command.md) |
| `db.collection.bulkWrite()` (driver-side fallback to legacy writes) | 3.2+ | — | already works via driver; no server change needed |
| Search index helpers | 6.0+ | 3 | covered by [`search-vector-search.md`](./tasks/search-vector-search.md) |
| `db.createEncryptedCollection()` | 6.0+ | 3 | covered by [`queryable-encryption-fle2.md`](./tasks/queryable-encryption-fle2.md) |

---

## Coverage check

- Total rows: ~80
- Tier 1 entries: ~30 (some grouped into batch task files)
- Tier 2 entries: ~25
- Tier 3 entries: ~15
- Tier 4 (deferred) entries: ~10

Every row points to a task file in `tasks/`, the deferred doc, or to a grouped task file (e.g. multiple expressions group into `pure-expressions-batch.md`).
