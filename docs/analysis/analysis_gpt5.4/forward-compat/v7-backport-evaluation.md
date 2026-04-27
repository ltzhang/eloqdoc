# v7 Backport Evaluation For A 4.0 Base

For your scope, v7 introduces one important new write API and one expensive encryption feature family. Much of the rest is sharding, telemetry, or analytical infrastructure that you can safely defer.

## High-value user-facing additions

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| Server-side `bulkWrite` command | `bulkWrite` | `Needs a lot of development` | You can build this on top of existing insert/update/delete machinery, but matching modern semantics needs nontrivial work: multi-namespace request parsing, ordered/unordered behavior, per-op result accounting, cursor responses, statement IDs, and error batching. |
| Queryable-encryption count metadata | `getQueryableEncryptionCountInfo` | `Very hard to do` | This is part of the same FLE2 architecture as `encryptedFields` and compaction. Not realistic as a standalone compatibility patch. |
| New capped-collection `collMod` behaviors | `collMod.cappedSize`, `collMod.cappedMax`, `dryRun` | `Needs a lot of development` | This is doable because 4.0 already has capped collections, but it still needs catalog updates, validation, and careful correctness rules. |
| New index state controls through `collMod` | `unique`, `prepareUnique`, `forceNonUnique` in later `collMod` request shapes | `Very hard to do` | This reaches deep into index build semantics, duplicate checking, uniqueness enforcement, and transitional states. It is far more invasive than hidden indexes. |

## Features to deprioritize

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| Query analyzer | `analyzeShardKey`, `configureQueryAnalyzer`, `_refreshQueryAnalyzerConfiguration` | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | This is tied to sharding analysis and telemetry, which you explicitly excluded. |
| Metadata consistency APIs | `checkMetadataConsistency` and related internal commands | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | Administrative consistency tooling, not a core data API. |
| Sampling / mirrored / telemetry fields | `sampleId`, `mirrored`, `returnOnStart`, `includeBuildUUIDs` changes | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | These are not the right place to spend engineering effort early. |
| OIDC shell flag | `oidcAccessToken` | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | Client-auth surface, not central to your CRUD/data-model roadmap. |

## Practical take

If you only want one meaningful v7 feature for application developers, it is `bulkWrite`.

If you want to ship it in a way that developers will trust:

- parse the v7 request form cleanly
- route each op to existing write paths
- preserve ordered/unordered semantics
- return modern result grouping and error reporting

That is absolutely feasible on top of 4.0, but it is not “small.”
