# v6 Backport Evaluation For A 4.0 Base

v6 is not the big parser break. For your scope it is mostly a feature-surface expansion release.

## High-value user-facing additions

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| `encryptedFields` at collection creation | `create.encryptedFields` | `Very hard to do` | This is the entry point into queryable encryption / FLE2. It is not just a new collection option; it drags in encrypted metadata, auxiliary state, special indexes, and query rewrite behavior. |
| `changeStreamPreAndPostImages` collection option | `create.changeStreamPreAndPostImages` | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | User-visible, but not in your target area of data model / data manipulation APIs. It is tied to change stream semantics rather than core CRUD/data modeling. |
| `listIndexes.includeIndexBuildInfo` | `listIndexes.includeIndexBuildInfo` | `Very easily doable` | Mostly reply-shape and build-state reporting. Useful for compatibility, but not an engine rewrite. |
| `createIndexes.collectionUUID`, `drop.collectionUUID`, `find.collectionUUID`, similar safety fields | multiple commands | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | These are mostly internal correctness and routing/safety fields, not primary user-facing capabilities. |

## Encryption-related features

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| encryption-information plumbing on reads and writes | `find.encryptionInformation`, `aggregate.encryptionInformation`, `findAndModify.encryptionInformation`, `count.encryptionInformation` | `Very hard to do` | This is the same architectural family as `encryptedFields`: it assumes a queryable-encryption execution model that 4.0 does not have. |
| structured-encryption compaction | `compactStructuredEncryptionData` | `Very hard to do` | This is not independently implementable without the underlying queryable-encryption state model. |

## Shell and flag changes

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| `crashOnInvalidBSONError` shell option | shell CLI flag | `Very easily doable` | Pure shell/client behavior. |
| removal of legacy read/write compatibility shell modes | `readMode`, `writeMode`, `rpcProtocols`, `useLegacyWriteOps` | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | This is a cleanup decision, not a must-have forward-compat feature for your server fork. |

## v6 features that look large in diffs but are not worth prioritizing

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| `collMod` refactoring in IDL | `collMod` diff noise | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | Much of the apparent field churn in the generated diff is from the command being restructured, not from core user-visible semantics disappearing. |
| cluster-parameter and write-block mode commands | `setClusterParameter`, `setUserWriteBlockMode`, related internal commands | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | Outside your target scope. |

## Suggested implementation order for the v6 slice

1. `listIndexes.includeIndexBuildInfo` if you want low-cost wire compatibility.
2. Defer all queryable-encryption features unless they are a top-level product requirement.
3. Treat most `collectionUUID` and envelope additions as optional compatibility work, not roadmap drivers.
