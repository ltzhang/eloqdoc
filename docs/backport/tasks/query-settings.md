# Query settings (full)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 3 | 8.0 | 2–3 months |

## Scope

A new query-planner influence mechanism that lets operators set persistent settings (index hints, plan rejection) for query shapes without modifying application code.

```js
db.adminCommand({
  setQuerySettings: { /* shape descriptor */ },
  settings: {
    indexHints: [{ allowedIndexes: ["myIdx"] }],
    queryFramework: "classic",        // or "sbe"
    reject: false                     // can be set true to reject this shape entirely
  }
})

db.adminCommand({removeQuerySettings: { /* shape descriptor */ }})
```

Settings persist in a `system.querySettings` collection and survive restart.

Also includes:
- `find/aggregate/distinct.querySettings` request field — internal echo for cluster propagation.
- `$queryStats` aggregation stage — reports per-shape stats including any settings applied.

## Why it's Tier 3

1. **Query shape hashing.** Compute a stable hash of the query's structural shape (predicates, sort, projection — not values). Must be deterministic across server restarts and stable across non-semantic differences.
2. **Persistent storage.** Settings persist in `system.querySettings` and are loaded into memory on startup.
3. **Planner integration.** Every query must look up settings by shape hash before plan enumeration; settings can prune candidates, force a specific index, or reject the query.
4. **Catalog invalidation.** When settings change, the in-memory cache must be invalidated.
5. **Authorization.** New `setQuerySettings` action; only privileged users can modify.
6. **`$queryStats`** is a sibling project (the consumer side) — also Tier 3 because it requires the stats subsystem.

## Phasing

1. **Query shape hashing.** Standalone library; can be unit-tested without the planner. ~3 weeks.
2. **Persistence layer.** `system.querySettings` collection + in-memory cache + invalidation. ~2 weeks.
3. **Commands.** `setQuerySettings`/`removeQuerySettings`/`listQuerySettings`. ~2 weeks.
4. **Planner integration.** Hook into `getExecutor` to look up settings. ~3 weeks.
5. **Reject semantics.** Distinct error for shape-rejected queries. ~1 week.
6. **`$queryStats` consumer.** Stats subsystem reads settings and emits them in stats output. ~3 weeks.

## Dependencies

- **Blocked by:** none.
- **Blocks:** `$queryStats` (depends on this); future query-planner work that wants per-shape behavior.

## EloqDoc-specific considerations

- **Single-node simplification.** Stock MongoDB has cluster-wide propagation of query settings via the config server. EloqDoc's single-node model means settings are local — no cluster propagation. Simpler.
- **Plan-cache interaction.** The plan cache is keyed by a similar but distinct shape concept. Decide how query-settings shape and plan-cache shape interact (typically: query-settings shape is coarser, takes precedence).

## Recommendation

Tier 3 — start only when there's a clear request from operators. Most production deployments use index hints in application code, which already works in EloqDoc 4.0.3.

## Notes from source analyses

`analysis_gpt5.5/feature-backport-evaluation.md`: "Query shape hashing, persistent settings, planner hooks, validation, authorization." `analysis_cc/forward_compat_eval.md` rates Tier 3 explicitly. `analysis_gpt5.4` agrees and notes most modern Atlas features depend on this — but EloqDoc isn't Atlas.
