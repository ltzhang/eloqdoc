# Forward-Compatibility Backport Review

This folder answers a narrower question than the earlier API inventory:

> Starting from a MongoDB 4.0 codebase, which later user-facing data APIs are realistic to backport, which are expensive, and which should be ignored for now?

## Scope

Included:

- data model features
- data manipulation APIs
- new index options or new index families
- collection options that affect user-visible data behavior
- security- and compatibility-related command flags that affect client-visible behavior

Explicitly deprioritized:

- sharding
- replication
- transactions
- placement and metadata movement
- storage-engine internals unless they are unavoidable for a user-facing feature
- test-only, internal, and topology-only commands

## Important baseline caveat

The earlier branch-by-branch analysis used `v4.4` as the comparison baseline because that was the original review scope. Your target is `v4.0`.

That means the estimates in this folder are intentionally adjusted upward where `4.0` is missing features that were already present by `4.4`.

The biggest examples are:

- hidden indexes
- wildcard indexes / `wildcardProjection`

Those are called out here even though they do not show up cleanly as “new in v5+” in the earlier v4.4-based diff set.

## Rating scale

Every feature is marked as exactly one of:

1. `Very easily doable`
2. `Needs a lot of development`
3. `Very hard to do`
4. `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue`

## Recommended reading order

1. [v5-backport-evaluation.md](/home/lintaoz/work/mongo/analysis/forward-compat/v5-backport-evaluation.md)
2. [v6-backport-evaluation.md](/home/lintaoz/work/mongo/analysis/forward-compat/v6-backport-evaluation.md)
3. [v7-backport-evaluation.md](/home/lintaoz/work/mongo/analysis/forward-compat/v7-backport-evaluation.md)
4. [v8-backport-evaluation.md](/home/lintaoz/work/mongo/analysis/forward-compat/v8-backport-evaluation.md)
5. [feature-families.md](/home/lintaoz/work/mongo/analysis/forward-compat/feature-families.md)

## Quick summary

### Best candidates to implement first

- Stable API request fields and shell flags (`apiVersion`, `apiStrict`, `apiDeprecationErrors`)
- `listCommands` metadata compatibility (`secondaryOk`)
- hidden indexes
- small list/index command compatibility fields that are mostly parser or reply-shape changes

### Good candidates, but not “quick”

- `let` / `runtimeConstants` support across query and write commands
- `bulkWrite`
- wildcard index parity if your 4.0 fork does not already have it
- some capped-collection `collMod` improvements

### Features that want major architecture work

- time-series collections
- clustered collections / clustered TTL behavior
- queryable encryption and `encryptedFields`
- search indexes
- query settings / query stats plumbing
- unique-index state transitions through newer `collMod` paths

### Features to ignore for now

- sharding-only APIs
- migration / balancer / placement commands
- tenant and topology management
- internal `collectionUUID`, `sampleId`, `mirrored`, routing, and test-only fields unless you need wire compatibility for a specific client

## Source base

These docs are derived from the existing inventories under [analysis/api-review](/home/lintaoz/work/mongo/analysis/api-review), especially:

- [comparison.md](/home/lintaoz/work/mongo/analysis/api-review/comparison.md)
- [v5.md](/home/lintaoz/work/mongo/analysis/api-review/v5.md)
- [v6.md](/home/lintaoz/work/mongo/analysis/api-review/v6.md)
- [v7.md](/home/lintaoz/work/mongo/analysis/api-review/v7.md)
- [v8.md](/home/lintaoz/work/mongo/analysis/api-review/v8.md)
- the generated command diffs and inventories in `analysis/api-review/generated/`
