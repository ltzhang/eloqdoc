# v5 Backport Evaluation For A 4.0 Base

This document covers the user-facing parts of the v5-era surface that matter if your starting point is MongoDB 4.0.

## High-priority features

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| Stable API envelope | `apiVersion`, `apiStrict`, `apiDeprecationErrors` on commands; shell `--apiVersion`, `--apiStrict`, `--apiDeprecationErrors` | `Very easily doable` | This is mostly parser, validation, metadata, and reply-advertising work. It does not require a new storage model. |
| `listCommands` metadata rename | `slaveOk` -> `secondaryOk`, add API-version metadata | `Very easily doable` | Mostly reply-shape and metadata exposure work. |
| `hello` compatibility path | `hello`, shell helper fallback behavior | `Very easily doable` | This is wire/API compatibility, not a storage or query-engine feature. |

## Data-model and data-manipulation features

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| Hidden indexes | `createIndexes.hidden`, `collMod.index.hidden`, `listIndexes.hidden` | `Very easily doable` | This is a contained index-catalog and planner-visibility feature. It needs plan cache invalidation and planner filtering, but it does not require a new storage format. |
| `let` variables for writes | `delete.let`, `update.let` | `Needs a lot of development` | You need expression binding to flow through write-command parsing, match/update execution, and explain/error behavior. This is not a one-line parser backport. |
| `runtimeConstants` on writes | `delete.runtimeConstants`, `update.runtimeConstants` | `Needs a lot of development` | Same reason as `let`: it reaches expression evaluation and command execution plumbing, not just command parsing. |
| Time-series collections | `create.timeseries`, `collMod.timeseries`, time-series-aware `listIndexes` / query rewrites | `Very hard to do` | This is a new logical data model built around bucket collections, translated writes, translated indexes, translated queries, and special catalog handling. This is far beyond “new command fields.” |
| Clustered collections / clustered TTL behavior | `create.clusteredIndex`, clustered `expireAfterSeconds` handling | `Very hard to do` | This changes collection layout assumptions and catalog semantics. On a 4.0 base, this is close to an architecture port, not a feature flag. |

## Things that look big in the v5 diff but should not drive the roadmap

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| Typed command rewrites of existing CRUD commands | `find`, `aggregate`, `drop`, `dropIndexes`, `listIndexes`, `findAndModify`, etc. becoming IDL-described | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | In many cases this is a parser/schema cleanup, not a new capability. If 4.0 already has the behavior, you do not need to re-implement it just because the later branch expresses it in IDL. |
| Role/user/auth command re-expression | `createRole`, `createUser`, `updateRole`, `updateUser`, `rolesInfo`, `usersInfo` | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | The important v5 security addition for your scope is the Stable API envelope. Most of the auth command additions in the diff are not new data-model features. |
| Sharding and resharding command families | `_configsvr*`, `_shardsvr*`, `reshardCollection`, etc. | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | Outside your stated scope. |

## Extra 4.0-only gap you should not miss

Because you are starting from 4.0 rather than 4.4, there are indexing features that are easy to overlook if you only read the v4.4 -> v8 diff.

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| Wildcard indexes | `createIndexes` with key patterns like `{"$**": 1}` and `wildcardProjection` | `Needs a lot of development` | This is a real new index access method with key generation, validation, planner integration, and catalog behavior. It is still much more contained than time-series or queryable encryption, but it is not a trivial parser backport. |

## Suggested implementation order for the v5 slice

1. Stable API fields and shell flags.
2. `hello` / `listCommands` compatibility.
3. Hidden indexes.
4. `let` / `runtimeConstants`.
5. Wildcard index parity if your fork needs it.
6. Time-series and clustered collections only if they are business-critical.
