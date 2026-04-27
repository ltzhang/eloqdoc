# Tier 4 — Deferred (explicitly out of scope)

These features were considered and **deliberately deferred**. This document exists so that engineers picking up the backport plan can confirm a feature was intentionally excluded, rather than missed.

For each item, the rationale is given. If the team's product priorities change, any Tier 4 item can be re-evaluated, but each would become a new project beyond the scope of "bring EloqDoc's data API surface forward toward MongoDB 8.0."

---

## Sharding internals

**Examples:** `addShard`, `removeShard`, `moveChunk`, `mergeChunks`, `splitChunk`, `mongos` routing, config server commands, `enableSharding`, `shardCollection`, resharding (`reshardCollection`), zoned sharding.

**Why deferred:** EloqDoc has no `mongos` layer. Distributed semantics are provided by Data Substrate, which has its own model. Implementing the MongoDB sharding surface on top of Data Substrate would be a parallel architecture, not a backport. Out of scope per the original brief.

---

## Replication coordination commands

**Examples:** `replSetGetConfig`, `replSetReconfig`, `replSetStepDown`, `replSetSyncFrom`, `voteCommitIndexBuild`, `setIndexCommitQuorum`, `voteAbortIndexBuild`, `applyOps`, oplog manipulation.

**Why deferred:** EloqDoc has no oplog-based replication. Data Substrate provides distributed durability via its log service. The replica-set coordination commands have no meaning in this model.

**Exception:** `commitQuorum` parsing on `createIndexes` — accepted-and-ignored as Tier 1 (see [`tasks/commitquorum-parsing.md`](./tasks/commitquorum-parsing.md)) because clients send it.

---

## Transaction internals

**Examples:** Multi-document transaction state machines, `commitTransaction`, `abortTransaction` semantics that go beyond simple wrappers around the Data Substrate transaction primitive.

**Why deferred:** Per the original brief, transactions "can be handled correctly and easily with the lower-level storage engines." EloqDoc's Data Substrate provides ACID transactions; the MongoDB transaction-API surface should already work via that. Not a data-API task.

---

## Tenant migration / serverless

**Examples:** `tenantMigrationDonor*`, `tenantMigrationRecipient*`, `donorStartMigration`, all serverless cluster orchestration commands.

**Why deferred:** Atlas-specific feature; not relevant to a self-hosted EloqDoc deployment.

---

## FTDC (full-time diagnostic data capture) and diagnostic commands

**Examples:** `getDiagnosticData`, `getShardMap`, `xray`, `setLogLevel` extensions, internal-diagnostics-only stages.

**Why deferred:** Operational tooling, not a data API. Add per-feature only if a specific operator workflow demands it.

---

## Query analyzer / query sampling infrastructure

**Examples:** `configureQueryAnalyzer`, `$listSampledQueries` aggregation stage, `sampleId` and `mirrored` request fields, the entire `system.queryAnalyzers` and `config.sampledQueries` collection family.

**Why deferred:** This is telemetry / observability infrastructure. The request fields are stripped at the envelope ([`tasks/common-command-fields.md`](./tasks/common-command-fields.md)). Without the analyzer subsystem, sampling has no consumer.

---

## Internal-only aggregation stages

**Examples:** `$_internalUnpackBucket` (covered by [`time-series-collections.md`](./tasks/time-series-collections.md) when that's pursued), `$_internalSplitPipeline` (already in EloqDoc), `$_internalApplyOplogUpdate`, `$_internalChangeStreamSourceTransform`, `$_internalQueryAnalyzer*`.

**Why deferred:** These exist only to support other features. Implement them only as needed when their parent feature is pursued.

---

## Cluster parameters / write-block / cluster management

**Examples:** `setClusterParameter`, `getClusterParameter`, `setUserWriteBlockMode`.

**Why deferred:** Cluster-orchestration features not relevant to a single-node EloqDoc model.

---

## Test-only commands

**Examples:** `configureFailPoint`, `_testApiVersion`, `internalRenameIfOptionsAndIndexesMatch`, anything gated on `--enableTestCommands`.

**Why deferred:** Internal infrastructure for upstream tests. Not user-facing.

---

## Columnstore indexes

**Examples:** `createIndexes` with `columnstore` type, `columnstoreProjection` / `columnstoreCompressor` options.

**Why deferred:** Columnstore indexes were experimental in 6.0, GA in 7.0, and **deprecated in 8.0**. MongoDB has signaled they're moving away from this approach in favor of compound wildcard indexes. Implementing a deprecated feature is high cost / low durable value.

---

## `returnOnStart` two-phase index build

**Examples:** `createIndexes.returnOnStart` (7.0).

**Why deferred:** This is a replication-coordination feature — return to the client when the local primary starts the build, before secondaries finish. No replicas in EloqDoc → no meaningful difference between the legacy "wait for completion" semantics and the two-phase form. Accept and ignore as part of [`tasks/commitquorum-parsing.md`](./tasks/commitquorum-parsing.md) if drivers send it.

---

## `recordIdsReplicated` collection option

**Why deferred:** Specific to oplog-based replication semantics around RecordIds. Not applicable to EloqDoc's storage model.

---

## When to re-evaluate Tier 4

Move an item out of Tier 4 only when:
1. A specific product use case demands the feature.
2. There is committed engineering capacity for what is, in most cases, a multi-month project.
3. The feature can be sized with a design doc before implementation begins.

Adding a feature to the backport plan should not be a casual decision. Review tier classifications quarterly against actual user demand and EloqDoc roadmap.
