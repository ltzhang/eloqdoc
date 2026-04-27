# Tier 2 — Needs Effort & Design

**Definition:** Implementable on EloqDoc's existing architecture, but require substantive work in the planner, executor, catalog, or command framework. Each task is a sprint-sized effort — typically 1–6 weeks for one engineer. Multiple tasks can run in parallel; some have dependencies (called out in each spec).

**Total tasks:** 17. Aggregate effort: ~6–9 engineer-months if done sequentially.

## Recommended phasing

### Phase 2.A — "Index improvements" (~6–8 weeks)
The two index-related tasks are independent and high-leverage. Wildcard indexes are also a 4.2 catch-up that EloqDoc never received.

1. [`hidden-indexes.md`](./tasks/hidden-indexes.md) — `hidden: true` flag on indexes, planner exclusion, plan-cache invalidation, shell helpers
2. [`wildcard-indexes.md`](./tasks/wildcard-indexes.md) — `$**` key generator, `wildcardProjection`, planner integration
3. [`prepareunique-workflow.md`](./tasks/prepareunique-workflow.md) — convert-to-unique workflow

### Phase 2.B — "Command flags & syntax" (~4 weeks, parallelizable)
4. [`let-runtime-constants.md`](./tasks/let-runtime-constants.md) — `let` and `runtimeConstants` on find/update/delete/aggregate
5. [`update-delete-hint.md`](./tasks/update-delete-hint.md) — `hint` on update/delete, planner enforcement
6. [`allowdiskuse-find.md`](./tasks/allowdiskuse-find.md) — `find.allowDiskUse` (blocking sort spill)
7. [`pipeline-update-syntax.md`](./tasks/pipeline-update-syntax.md) — pipeline-form update (`update.u: [{$set:...}]`)

### Phase 2.C — "Aggregation stages" (~10 weeks, parallelizable across engineers)
8. [`document-source-merge.md`](./tasks/document-source-merge.md) — `$merge` (write from pipeline; the most-requested 5.0 stage)
9. [`document-source-unionwith.md`](./tasks/document-source-unionwith.md) — `$unionWith` (nested pipeline union)
10. [`document-source-fill.md`](./tasks/document-source-fill.md) — `$fill`
11. [`document-source-densify.md`](./tasks/document-source-densify.md) — `$densify`
12. [`setwindowfields.md`](./tasks/setwindowfields.md) — `$setWindowFields` (largest single-stage effort)
13. [`window-operators.md`](./tasks/window-operators.md) — window-only operators ($rank, $expMovingAvg, etc.)
14. [`accumulator-n-value-family.md`](./tasks/accumulator-n-value-family.md) — N-value accumulators
15. [`percentile-median-accumulators.md`](./tasks/percentile-median-accumulators.md) — t-digest accumulators
16. [`document-source-listcatalog.md`](./tasks/document-source-listcatalog.md) — `$listCatalog`

### Phase 2.D — "High-leverage commands" (~6–8 weeks)
17. [`bulkwrite-command.md`](./tasks/bulkwrite-command.md) — `bulkWrite` (the single highest-value Tier 2 task per all three analyses)
18. [`analyze-command.md`](./tasks/analyze-command.md) — `analyze` (basic histogram statistics)
19. [`pre-post-images-collection-option.md`](./tasks/pre-post-images-collection-option.md) — collection option, parser+catalog (full semantics deferred)

## Index of all Tier 2 tasks

| # | Task | Pattern | Effort | Spec |
| - | ---- | ------- | ------ | ---- |
| 1 | Hidden indexes | catalog + planner | 2 w | [hidden-indexes.md](./tasks/hidden-indexes.md) |
| 2 | Wildcard indexes (4.2 catch-up) | E (index) | 4–6 w | [wildcard-indexes.md](./tasks/wildcard-indexes.md) |
| 3 | `prepareUnique` workflow | catalog + write path | 2–3 w | [prepareunique-workflow.md](./tasks/prepareunique-workflow.md) |
| 4 | `let` / `runtimeConstants` | command framework + executor | 1–2 w | [let-runtime-constants.md](./tasks/let-runtime-constants.md) |
| 5 | `update.hint`, `delete.hint` | planner enforcement | 1 w | [update-delete-hint.md](./tasks/update-delete-hint.md) |
| 6 | `find.allowDiskUse` | executor | 2–3 w | [allowdiskuse-find.md](./tasks/allowdiskuse-find.md) |
| 7 | Pipeline-form update | update parser + executor | 1 w | [pipeline-update-syntax.md](./tasks/pipeline-update-syntax.md) |
| 8 | `$merge` | B + executor | 3 w | [document-source-merge.md](./tasks/document-source-merge.md) |
| 9 | `$unionWith` | B + nested execution | 2 w | [document-source-unionwith.md](./tasks/document-source-unionwith.md) |
| 10 | `$fill` | B + sorted-stream | 2 w | [document-source-fill.md](./tasks/document-source-fill.md) |
| 11 | `$densify` | B + sorted-stream | 2 w | [document-source-densify.md](./tasks/document-source-densify.md) |
| 12 | `$setWindowFields` | B + window executor | 4–6 w | [setwindowfields.md](./tasks/setwindowfields.md) |
| 13 | Window operators | D + window helpers | 2 w | [window-operators.md](./tasks/window-operators.md) |
| 14 | N-value accumulators | D | 1–2 w | [accumulator-n-value-family.md](./tasks/accumulator-n-value-family.md) |
| 15 | `$percentile` / `$median` | D + t-digest | 2 w | [percentile-median-accumulators.md](./tasks/percentile-median-accumulators.md) |
| 16 | `$listCatalog` | B + catalog reader | 1 w | [document-source-listcatalog.md](./tasks/document-source-listcatalog.md) |
| 17 | `bulkWrite` (incl. 8.0 enhancements) | A + dispatch | 4–6 w | [bulkwrite-command.md](./tasks/bulkwrite-command.md) |
| 18 | `analyze` | A + sampling | 2–3 w | [analyze-command.md](./tasks/analyze-command.md) |
| 19 | Pre/post-image collection option | G (collection option) | 1 w | [pre-post-images-collection-option.md](./tasks/pre-post-images-collection-option.md) |

## Critical-path notes

- **`bulkWrite` is the highest-value single deliverable** per all three source analyses. Mongosh 8.0 `db.getMongo().bulkWrite()` requires it. If you have only 1–2 engineer-months for Tier 2, do this and a handful of Tier 1.
- **`$setWindowFields` is the largest single-stage task** (4–6 weeks alone). Engineers tackling it will benefit from finishing N-value and percentile accumulators first since the same accumulator infrastructure is reused.
- **Wildcard indexes are a 4.2 catch-up.** They were never in EloqDoc; this is *not* a forward-port from 5.0+ but a missing-feature fix.
- **`$merge` and `$unionWith` together** unblock most non-trivial 5.0+ aggregation pipelines.
