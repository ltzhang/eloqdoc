# Tier 3 — Hard without Significant Refactor

**Definition:** Each task is a multi-sprint project introducing a new subsystem, requiring deep storage-engine changes, or depending on external services. These features can be implemented on top of EloqDoc 4.0.3 but the implementation is large enough to warrant treating it as its own project — typically 1–6 engineer-months per task.

**Total tasks:** 9. Aggregate effort: ~3–6 engineer-years total. **Don't tackle these in parallel without dedicated owners.**

Each Tier 3 spec below is **scope+phasing only** — the actual design doc is left as the first deliverable of the project, not pre-written here. The task file's job is to:
- Establish what would be in/out of scope
- Identify the architectural risks
- Suggest a phasing
- Identify the dependencies on Tier 1/Tier 2 work
- Quote the source analyses' positions

When prioritizing a Tier 3 task, the team should commission a separate design doc as project step 1.

## Tasks

### Storage / collection-type projects
1. [`time-series-collections.md`](./tasks/time-series-collections.md) — Full time-series parity (insert + find basic ⟶ updates/deletes/secondary indexes/geo). Largest single Tier 3 effort.
2. [`clustered-collections.md`](./tasks/clustered-collections.md) — Full clustered-collection semantics. Smaller if Data Substrate already orders by `_id`; larger otherwise.
3. [`compound-wildcard-indexes.md`](./tasks/compound-wildcard-indexes.md) — Builds on Tier 2 wildcard indexes with major planner work.
4. [`change-streams-pre-post-image.md`](./tasks/change-streams-pre-post-image.md) — Capture and serve pre/post images. Requires Data Substrate event hooks.

### Encryption project
5. [`queryable-encryption-fle2.md`](./tasks/queryable-encryption-fle2.md) — FLE2 / Queryable Encryption — state collections, encrypted query rewriting, key vault, compaction/cleanup. Multi-month, may need a security expert.

### Query-planner / optimizer projects
6. [`query-settings.md`](./tasks/query-settings.md) — Full query settings infrastructure: shape hashing, persistent settings, planner reject. Includes `$queryStats` consumer side.

### Search / vector projects
7. [`search-vector-search.md`](./tasks/search-vector-search.md) — `$search`/`$searchMeta`/`$vectorSearch`/`$rankFusion`/`$scoreFusion` plus search-index management commands. Requires deciding on a search backend.

### Auth project
8. [`oidc-auth.md`](./tasks/oidc-auth.md) — OIDC (OAuth 2.0) authentication. Self-contained; ~2 engineer-months.

### Stable API project (escalated form of the Tier 1 stub)
9. Stable API full strict-mode enforcement — covered as the escalation path in [`stable-api-envelope.md`](./tasks/stable-api-envelope.md). Listed here for visibility.

## Sequencing recommendation

1. If a single Tier 3 must be picked, **OIDC authentication** is the smallest and most self-contained.
2. **Time-series basic** (insert + find + TTL) per `analysis_cc`'s phasing is reachable in ~6 weeks if the team commits — but full parity is a much larger investment.
3. **FLE2** and **search/vector** are the hardest and require external dependencies (crypto expertise / search backend service). Prioritize only with explicit product demand.
4. **Compound wildcard indexes** depends on the Tier 2 wildcard implementation; sequence after.

## What's deliberately not in Tier 3

- **Sharding internals.** Already Tier 4.
- **Replication coordination.** Already Tier 4.
- **Columnstore indexes.** Tier 4 — deprecated upstream in 8.0; not worth the investment.
- **Query analyzer / sampling.** Tier 4 — telemetry feature, low value.

## Index of all Tier 3 tasks

| # | Task | Risk area | Effort | Spec |
| - | ---- | --------- | ------ | ---- |
| 1 | Time-series (full parity) | Storage | 3–6 mo | [time-series-collections.md](./tasks/time-series-collections.md) |
| 2 | Clustered collections (full) | Storage | 1–3 mo | [clustered-collections.md](./tasks/clustered-collections.md) |
| 3 | Compound wildcard indexes | Planner | 1–2 mo | [compound-wildcard-indexes.md](./tasks/compound-wildcard-indexes.md) |
| 4 | Change-stream pre/post images | Event hooks | 1–2 mo | [change-streams-pre-post-image.md](./tasks/change-streams-pre-post-image.md) |
| 5 | Queryable Encryption (FLE2) | Crypto | 4–6 mo | [queryable-encryption-fle2.md](./tasks/queryable-encryption-fle2.md) |
| 6 | Query settings (full) | Planner | 2–3 mo | [query-settings.md](./tasks/query-settings.md) |
| 7 | Search/vector ($search, etc.) | External service | 2–4 mo + backend | [search-vector-search.md](./tasks/search-vector-search.md) |
| 8 | OIDC authentication | Auth | 2 mo | [oidc-auth.md](./tasks/oidc-auth.md) |
| 9 | Stable API strict enforcement | IDL framework | 2–3 mo | (escalation in [stable-api-envelope.md](./tasks/stable-api-envelope.md)) |
