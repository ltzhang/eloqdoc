# v8 Backport Evaluation For A 4.0 Base

For your scope, v8 is where the backport cost becomes sharply bimodal: a few things are incremental extensions of earlier work, while the rest assume major newer subsystems.

## High-value user-facing additions

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| Search index command family | `createSearchIndexes`, `listSearchIndexes`, `updateSearchIndex`, `dropSearchIndex`, `replicateSearchIndexCommand` | `Very hard to do` | This assumes the Atlas Search / `mongot` style architecture. On a 4.0 base without that subsystem, these commands are mostly shells over a missing backend. |
| Structured-encryption cleanup | `cleanupStructuredEncryptionData` | `Very hard to do` | Same dependency chain as the earlier queryable-encryption work. |
| Query settings | `setQuerySettings`, `removeQuerySettings`, `find.querySettings`, `aggregate.querySettings`, `distinct.querySettings` | `Very hard to do` | This depends on a much newer query-planning and query-shape management model than 4.0 has. |
| `bulkWrite` extensions | `bulkWrite.let`, `bulkWrite.errorsOnly`, original-query/original-collation fields | `Needs a lot of development` | If you already backport `bulkWrite` from v7, these are an incremental extension. If you have not, this still sits behind the same big command effort. |
| `distinct.hint` | `distinct.hint` | `Very easily doable` | Compared with the rest of v8, this is a straightforward parser and planner plumbing enhancement. It is a good “small compatibility win.” |

## Features to deprioritize

| Feature | Commands / Flags | Rating | Why |
|---|---|---|---|
| Query-stats metric fields | `includeQueryStatsMetrics` on `find`, `aggregate`, `getMore` | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | Observability feature, not central to application-visible data behavior. |
| `compact`, `autoCompact`, `lockInfo`, FTDC/xray logging commands | command family | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | Operational tooling and storage/diagnostic behavior. |
| unsplittable/unshard/move-primary families | many v8 added commands | `Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue` | Outside your target scope. |

## Practical take

The most realistic v8 user-facing additions to take seriously are:

1. `distinct.hint` as a small compatibility improvement.
2. `bulkWrite` extensions, but only after the v7 `bulkWrite` base exists.

The v8 search and query-settings surfaces should be treated as architecture projects, not as normal forward-compatibility backports.
