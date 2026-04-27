# Search and vector search (`$search`, `$vectorSearch`, `$rankFusion`, `$scoreFusion`, search index commands)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 3 | 8.0 (server commands; mongosh helpers earlier) | 2–4 months + search-backend dependency |

## Scope

A family of features tied to MongoDB Atlas Search and Atlas Vector Search:

### Aggregation stages
- `$search` — full-text search (8.0).
- `$searchBeta` — earlier-version compatibility name.
- `$searchMeta` — Atlas Search metadata (faceting, etc.).
- `$vectorSearch` — vector / approximate nearest neighbor search.
- `$rankFusion`, `$scoreFusion` — combine results from multiple search stages by rank or score.
- `$listSearchIndexes` — pipeline form of the same-named command.

### Commands
- `createSearchIndexes`, `dropSearchIndex`, `updateSearchIndex`, `listSearchIndexes` — manage search-index definitions.

### Mongosh helpers
- `db.collection.createSearchIndex(es)`, `getSearchIndexes()`, `dropSearchIndex()`, `updateSearchIndex()`.

## Why it's Tier 3

These features require an **external search backend** — in stock MongoDB, this is `mongot`, a Lucene-based companion process. Without `mongot`:
1. The aggregation stages have nothing to dispatch to.
2. The index management commands are metadata-only stubs.
3. The mongosh helpers fail at the wire level.

EloqDoc has no equivalent search backend today.

## Architectural decision required

Before implementing, the team must decide on a search backend. Options:

1. **Embed Lucene-equivalent (Tantivy?, Apache Lucene C++ port).** Adds a major dependency but unlocks both `$search` and `$vectorSearch`. ~3 months for backend integration alone.
2. **Adopt OpenSearch / Elasticsearch as a sidecar.** Existing battle-tested backend; adds a deployment dependency. ~1 month for the bridge.
3. **Implement a lightweight FTS subset in-process.** Cheapest but less capable. May not scale.
4. **Defer entirely** until product demand is clear. **Recommended unless there's a specific RAG / semantic-search use case driving demand.**

## Phasing (assuming backend is chosen)

1. **Search index management commands.** Metadata-only stubs that delegate to the backend. ~3 weeks.
2. **`$search` stage.** Wire input pipeline through to backend; receive scored documents. ~4 weeks.
3. **`$vectorSearch` stage.** Same plumbing, different query shape. ~3 weeks.
4. **`$searchMeta` stage.** Faceting/aggregation metadata. ~2 weeks.
5. **`$rankFusion`/`$scoreFusion` stages.** Pure pipeline-level merge logic; can be implemented even without a search backend (consumes any input pipeline). ~2 weeks.
6. **Mongosh helpers.** Already exist in mongosh sources; just need to work against EloqDoc's commands.

## Dependencies

- **Blocked by:** Architectural decision on search backend (above).

## EloqDoc-specific considerations

- **No mongot.** EloqDoc would need to either bundle a search backend or integrate with one.
- **Vector search index storage.** ANN indexes are large; storage planning needed.
- **Network model.** If the search backend is a sidecar, the mongod-to-search-backend communication channel needs design.

## Recommendation

**Defer indefinitely** unless there's a concrete product demand for full-text or vector search. The implementation is large, the backend dependency is significant, and most use cases can be served by Atlas (for users who need it) or by integrating with a separate search service at the application level.

If demand exists: option 5 (`$rankFusion`/`$scoreFusion`) is the only piece implementable without a backend — those are pure pipeline-merge stages and could be a Tier 2 task on their own. They take inputs from other stages (could be regular `$match` results) and merge.

## Notes from source analyses

All three analyses rate Tier 3 ("very hard"). `analysis_gpt5.5/feature-backport-evaluation.md`: "Real behavior requires mongot/search service. Metadata-only stubs are possible but not useful for query execution." `analysis_cc` agrees and is the most explicit: "Skip entirely unless a search backend is committed to."

`$rankFusion`/`$scoreFusion` as a Tier 2 carve-out is supported by `analysis_gpt5.5` but notes it provides limited value without `$search`/`$vectorSearch` to fuse.
