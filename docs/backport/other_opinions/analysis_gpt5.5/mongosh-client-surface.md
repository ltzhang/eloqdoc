# Mongosh Client Surface Check

This addendum inspects the sibling checkout at `../mongosh`:

- Branch: `main`
- HEAD: `6165a9b2 chore: update auto-generated files [skip actions]`
- Relevant packages: `@mongosh/shell-api` 5.1.7, `@mongosh/service-provider-core` 5.0.4, Node driver dependency `mongodb ^7.1.0`.

The earlier server-only analysis said modern `mongosh` was not present in the MongoDB server repo.  That is true for `mongo/`, but incomplete for this workspace because the actual `mongosh` source is a sibling repository.  The backport implications below should be read together with the server API documents.

## Architecture

Modern `mongosh` does not hand-build most CRUD commands the way the legacy `mongo` shell did.  It routes shell methods through a service-provider abstraction backed by the Node driver:

| Layer | Source | Role |
| --- | --- | --- |
| Shell API | `../mongosh/packages/shell-api/src/collection.ts`, `database.ts`, `mongo.ts` | User-facing methods such as `db.coll.createIndex()`, `db.coll.bulkWrite()`, `db.createEncryptedCollection()`, `db.getMongo().bulkWrite()`. |
| Service-provider interface | `../mongosh/packages/service-provider-core/src/*.ts` | Method contract used by shell-api. |
| Node driver provider | `../mongosh/packages/service-provider-node-driver/src/node-driver-service-provider.ts` | Calls `mongodb` driver collection, db, client, FLE, and search-index methods. |

For a 4.0-derived server, this means compatibility is mostly about supporting the commands the current Node driver emits, not just the TypeScript shell helper names.

## Data Helpers Observed

| Mongosh helper | Source | Server dependency | Backport implication |
| --- | --- | --- | --- |
| `db.collection.bulkWrite()` | `collection.ts` | Calls Node driver `collection.bulkWrite()`; helper is marked 3.2+. | This is not the same as the 8.0 cross-namespace `bulkWrite` command. It can work against 4.0 if the driver falls back to ordinary insert/update/delete write commands. |
| `db.getMongo().bulkWrite()` | `mongo.ts` | Calls Node driver `mongoClient.bulkWrite()`; helper is marked 8.0+. | This maps to client-level/cross-namespace bulk write and needs the 8.0 `bulkWrite` command semantics. |
| `db.collection.createIndex()`, `createIndexes()`, `ensureIndex()` | `collection.ts` | Calls driver `collection.createIndexes()` with options and optional `commitQuorum`. | A 4.0 server already has `createIndexes`; unknown modern options must be accepted/rejected carefully. `commitQuorum` should be parsed or explicitly rejected. |
| `db.collection.hideIndex()`, `unhideIndex()` | `collection.ts` | Uses `collMod` hidden index option via helper `setHideIndex`. Marked 4.4+. | Requires hidden index catalog/planner behavior for real compatibility. |
| `db.createCollection()` | `database.ts` | Passes Node driver `CreateCollectionOptions`. | Modern options such as `timeseries`, `clusteredIndex`, `encryptedFields`, `changeStreamPreAndPostImages` can be sent if the user supplies them. |
| `db.createEncryptedCollection()` | `database.ts`, `field-level-encryption.ts` | Uses `mongodb-client-encryption`/libmongocrypt `createEncryptedCollection`. | Requires Queryable Encryption/FLE2 support for real semantics. |
| `db.getMongo().getClientEncryption()` | `mongo.ts`, `field-level-encryption.ts` | Exposes encryption/decryption/data-key helpers. | Server needs key vault collection operations at minimum; encrypted collection/query APIs need FLE2 support. |
| `db.collection.getSearchIndexes()` | `collection.ts` | Calls driver `collection.listSearchIndexes()`. Helper is marked 6.0+. | Core server source has search-index commands in 8.0; Atlas/cloud deployments exposed these earlier. A 4.0-compatible server should reject unless search metadata/service exists. |
| `db.collection.createSearchIndex()`, `createSearchIndexes()` | `collection.ts` | Calls driver `collection.createSearchIndexes()`. Supports `type: "search" | "vectorSearch"` and omits `type` for plain search. | Needs search-index command support and, for usefulness, a search/vector execution backend. |
| `db.collection.dropSearchIndex()`, `updateSearchIndex()` | `collection.ts` | Calls driver search-index methods. | Same search service dependency. |
| `db.collection.analyzeShardKey()`, `configureQueryAnalyzer()` | `collection.ts` | Admin commands, marked 7.0+. | Sharding/query sampling system; ignore for data-only backport unless explicitly needed. |

## Corrections to the Server-Only Analysis

1. `mongosh` should be treated as a real client input, not absent from the workspace.
2. `db.collection.bulkWrite()` does not by itself prove the server must implement the 7.0/8.0 `bulkWrite` command, because the Node driver collection helper can use legacy batch write commands.  The 8.0-specific helper is `db.getMongo().bulkWrite()`.
3. Search index helpers are present in current `mongosh` and marked 6.0+, even though the checked server branch exposes the command IDL in 8.0.  This is a product/deployment mismatch: Atlas Search support existed outside the community server command surface.  A 4.0-derived server should either implement explicit unsupported errors or a full search-service integration.
4. FLE helpers are first-class in `mongosh`; `encryptedFields`, encrypted collection creation, and `encryptExpression()` paths make Queryable Encryption compatibility more visible to users than the server-only docs implied.
5. Modern index options reach the server through Node driver option objects.  Parser tolerance for `commitQuorum`, hidden indexes, wildcard/columnstore options, and future index options matters for user experience even before real feature support is added.

## Updated Backport Recommendations From Mongosh

Very easily doable:

- Keep legacy CRUD command compatibility strong so Node driver-backed collection helpers continue to work.
- Add clear command errors for unsupported search/FLE/time-series/columnstore options rather than unknown-command failures.
- Parse and report unsupported modern options consistently (`commitQuorum`, `encryptedFields`, search index specs, vector search type).

Needs a lot of development:

- `db.getMongo().bulkWrite()` / client-level bulk write command.
- Hidden indexes because `mongosh` exposes `hideIndex()`/`unhideIndex()` directly.
- Good `createCollection()` behavior for time-series and encrypted collections if users call those helpers.

Very hard:

- `createEncryptedCollection()` and encrypted query helpers.
- Search/vector search helpers with real semantics.
- Columnstore and other new index access methods.

