# EloqDoc Backport Plan — MongoDB 4.0.3 → 8.0 Data API Forward Compatibility

**Status:** Implementation-ready  
**Audience:** EloqDoc engineering team  
**Source baseline:** EloqDoc, fork of MongoDB 4.0.3 (`eloqdoc/`)  
**Target API surface:** MongoDB 8.0 (data-model and data-manipulation API only)

---

## What this folder is

A canonical, EloqDoc-grounded plan to bring EloqDoc's wire/API surface from MongoDB 4.0.3 toward MongoDB 8.0. It aggregates and reconciles three independent prior analyses (`analysis_cc/`, `analysis_gpt5.4/`, `analysis_gpt5.5/`) and grounds every recommendation in a verified read of the EloqDoc source tree.

**Scope:** Data-model and data-manipulation APIs only — index types, aggregation stages/expressions/accumulators, query/write command flags, write commands (`bulkWrite`), security envelope (Stable API), client-visible mongosh surface.

**Out of scope:** Sharding internals, replication coordination, transaction internals (handled by Data Substrate), oplog mechanics, FTDC, query analyzer/sampling. See [`tier-4-deferred.md`](./tier-4-deferred.md) for the explicit list and rationale.

---

## How to use this folder

1. **Start with [`baseline.md`](./baseline.md)** — what EloqDoc 4.0.3 *already* supports, plus the seven extension patterns every backport relies on. Engineers should read this once before picking up any task.

2. **Skim [`version-evolution.md`](./version-evolution.md)** — a one-screen-per-version delta table for 5.0 / 6.0 / 7.0 / 8.0. Every row links to a task spec or to `tier-4-deferred.md`.

3. **Pick a tier and a task:**
   - [`tier-1-simple.md`](./tier-1-simple.md) — Simple. Parser/expression/stage-alias work. Days each. ~16 tasks. Start here for fast wins.
   - [`tier-2-effort.md`](./tier-2-effort.md) — Needs Effort & Design. Planner/executor/catalog work. Weeks each. ~17 tasks. Highest-value: `bulkWrite`, hidden indexes, wildcard indexes.
   - [`tier-3-hard.md`](./tier-3-hard.md) — Hard without Significant Refactor. New subsystem / architecture work. Months each. ~9 tasks. Treat each as a multi-sprint project.

4. **Each `tasks/<feature>.md` is implementation-ready.** It tells you which extension macro to use, which files to create vs. modify, what the dependencies are, and EloqDoc-specific gotchas (no oplog, no global locking, Data Substrate storage). Use it as the starter spec for a PR or sprint.

---

## One-page summary

| Tier | Definition | Task count | Aggregate effort |
| ---- | ---------- | ---------- | ----------------- |
| **1 — Simple** | Parser-only, pure expressions, stage aliases. Single PR each. | ~16 | ~6–10 engineer-weeks total |
| **2 — Needs Effort & Design** | Planner / executor / catalog work. Single sprint each. | ~17 | ~6–9 engineer-months total |
| **3 — Hard w/o Significant Refactor** | New subsystem (FLE2, time-series full parity, search, columnstore, query settings). Multi-sprint projects. | ~9 | ~3–6 engineer-years total |
| **4 — Deferred** | Out of scope for this initiative. | n/a | n/a |

**Highest-leverage targets** (consensus across all three source analyses):
1. **`bulkWrite` command** (Tier 2) — single biggest client-compat win. Required for mongosh 8.0 `db.getMongo().bulkWrite()`.
2. **Stable API envelope** (Tier 1) — required for any modern driver running in `apiStrict: true` mode.
3. **Pure-expression batch** (Tier 1) — trig, regex, trim, conversions, bit-ops, date arithmetic. Cheap and high coverage.
4. **Hidden + wildcard indexes** (Tier 2) — enables modern index strategies; wildcard is a 4.2 catch-up that never landed.
5. **`$merge` + `$unionWith` + `$documents`** (Tier 1–2) — unblocks pipelines used by every modern aggregation.

---

## Source analyses (for cross-reference)

| Source | Strength | Where the task specs cite it |
| ------ | -------- | ---------------------------- |
| [`analysis_cc/forward_compat_eval.md`](../analysis_cc/forward_compat_eval.md) | Most detailed file-by-file implementation notes; explicit phasing for time-series and FLE2. | Files-to-create and phasing guidance. |
| [`analysis_gpt5.5/feature-backport-evaluation.md`](../analysis_gpt5.5/feature-backport-evaluation.md) | Cleanest decision matrix; clear "ignore vs. needs dev" calls. | Tier classification when sources disagree. |
| [`analysis_gpt5.5/raw/command-deltas.md`](../analysis_gpt5.5/raw/command-deltas.md) | Source-of-truth delta lists per version. | Field-level coverage in `version-evolution.md`. |
| [`analysis_gpt5.4/forward-compat/feature-families.md`](../analysis_gpt5.4/forward-compat/feature-families.md) | Sequencing intuition (which families to do together). | Within-tier ordering recommendations. |

When source analyses **disagree** on a feature's tier, the task file calls out the divergence in its `## Notes from source analyses` section and explains the chosen tier. Notable disagreements: `$unionWith` (`cc`=Tier 1, others=Tier 2), `commitQuorum` (full Tier 2 vs. parser-only Tier 1 escape), time-series (Tier 2 basic vs. Tier 3 full).

---

## Glossary

- **EloqDoc-specific** — refers to deviations from stock MongoDB 4.0.3 (no oplog-based replication, no WiredTiger, `eloq_locker_noop.h` no-op locking, Data Substrate storage). Each task spec flags what this means for the implementation.
- **Tier** — implementation-effort classification, defined per the user's classes: Simple / Needs Effort & Design / Hard w/o Significant Refactor.
- **Extension pattern** — one of seven canonical ways to add a feature to the MongoDB 4.0.3 codebase (see [`baseline.md`](./baseline.md)).

---

## Conventions

- All EloqDoc paths are relative to `eloqdoc/` (e.g. `src/mongo/db/pipeline/document_source.h`).
- Effort estimates assume one experienced C++ engineer familiar with the MongoDB code style. Add 30–50% for first-time contributors.
- Acceptance criteria reference jstest paths under `tests/jstests/` — many tests will need to be adapted from upstream MongoDB and made EloqDoc-compatible.
- Where a task is a parser-only "compatibility" stub (accept the field, ignore the semantics), it is marked **(stub)** in the title.
- Whenever a backport intentionally differs from MongoDB semantics, document the difference explicitly in the task spec and implementation notes. State what MongoDB does, what EloqDoc does instead, and why that tradeoff is acceptable for EloqDoc.
