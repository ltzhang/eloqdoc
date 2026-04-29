# Forward Compatibility Evaluation: Implementing v5–v8 APIs on MongoDB v4.0

**Premise:** You have the MongoDB 4.0 source codebase (`origin/v4.0`). You want to implement forward compatibility — making the server accept and correctly execute APIs from v4.2 through v8.0 — without using the original source code of those later versions.

**Scope:** Data model, indexing, query, aggregation, encryption, and auth APIs only. Sharding, replication, and transaction internals are excluded.

---

## Rating Scale

| Category | Label | Definition |
|---|---|---|
| **1** | Very Easily Doable | Pure command/parsing/logic change; no storage, index format, or infrastructure changes needed. Can be done by one engineer in days to a week. |
| **2** | Needs a Lot of Development | Significant new code required, but fits within existing architecture patterns. Weeks to a few months. |
| **3** | Very Hard | Requires new storage formats, deep core refactoring, new cryptographic infrastructure, or integration with external services. Months to a year+. |
| **4** | Ignore for Now | Systems-level (sharding, replication), not user-facing, or a pure storage-engine concern. Do not implement on v4.0 codebase unless specifically needed. |

---

## Feature-by-Feature Evaluation

---

### A. Index Types

#### A1. Wildcard Index (`"wildcard"`) — Introduced: v4.2
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. New `IndexType` enum value and `IndexNames::WILDCARD` constant in `index_names.h`.
2. New key generator (`wildcard_key_generator.cpp/h`): must recursively enumerate all field paths in a document (respecting `wildcardProjection`) and generate a key for each path-value pair.
3. New access method (`wildcard_access_method.cpp/h`): implements `IndexAccessMethod` interface — insert, delete, validate, and getKeys.
4. Query planner changes: teach the planner to recognize wildcard-indexed fields and create an index scan for any field in the wildcard path. The planner must handle the case where the index contains keys for multiple fields, and extract the relevant bounds for a given query predicate.
5. Integration with `createIndexes` command to accept `wildcardProjection` option.
6. Ensure `$**` key pattern parses correctly.

**Why it's category 2 (not 3):**
The existing infrastructure for index access methods (the `IndexAccessMethod` base class) is well-abstracted. The key generator logic is complex but self-contained. The query planner integration is the hardest part but follows existing patterns (similar to how multikey index planning works).

**Key files to create/modify:**
- `src/mongo/db/index/wildcard_access_method.cpp/h`
- `src/mongo/db/index/wildcard_key_generator.cpp/h`
- `src/mongo/db/index_names.h` (add constant)
- `src/mongo/db/query/planner_wildcard_helpers.cpp/h`
- `src/mongo/db/query/index_entry.h` (wildcard metadata)

---

#### A2. Compound Wildcard Indexes — Introduced: v6.0 (exp), v7.0 (GA)
**Rating: 3 — Very Hard**

**What it requires:**
Building on A1, additionally:
1. The key generator must handle compound keys where one component is a wildcard expansion.
2. The planner must reason about which field in the compound is the wildcard and which are concrete.
3. Multi-key interaction: a compound wildcard index on `{ a.$**: 1, b: 1 }` where `a` is an array requires tracking both wildcard and standard multi-key paths.
4. Significant query planner work for bound calculation and index intersection.

**Why it's category 3:** The interaction between wildcard semantics and compound index planning is deeply complex. The planner needs major new logic to reason about which sub-predicates are satisfiable by which component.

---

#### A3. 2DSphere Bucket Index — Introduced: v6.0
**Rating: 4 — Ignore for Now**

This index type exists only to support geospatial indexing on the internal `system.buckets.*` collections used by time-series. It is not user-created directly. You would implement it as part of time-series collection support (see C1). Do not implement in isolation.

---

#### A4. Column Store Index (`"columnstore"`) — Introduced: v6.0
**Rating: 3 — Very Hard**

**What it requires:**
1. Completely new storage layout: instead of storing `{ key: value, recordId }` per document, a column store stores all values for field `f` together across documents.
2. New access method with columnar scan operations.
3. A new cursor type that can return projected fields without touching the full document.
4. Query planner changes to recognize when a column store can service a projection-heavy scan.
5. Integration with the WiredTiger or alternative storage engine's column API (or build your own columnar storage layer).

**Additionally:** MongoDB 8.0 deprecated this index type. Implementing it on v4.0 is very expensive for diminishing returns.

**Recommendation:** Skip entirely.

---

#### A5. Encrypted Range Index (`"queryable_encrypted_range"`) — Introduced: v7.0
**Rating: 4 — Ignore for Now**

This is an internal implementation detail of FLE2 range queries. It is not user-created; it is created implicitly by the FLE2 subsystem. Implementing FLE2 range (see G2) would include this.

---

### B. Index Build Changes

#### B1. `commitQuorum` on `createIndexes` — Introduced: v4.4
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. Add `commitQuorum` parameter parsing in `createIndexes` command.
2. Implement a two-phase index build protocol: build phase (all members build in background) + commit phase (primary waits for N members to report completion).
3. New internal command: `voteCommitIndexBuild` for secondaries to report readiness.
4. New internal command: `setIndexCommitQuorum` to change the quorum after start.

**Why it's category 2 and not 3:** The index build infrastructure exists; this is adding orchestration on top. The hardest part is the distributed coordination, but since you're ignoring replication internals, you may implement a simplified version: always commit when the primary finishes (equivalent to `commitQuorum: 0`).

**Simplified path:** Accept and parse `commitQuorum` but always use primary-only semantics → effectively category 1.

---

#### B2. Removal of `background` Option — Introduced: v4.4 (ignored), v5.0 (removed)
**Rating: 1 — Very Easily Doable**

Simply stop honoring the `background` field (or silently ignore it). All builds become what was previously called "background" by default. This requires changing how the index build is started (always use the non-blocking path) — a small code change if index builds already run without exclusive locks in 4.0 (they do for replica sets with `--background`).

---

### C. Collection Types

#### C1. Time-Series Collections — Introduced: v5.0
**Rating: 2 — Needs a Lot of Development** (for basic support) / **3 — Very Hard** (for full feature parity)

**Basic support (category 2):**
Time-series collections can be implemented as a virtualization layer on top of normal MongoDB collections:

1. **Storage:** Create a system collection `system.buckets.<name>` as the physical storage. Use a clustered-by-`_id` or hashed approach for bucket addressing.
2. **Bucket catalog:** Maintain an in-memory or persistent mapping from `(metaField value, time bucket) → bucket document _id`. Source: `src/mongo/db/timeseries/bucket_catalog.cpp`.
3. **Insert routing:** Intercept inserts to `<name>`, route to the appropriate bucket in `system.buckets.<name>`, creating a new bucket document if none matches. Pack measurements into the columnar `data` field.
4. **Query translation:** Intercept queries on `<name>`, translate to queries on `system.buckets.<name>`, and inject `$_internalUnpackBucket` to decompress results.
5. **View registration:** Register `<name>` as a view over `system.buckets.<name>`.
6. **`create` command:** Accept and parse `timeseries` option, `expireAfterSeconds`.

**What makes it hard:**
- The bucket catalog must handle concurrent inserts efficiently.
- Bucket compression (BSON delta compression) significantly improves storage but adds complexity.
- Time-series query optimization (skipping buckets by time range) requires query planner changes.
- Secondary indexes on time-series fields require special handling.

**Full parity hardness (category 3):** Updates/deletes on time-series, sharding, geospatial indexes, upserts (v8.0), and extended timestamp range all add significant complexity. Start with insert + find + TTL and expand gradually.

**Recommended phasing:**
1. Create `system.buckets.*`, intercept inserts → **2 weeks**
2. Basic find with bucket unpacking → **1 week**
3. TTL support → **3 days**
4. Secondary indexes → **1 week**
5. Updates/deletes → **2 weeks**

---

#### C2. Clustered Collections — Introduced: v6.0
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. The WiredTiger storage engine already supports clustering by a key (its internal table storage is B-tree keyed by `_id`). The main work is exposing this to the MongoDB layer.
2. Add `clusteredIndex` parsing in `create` command.
3. Remove the separate `_id` index creation for clustered collections.
4. Update the catalog to store `ClusteredCollectionInfo`.
5. Update query planning to use range scans on the cluster key directly (without a separate index).
6. Update `collStats` and `listIndexes` to correctly report the clustered index.

**Why it's category 2:** WiredTiger (the storage engine) already stores records sorted by `_id`. The main work is removing the extra `_id` index and teaching the query planner to exploit the physical order.

---

#### C3. Encrypted Collections (FLE2) — Introduced: v6.0
**Rating: 3 — Very Hard**

See section G (Encryption). Creating a collection with `encryptedFields` is the easy part (just storing metadata). The hard part is everything else — query rewriting, state collections, key management.

---

### D. Aggregation Pipeline Stages

#### D1. `$merge` — Introduced: v4.2
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. Parse the `$merge` specification (IDL or manual parser).
2. After the pipeline produces documents, implement the merge loop:
   - For each output document, apply the `on` field(s) as a lookup key.
   - Execute an `updateOne` with `upsert: true` using the appropriate `whenMatched` logic.
   - Handle `whenMatched: "merge"` via `$set` operator.
   - Handle `whenMatched: [<pipeline>]` by running a sub-pipeline update.
3. Cross-database write support (look up the target namespace).
4. Ensure atomic semantics at the per-document level.

**Why it's category 2:** The execution logic (upsert loop) is straightforward. The tricky parts are the pipeline-as-update and ensuring correct error handling for the various `whenNotMatched: "fail"` cases.

---

#### D2. `$planCacheStats` — Introduced: v4.2
**Rating: 1 — Very Easily Doable**

The plan cache infrastructure already exists in 4.0. This stage just reads entries from the in-memory plan cache and formats them as documents. Implement `DocumentSourcePlanCacheStats` that iterates the cache and returns plan documents.

**Effort:** 2–3 days.

---

#### D3. `$unionWith` — Introduced: v4.4
**Rating: 1 — Very Easily Doable**

**What it requires:**
1. Parse `{ $unionWith: "<coll>" | { coll, pipeline } }`.
2. Execute the sub-pipeline on the specified collection.
3. Concatenate the results to the current pipeline's output.

This is essentially: run two pipelines, concatenate results. No storage changes, no complex planner integration. Just a new `DocumentSource` implementation.

**Effort:** 3–5 days.

---

#### D4. `$setWindowFields` — Introduced: v5.0
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. A new `DocumentSourceSetWindowFields` stage.
2. Window frame computation: track a sliding window of documents (position-based or range-based).
3. For each window operator (`$sum`, `$avg`, `$rank`, etc.), implement the windowed variant.
4. `partitionBy` support: process each partition independently.
5. For range-based windows on dates, time arithmetic.
6. For `$rank`/`$denseRank`/`$documentNumber`: maintain rank within partition.
7. For `$expMovingAvg`: exponential smoothing computation.
8. For `$integral`/`$derivative`: numerical integration/differentiation over time windows.

**Why it's category 2 (not 3):** Window functions are computationally defined — no new storage format needed. The complexity is in the logic (especially for partition management and the various frame types), but it all operates on in-memory document buffers.

**Estimated effort:** 4–8 weeks for core `$rank`, `$sum`, `$avg`, `$expMovingAvg`. Additional 2–4 weeks for `$integral`, `$derivative`, `$covariancePop/Samp`.

---

#### D5. `$densify` — Introduced: v6.0
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. Parse the `$densify` specification (field, range.step, range.bounds, unit, partitionByFields).
2. Implement a gap detector: as documents flow through, detect missing values in the sequence.
3. Generate synthetic documents to fill the gaps.
4. For date fields, use MongoDB's date arithmetic to compute the correct date offsets.
5. For `bounds: "partition"`: maintain per-partition state.

**Why it's category 2:** All logic is in the stage itself; no storage changes. The date arithmetic and partition tracking are non-trivial but self-contained.

**Effort:** 2–4 weeks.

---

#### D6. `$fill` — Introduced: v6.0
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. Parse the `$fill` specification.
2. For `method: "locf"`: maintain the last observed non-null value per partition.
3. For `method: "linear"`: buffer two surrounding non-null values, then interpolate for all null docs between them.
4. For `value: <expr>`: simply substitute the expression result.

This is implementable on top of `$setWindowFields` or as a standalone stage. MongoDB's implementation internally decomposes `$fill` into `$setWindowFields` + `$addFields`.

**Effort:** 2–3 weeks (can reuse $setWindowFields infrastructure).

---

#### D7. `$accumulator` (Custom JS Accumulator in `$group`) — Introduced: v4.4
**Rating: 1 — Very Easily Doable**

The 4.0 codebase already has JavaScript execution infrastructure (SpiderMonkey). Implementing `$accumulator` means:
1. Parse the spec (init, accumulate, merge, finalize, lang).
2. Compile and cache the JS functions.
3. Call `init()` at group start, `accumulate()` per document, `merge()` when splitting, `finalize()` at group end.

The JS execution environment exists; this is just wiring.

**Effort:** 1–2 weeks.

---

#### D8. `$function` (Custom JS Expression) — Introduced: v4.4
**Rating: 1 — Very Easily Doable**

Similar to D7. Parse `{ $function: { body, args, lang } }`, compile the function, and call it with the args evaluated from the document. Fits within the existing expression evaluation framework.

**Effort:** 3–5 days.

---

#### D9. New Accumulators: `$minN`, `$maxN`, `$firstN`, `$lastN`, `$top`, `$bottom`, `$topN`, `$bottomN` — Introduced: v6.0
**Rating: 1 — Very Easily Doable**

These are new `$group` accumulators. Each maintains a bounded heap or sorted buffer:
- `$minN`/`$maxN`: maintain a min/max heap of size N.
- `$firstN`/`$lastN`: maintain a circular buffer of size N.
- `$top`/`$bottom`/`$topN`/`$bottomN`: maintain a sort-keyed heap of size N.

All follow the existing `Accumulator` interface in 4.0. No storage changes, no planner changes.

**Effort:** 1–2 weeks for all of them together.

---

#### D10. `$locf` Accumulator (Window) — Introduced: v6.0
**Rating: 1 — Very Easily Doable**

If `$setWindowFields` (D4) is implemented, `$locf` is a trivial window function: carry the last non-null value forward.

**Effort:** 1–2 days after D4 is done.

---

#### D11. `$percentile` and `$median` — Introduced: v7.0
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. Implement the t-digest algorithm (the "approximate" percentile method MongoDB uses). This is a well-known streaming algorithm for approximate quantile computation.
2. Integrate as a new `Accumulator` class for `$group`.
3. Integrate as a window function operator for `$setWindowFields`.
4. Handle the `p` array (multiple percentiles from one pass).

**Why it's category 2:** The t-digest algorithm is well-documented but non-trivial to implement correctly. Mergeability (needed for distributed aggregation) requires careful implementation.

**Alternative:** Use an exact algorithm (sort + index) within `$group` where memory allows. For large groups, t-digest is necessary.

**Effort:** 2–4 weeks.

---

#### D12. `$listCatalog`, `$queryStats`, `$listSampledQueries` — Introduced: v6.0/v7.0
**Rating: 1 — Very Easily Doable**

Each is a source stage that reads internal server state (catalog, query stats counters, sampled query store) and formats it as documents. No storage or planner changes.

**`$listCatalog`:** Iterate the catalog and return collection metadata. 2–3 days.  
**`$queryStats`:** Requires a query stats collection subsystem. 1–2 weeks for the stats store + stage.  
**`$listSampledQueries`:** Requires a query sampling framework. 2–3 weeks.

---

#### D13. `$changeStreamSplitLargeEvent` — Introduced: v6.0
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. After change stream events are produced, check if any event document exceeds 16MB.
2. If so, split the event into fragments and attach fragment metadata.
3. The client reassembles the fragments.

Requires understanding the internal change stream pipeline structure, which is complex.

**Effort:** 2–4 weeks, mostly to understand and hook into the change stream internals.

---

#### D14. `$vectorSearch`, `$search`, `$searchMeta`, `$rankFusion`, `$scoreFusion` — Introduced: v8.0
**Rating: 3 — Very Hard**

**What they require:**
- `$vectorSearch`: Requires a vector index (ANN index) and a `mongot` search daemon running alongside MongoDB. The ANN index itself (HNSW, IVF, etc.) is complex to implement correctly.
- `$search` / `$searchMeta`: Full Lucene-based full-text search via external `mongot` process.
- `$rankFusion` / `$scoreFusion`: Can be implemented once `$vectorSearch` and `$search` exist; the fusion logic itself is simpler.

**Why it's category 3:** `$vectorSearch` requires:
1. A new index type with ANN data structure (HNSW graph or IVF clusters).
2. An external `mongot` process or a built-in ANN implementation.
3. Inter-process communication between mongod and mongot.
4. Score propagation through the pipeline.

Implementing a basic ANN (approximate nearest neighbor) from scratch is a substantial engineering effort. Using an open-source library (e.g., hnswlib, faiss) is feasible but still requires significant integration work.

**Estimated effort:** 3–6 months for basic vector search. Atlas Search is even more work (requires Lucene).

---

#### D15. `$querySettings` Stage — Introduced: v8.0
**Rating: 4 — Ignore for Now**

This is a query optimizer hint system, not a data model API. It does not affect what data can be stored or retrieved, only how queries are planned.

---

### E. Update Command Features

#### E1. Aggregation Pipeline Form for `update` — Introduced: v4.2
**Rating: 1 — Very Easily Doable**

**What it requires:**
1. Detect if the `u` field in an update is an array (pipeline) vs. a document (operator-based).
2. If array: run the array as an aggregation pipeline over the matched document to produce the replacement document.
3. Use the result as the new document value (or as the update payload for `$set`-like behavior).

The aggregation pipeline infrastructure already exists. Feeding a single document through a pipeline and using the result is straightforward.

**Effort:** 1–2 weeks.

---

#### E2. Update/Delete on Time-Series Collections — Introduced: v6.0
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. For **metaField updates:** Extract the bucket containing the document, update the `meta` field in the bucket document.
2. For **measurement field updates:** Full bucket rewrite (unpack bucket → modify document → repack bucket).
3. For **deletes:** Unpack bucket, remove specified documents, repack remaining.
4. Handle the case where an update empties a bucket (delete the bucket document).

**Why it's category 2:** The bucket read/write logic is complex, but all operations are built on existing CRUD infrastructure.

---

#### E3. Upsert into Time-Series — Introduced: v8.0
**Rating: 2 — Needs a Lot of Development**

Builds on E2. When no document matches, insert a new measurement. The new measurement goes through normal time-series insert routing (bucket selection, packing).

---

#### E4. `bulkWrite` Command — Introduced: v7.0
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. New top-level command `bulkWrite` that accepts `ops` (mixed insert/update/delete) and `nsInfo` arrays.
2. Route each operation to the appropriate collection based on `nsInfo` index.
3. Execute operations in order (if `ordered: true`) or in parallel (if `ordered: false`).
4. Aggregate results into a unified response.

The individual insert/update/delete operations reuse existing write command infrastructure.

**Effort:** 2–4 weeks.

---

### F. Auth and Security Features

#### F1. SCRAM-SHA-256 — Already in v4.0
**Rating: 4 — Already Done**

SCRAM-SHA-256 is already in MongoDB 4.0.

---

#### F2. CSFLE Schema Enforcement — Introduced: v4.2
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. Extend the JSON Schema validator to recognize `encrypt` and `encryptMetadata` keywords.
2. During document validation, check that fields with `encrypt` specifications contain BinData subtype 6.
3. Parse the `encrypt` spec (keyId, bsonType, algorithm).
4. Allow `bypassDocumentValidation: true` to skip encryption checks.

The server itself does NOT decrypt values — it only validates the shape. This is implementable without a crypto library.

**Effort:** 2–3 weeks for the schema parser + validator integration.

---

#### F3. Certificate Rotation (`rotateCertificates`) — Introduced: v4.4
**Rating: 1 — Very Easily Doable**

**What it requires:**
1. Register a new command `rotateCertificates`.
2. On execution, reload the TLS context from the configured certificate files without restarting.

The OpenSSL/BoringSSL context reload logic exists in the 4.0 codebase (TLS is already supported). This is plumbing to trigger it on demand.

**Effort:** 3–5 days.

---

#### F4. Queryable Encryption (FLE2) — Introduced: v6.0
**Rating: 3 — Very Hard**

**What it requires:**
1. **New crypto primitives:** FLE2 uses specialized encryption schemes (AEAD with deterministic encryption for equality, structured encryption for range). These are not available in standard crypto libraries.
2. **State collections:** For each encrypted collection, maintain ESC (Encrypted State Collection), ECC (Encrypted Counter Collection), ECOC (Encrypted Collection Offset Collection).
3. **Query rewriting:** When a query contains an encrypted equality predicate, rewrite it to look up the ESC and then match documents using the encrypted tokens.
4. **Insert handling:** On insert, update the ESC and ECC with the encrypted field tokens.
5. **Delete/update handling:** Update the ECOC and ECC on modification.
6. **`compactStructuredEncryptionData`:** Merge and compact the state collections.
7. **`cleanupStructuredEncryptionData`:** Remove deleted-document tokens.

**Why it's category 3:** The cryptographic scheme (Structured Encryption from Pappas et al.) is highly specialized. Implementing it correctly requires deep crypto expertise. Getting the state collection management correct under concurrent writes is also very challenging.

**Recommendation:** If equality-only FLE2 is needed, it's category 3 but achievable. Range FLE2 is even harder and likely requires crypto library support.

---

#### F5. MONGODB-OIDC Authentication — Introduced: v7.0
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. New SASL mechanism implementation.
2. OIDC token validation (JWT parsing, signature verification, claims validation).
3. Integration with an external OIDC identity provider.
4. Human-in-the-loop flow for interactive authentication.
5. Machine authentication flow for service accounts.

**Effort:** 4–6 weeks. JWT libraries are available; the SASL plumbing exists.

---

### G. Encryption (FLE2 Components — Standalone)

#### G1. `encryptedFields` in `create` — Introduced: v6.0
**Rating: 1 — Very Easily Doable** (metadata only)

Storing the `encryptedFields` configuration in collection metadata is straightforward — just add it to `CollectionOptions` and persist it in the catalog. The hard part is honoring it (see G2).

**Effort:** 2–3 days for metadata storage.

---

#### G2. FLE2 Query Execution — Introduced: v6.0
**Rating: 3 — Very Hard**

See F4 above. This is the same feature — query rewriting to use ESC tokens for encrypted equality matches.

---

#### G3. `compactStructuredEncryptionData` — Introduced: v6.0
**Rating: 3 — Very Hard**

Requires FLE2 state collection infrastructure to be in place (F4). Cannot be implemented in isolation.

---

#### G4. `cleanupStructuredEncryptionData` — Introduced: v8.0
**Rating: 3 — Very Hard**

Same dependency on FLE2 infrastructure as G3.

---

### H. Time-Series-Specific Features

See section C1 for the base implementation. Additional features:

#### H1. Secondary Indexes on Time-Series — Introduced: v6.0
**Rating: 2 — Needs a Lot of Development**

Requires time-series base (C1) to be in place. Additionally:
- Index creation on the underlying `system.buckets.*` collection with schema conversion (field paths in measurement documents map to different paths in bucket documents).
- Query planner must translate index scans on measurement fields to scans on the bucket collection.

**Source reference:** `src/mongo/db/timeseries/timeseries_index_schema_conversion_functions.cpp`

---

#### H2. Geospatial Indexes on Time-Series — Introduced: v6.0
**Rating: 3 — Very Hard**

Requires both time-series support (C1) and the `2dsphere_bucket` index type implementation. The bucket index type must index the bounding box of all geospatial values in a bucket, which is non-trivial.

---

### I. Versioned API

#### I1. `apiVersion`, `apiStrict`, `apiDeprecationErrors` — Introduced: v5.0
**Rating: 1 — Very Easily Doable**

**What it requires:**
1. Parse these three fields from any incoming command request.
2. Store them in the `OperationContext` or command invocation context.
3. Mark each IDL-defined command with `api_version: "1"` or `""`.
4. If `apiStrict: true` and the command is not in v1, return an error.
5. If `apiDeprecationErrors: true` and a deprecated feature is used, return an error.

**Effort:** 1–2 weeks for the core mechanism; ongoing work to annotate each command correctly.

---

### J. Change Streams

#### J1. Pre/Post-Image Support — Introduced: v6.0
**Rating: 2 — Needs a Lot of Development**

**What it requires:**
1. Add `changeStreamPreAndPostImages: { enabled: bool }` to `CollectionOptions`.
2. On every update/delete operation, capture the pre-image (read the document before modification).
3. Store pre-images in a separate `config.system.preimages` collection with a TTL.
4. On every insert/update/replace, capture the post-image and optionally store it.
5. Change stream pipeline: when `fullDocumentBeforeChange` is requested, look up the pre-image from `config.system.preimages`.

**Effort:** 3–6 weeks. The CRUD hooks to capture pre-images and the pre-image collection management are the complex parts.

---

#### J2. `showExpandedEvents` (DDL events) — Introduced: v6.0
**Rating: 2 — Needs a Lot of Development**

Requires the change stream to emit events for DDL operations (createCollection, dropCollection, createIndexes, etc.). Involves hooking into the DDL command paths to generate oplog entries that change stream can pick up.

---

### K. Miscellaneous Commands

#### K1. `validateDBMetadata` — Introduced: v5.0
**Rating: 1 — Very Easily Doable**

Iterate all collections and indexes in a database, check their metadata against API version constraints. Straightforward catalog iteration + validation logic.

**Effort:** 1–2 weeks.

---

#### K2. `getClusterParameter` / `setClusterParameter` — Introduced: v6.0
**Rating: 4 — Ignore for Now**

These are for cluster-wide configuration management (stored in `config.clusterParameters`). Not data model APIs.

---

#### K3. `setUserWriteBlockMode` — Introduced: v6.0
**Rating: 4 — Ignore for Now**

Admin-only command for blocking all writes cluster-wide. Not a data model API.

---

#### K4. `getFLECount` — Introduced: v7.0
**Rating: 2 — Needs a Lot of Development**

Depends on FLE2 state collections (F4) being in place. The command itself (counting ESC documents) is trivial; the prerequisite is the hard part.

---

#### K5. `createSearchIndex`, `dropSearchIndex`, `updateSearchIndex` — Introduced: v8.0
**Rating: 3 — Very Hard**

Requires integration with `mongot` (external search daemon) or a built-in full-text/vector search engine. The command parsing is easy; the backend integration is very hard.

---

#### K6. `autoCompact` — Introduced: v8.0
**Rating: 4 — Ignore for Now**

Storage-engine-level automatic compaction. Not a data model API.

---

#### K7. `setQuerySettings` / `removeQuerySettings` — Introduced: v8.0
**Rating: 4 — Ignore for Now**

Query optimizer hint system. Not a data model API.

---

## Summary Table

| Feature | Version | Rating | Label |
|---|---|---|---|
| Wildcard index (`$**`) | 4.2 | **2** | Needs dev |
| Compound wildcard index | 6.0/7.0 | **3** | Very hard |
| `2dsphere_bucket` index | 6.0 | **4** | Ignore (part of TS) |
| Column store index | 6.0/7.0 | **3** | Very hard |
| Encrypted range index | 7.0 | **4** | Ignore (part of FLE2) |
| `commitQuorum` on index builds | 4.4 | **2** | Needs dev |
| Remove `background` option | 4.4/5.0 | **1** | Very easy |
| Time-series collections (basic) | 5.0 | **2** | Needs dev |
| Time-series full feature parity | 5.0–8.0 | **3** | Very hard |
| Clustered collections | 6.0 | **2** | Needs dev |
| FLE2/Queryable Encryption | 6.0 | **3** | Very hard |
| `$merge` | 4.2 | **2** | Needs dev |
| `$planCacheStats` | 4.2 | **1** | Very easy |
| `$unionWith` | 4.4 | **1** | Very easy |
| `$setWindowFields` | 5.0 | **2** | Needs dev |
| `$densify` | 6.0 | **2** | Needs dev |
| `$fill` | 6.0 | **2** | Needs dev |
| `$accumulator` (custom JS) | 4.4 | **1** | Very easy |
| `$function` (custom JS expr) | 4.4 | **1** | Very easy |
| `$minN`/`$maxN`/`$firstN` etc | 6.0 | **1** | Very easy |
| `$locf` window accumulator | 6.0 | **1** | Very easy |
| `$percentile` / `$median` | 7.0 | **2** | Needs dev |
| `$listCatalog` | 6.0 | **1** | Very easy |
| `$queryStats` | 6.0 | **1** | Needs dev (~2 weeks) |
| `$changeStreamSplitLargeEvent` | 6.0 | **2** | Needs dev |
| `$vectorSearch` | 8.0 | **3** | Very hard |
| `$search` / Atlas Search | 8.0 | **3** | Very hard |
| `$rankFusion` / `$scoreFusion` | 8.0 | **3** | Very hard (dep) |
| `$querySettings` | 8.0 | **4** | Ignore |
| `$listSearchIndexes` | 8.0 | **3** | Very hard (dep) |
| Update pipeline form | 4.2 | **1** | Very easy |
| Update/delete on time-series | 6.0 | **2** | Needs dev |
| Upsert into time-series | 8.0 | **2** | Needs dev |
| `bulkWrite` command | 7.0 | **2** | Needs dev |
| CSFLE schema enforcement | 4.2 | **2** | Needs dev |
| Certificate rotation | 4.4 | **1** | Very easy |
| FLE2 equality queries | 6.0 | **3** | Very hard |
| FLE2 range queries | 7.0 | **3** | Very hard |
| MONGODB-OIDC auth | 7.0 | **2** | Needs dev |
| `encryptedFields` (metadata only) | 6.0 | **1** | Very easy |
| `compactStructuredEncryptionData` | 6.0 | **3** | Very hard (dep) |
| `cleanupStructuredEncryptionData` | 8.0 | **3** | Very hard (dep) |
| Pre/post-image change streams | 6.0 | **2** | Needs dev |
| DDL events in change streams | 6.0 | **2** | Needs dev |
| Versioned API enforcement | 5.0 | **1** | Very easy |
| Secondary indexes on time-series | 6.0 | **2** | Needs dev |
| Geospatial on time-series | 6.0 | **3** | Very hard |
| `validateDBMetadata` | 5.0 | **1** | Very easy |
| `getClusterParameter/setClusterParameter` | 6.0 | **4** | Ignore |
| `setUserWriteBlockMode` | 6.0 | **4** | Ignore |
| `getFLECount` | 7.0 | **2** | Needs dev (dep) |
| Search index commands | 8.0 | **3** | Very hard |
| `autoCompact` | 8.0 | **4** | Ignore |
| `setQuerySettings` | 8.0 | **4** | Ignore |
| `recordIdsReplicated` in create | 8.0 | **4** | Ignore |
| `$listClusterCatalog` | 8.0 | **4** | Ignore |

---

## Recommended Implementation Order

### Phase 1: Low-Hanging Fruit (Category 1)
Estimated: 2–4 months total, can be parallelized.

1. **Remove deprecated commands** (`eval`, `group` → return `CommandNotFound`; `geoNear` cmd → redirect to error)
2. **Update pipeline form** (`u: [<pipeline>]` in `update`)
3. **`$merge` stage** ← actually category 2, do this early
4. **`$planCacheStats` stage**
5. **`$unionWith` stage**
6. **`$accumulator` and `$function`** (custom JS in aggregation)
7. **`$first`/`$last` as array expressions**
8. **New accumulators** (`$minN`, `$maxN`, `$firstN`, `$lastN`, `$top`, `$bottom`, `$topN`, `$bottomN`)
9. **Trigonometric expressions** (`$sin`, `$cos`, etc.)
10. **Certificate rotation** (`rotateCertificates`)
11. **Versioned API** (`apiVersion`, `apiStrict`, `apiDeprecationErrors`)
12. **`encryptedFields` metadata storage** (store in catalog; don't enforce yet)
13. **`$listCatalog`**
14. **Remove `background` index option**

### Phase 2: Core Features (Category 2)
Estimated: 6–12 months total.

1. **Wildcard indexes** — most impactful missing index type
2. **`$setWindowFields`** with core window operators
3. **Time-series collections** (basic: insert + find + TTL)
4. **`$merge` stage** (if not done in Phase 1)
5. **CSFLE schema enforcement** (validate encrypted field presence)
6. **`$densify` and `$fill`**
7. **`$percentile` and `$median`**
8. **Clustered collections**
9. **Pre/post-image for change streams**
10. **`bulkWrite` command**
11. **Time-series secondary indexes**
12. **`commitQuorum` for index builds**
13. **MONGODB-OIDC auth**
14. **`$changeStreamSplitLargeEvent`**
15. **Time-series updates and deletes**

### Phase 3: Advanced / Hard Features (Category 3)
Only tackle after Phase 2 is solid.

1. **FLE2 equality** (if queryable encryption is needed)
2. **Vector search** (if vector similarity search is needed)
3. **Compound wildcard indexes** (if needed beyond simple `$**`)
4. **Column store indexes** (deprioritize; deprecated in 8.0)

### Phase 4: Skip (Category 4)
Do not implement on v4.0 codebase:
- Sharding commands (`shardsvr_*`, `cluster_*`)
- Replication / tenant migration
- `setQuerySettings` (optimizer-only)
- `autoCompact` (storage engine)
- `recordIdsReplicated` (replication level)
- `$listClusterCatalog` (cluster-level admin)
