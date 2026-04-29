# Feature Families Across v5-v8

This file groups the later APIs by engineering shape instead of by release number.

## 1. Parser-and-flag compatibility

| Feature family | Versions | Rating | Notes |
|---|---|---|---|
| Stable API request fields and shell flags | v5+ | `Very easily doable` | Good early win. Mostly parser, validation, and metadata wiring. |
| `hello` / `listCommands` compatibility tweaks | v5+ | `Very easily doable` | Also a good early win. |
| Small reply-shape additions | v6+ / v8+ | `Very easily doable` | Examples: `listIndexes.includeIndexBuildInfo`, `distinct.hint`. |

## 2. Index feature work

| Feature family | Versions | Rating | Notes |
|---|---|---|---|
| Hidden indexes | v5-era practical target on a 4.0 base | `Very easily doable` | Contained catalog/planner feature. |
| Wildcard indexes | missing on 4.0 even if not “new in v5” | `Needs a lot of development` | New access method, but still much more contained than time-series or FLE2. |
| Unique-state transitions via `collMod` | v7+ | `Very hard to do` | Touches deep index semantics and transitional states. |
| Search indexes | v8 | `Very hard to do` | Depends on external/newer search architecture. |

## 3. Collection-model work

| Feature family | Versions | Rating | Notes |
|---|---|---|---|
| Time-series collections | v5+ | `Very hard to do` | New logical model, bucket collections, translated CRUD, translated indexes. |
| Clustered collections | v5+ | `Very hard to do` | Requires collection-layout and catalog assumptions that 4.0 does not share. |
| Capped collection refinement | v7+ | `Needs a lot of development` | Worth doing only if capped collections matter to you. |

## 4. Write/query API work

| Feature family | Versions | Rating | Notes |
|---|---|---|---|
| `let` / `runtimeConstants` | v5+ | `Needs a lot of development` | Expression and command plumbing work. |
| `bulkWrite` | v7+ / v8+ | `Needs a lot of development` | Large, but plausible on top of existing write commands. |
| Query settings | v8 | `Very hard to do` | Assumes much newer planner/query-shape management. |

## 5. Encryption family

| Feature family | Versions | Rating | Notes |
|---|---|---|---|
| `encryptedFields` collection option | v6+ | `Very hard to do` | Entry point into queryable encryption. |
| encryption-information request fields | v6+ | `Very hard to do` | Not meaningful without the full encryption architecture. |
| `compactStructuredEncryptionData` | v6+ | `Very hard to do` | Same dependency chain. |
| `getQueryableEncryptionCountInfo` | v7+ | `Very hard to do` | Same dependency chain. |
| `cleanupStructuredEncryptionData` | v8 | `Very hard to do` | Same dependency chain. |

## 6. Recommended roadmap

### First wave

- Stable API flags and request parsing
- `hello` / `listCommands`
- hidden indexes
- small parser/reply compatibility additions such as `distinct.hint`

### Second wave

- `let` / `runtimeConstants`
- `bulkWrite`
- wildcard indexes if needed for the product

### Third wave only if required

- capped collection refinement
- unique-index state transitions

### Treat as architecture projects

- time-series
- clustered collections
- queryable encryption
- search indexes
- query settings

## 7. Broad ignore rule

Anything in the earlier generated diffs that is primarily:

- `_configsvr*`
- `_shardsvr*`
- migration / balancer / chunk / placement work
- tenant topology management
- test-only commands
- command envelope telemetry fields

should stay in the `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` bucket for this project.
