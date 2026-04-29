# Feature Backport Evaluation

Classification key:

- Very easily doable: parser/helper/pure-expression changes with low storage risk.
- Needs a lot of development: implementable on 4.0 architecture but substantial executor/planner/catalog work.
- Very hard to do: requires new storage layout, external service, encryption architecture, or broad command framework changes.
- Ignore for now: not user-facing data API or mostly sharding/replication/transactions/systems.

## Command and Parser Compatibility

| Feature/API | First relevant version here | Classification | Implementation notes |
| --- | --- | --- | --- |
| Accept modern common command fields (`comment`, API metadata, internal sampling flags) | 5.0+ | Very easily doable | Add permissive parsing/ignore path for compatibility mode. Do not advertise semantics not present. |
| Stable API strict/deprecation enforcement | 5.0 | Very hard to do | Requires command framework support, stability annotations, API parameter propagation, and per-command error behavior. |
| `hello` alias/handshake behavior | 5.0 | Very easily doable | Mostly command alias/reply compatibility, unless exact topology fields are required. |
| `bulkWrite` command | 7.0 | Needs a lot of development | Dispatch to existing CRUD per `nsInfo`; implement ordered/unordered, cursor reply, per-op errors, auth, retry stmt ids, write concern. |
| `analyze` command | 7.0 | Needs a lot of development | Minimal sampling stats possible; full value depends on optimizer stats/catalog. |
| `setQuerySettings` / `removeQuerySettings` | 8.0 | Needs a lot of development | Query shape hashing, persistent settings, planner hooks, validation, authorization. |
| Search index commands | 8.0 | Very hard to do | Real behavior requires mongot/search service. Metadata-only stubs are possible but not useful for query execution. |

## Collection Data Models

| Feature/API | First relevant version here | Classification | Implementation notes |
| --- | --- | --- | --- |
| Hidden indexes | 5.0 branch includes support from post-4.0 line | Needs a lot of development | Catalog flag plus planner exclusion, plan cache invalidation, `collMod` hide/unhide, shell helpers. |
| Wildcard indexes | Post-4.0, visible in 5.0 index spec | Needs a lot of development | Requires key generator, wildcard projection, planner bounds, multikey handling. |
| `commitQuorum` / two-phase index build options | 5.0 | Needs a lot of development | Parser-only easy; exact replica-set semantics need index build coordinator changes. |
| Time-series collections | 5.0 | Very hard to do | Bucket catalog, bucket writes, `$_internalUnpackBucket`, TTL, compression, index rewrite, update/delete semantics. |
| Clustered collections | 5.0 | Very hard to do | Physical record ordering/identity and catalog invariants change. |
| Pre/post image collection option | 6.0 | Needs a lot of development | Catalog option alone is easy; real semantics require pre-image storage and change stream integration. |
| Queryable Encryption collection metadata (`encryptedFields`) | 6.0 | Very hard to do | Requires FLE2 state collections, encrypted query planning, token handling, compaction/cleanup commands. |
| Columnstore indexes | 8.0 | Very hard to do | New access method and planner. Parser/catalog-only compatibility should reject execution use. |
| `prepareUnique` / convert-to-unique workflow | 8.0 | Needs a lot of development | Needs duplicate detection, catalog state, write conflict handling, and `collMod` integration. |

## CRUD and Query Flags

| Feature/API | First relevant version here | Classification | Implementation notes |
| --- | --- | --- | --- |
| `find.allowDiskUse` | 5.0 | Needs a lot of development | Parser easy; real support needs blocking sort spill behavior in query executor. |
| `find/update/delete/aggregate let` | 5.0 | Needs a lot of development | Must flow variables into match expression and pipeline expression contexts. |
| Write `hint` for update/delete | 5.0+ | Needs a lot of development | Requires planner enforcement and error behavior; parser alone is insufficient. |
| `collectionUUID` request targeting | 5.0+ | Ignore for now | Systems/catalog consistency feature; can reject or ignore in single-node compatibility. |
| `isTimeseriesNamespace` | 5.0+ | Ignore unless time-series implemented | Internal routing/rewrite flag. |
| `encryptionInformation` | 6.0+ | Very hard to do | Part of Queryable Encryption. |
| `sampleId`, `mirrored`, query analyzer flags | 7.0 | Ignore for now | Telemetry/sampling infrastructure. |
| `distinct.hint` | 8.0 | Very easily doable to parse; needs dev to enforce | If distinct planning already supports hints, wire through. |

## Aggregation

| Feature/API | First relevant version here | Classification | Implementation notes |
| --- | --- | --- | --- |
| Alias/simple stages (`$set`, `$unset`, `$replaceWith`, `$count`) | 5.0 | Very easily doable | Rewrite to existing stages. |
| Pure expressions: trig, regex, trim/replace, conversions, bit ops | 5.0/8.0 | Very easily doable | Add expression classes and tests. |
| Date arithmetic (`$dateAdd`, `$dateDiff`, `$dateSubtract`, `$dateTrunc`) | 6.0 | Very easily doable to needs dev | Pure expressions but tricky timezone/unit semantics. |
| `$documents` | 6.0 | Very easily doable | Literal source stage. |
| `$unionWith` | 5.0 | Needs a lot of development | Needs nested pipeline execution against another namespace. |
| `$merge` | 5.0 | Needs a lot of development | Writes from pipeline with unique key and retry/write concern semantics. |
| `$fill`, `$densify` | 6.0/7.0 | Needs a lot of development | Sorted stream, partition, date/numeric bounds. |
| `$setWindowFields` and window accumulators | 7.0 | Needs a lot of development | Partition/sort/window executor and memory/spill behavior. |
| Top/bottom/N accumulators | 7.0 | Needs a lot of development | Accumulator logic, sorting, memory limits; no storage change. |
| `$median`, `$percentile` | 8.0 | Needs a lot of development | Approximation algorithm and accumulator/window integration. |
| `$search`, `$searchMeta`, `$vectorSearch` | 8.0 | Very hard to do | External mongot/search integration. |

## Client-Side Work

| Feature/API | Classification | Implementation notes |
| --- | --- | --- |
| Legacy shell helper wrappers for new commands/options | Very easily doable | Add JavaScript wrappers under `src/mongo/shell` if maintaining legacy shell. |
| Modern `mongosh` compatibility | Needs server command compatibility more than shell edits | The sibling `../mongosh` checkout is Node-driver-backed. Collection helpers generally call driver collection/db/client methods; update server parsers and semantics first. |
| `mongosh` `db.getMongo().bulkWrite()` | Needs a lot of development | Marked 8.0+ in `../mongosh/packages/shell-api/src/mongo.ts`; requires client-level/cross-namespace `bulkWrite`. |
| `mongosh` search index helpers | Very hard to do | `createSearchIndex(es)`, `getSearchIndexes`, `dropSearchIndex`, `updateSearchIndex` call Node driver search-index methods and need a search backend for real behavior. |
| Server-side C++ client command construction | Very easily doable to needs dev | Update `src/mongo/client` helpers only where internal code sends newer command shapes. |
