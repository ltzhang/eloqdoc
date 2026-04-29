# `analyze` command

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 7.0 | 2–3 weeks |

## Scope

The `analyze` command computes and persists statistics (in particular: histograms) about a collection's fields, used by the query optimizer. Stock MongoDB 7.0 introduced this with optional `key` (single field), `sampleRate`, and `sampleSize`.

```js
db.runCommand({analyze: "c"})                                       // all "appropriate" fields
db.runCommand({analyze: "c", key: "category", sampleSize: 10000})   // one field
db.runCommand({analyze: "c", key: "meta.score"})                    // one dotted field path
db.runCommand({analyze: "c", sampleRate: 0.1})                      // 10% sample
```

Statistics are persisted in the `system.statistics.<collection>` system namespace.

## Extension pattern

Pattern A (add a command). The work is in three layers:

1. **Sampling** — read N documents from the collection (use existing `$sample` infrastructure).
2. **Histogram construction** — for each scalar field, build an equi-depth histogram (or count-min sketch for high-cardinality strings).
3. **Persistence** — write to `system.statistics.<collection>`. Subsequent queries' optimizer reads these stats.

```cpp
// src/mongo/db/commands/analyze.cpp
class CmdAnalyze : public BasicCommand {
public:
    CmdAnalyze() : BasicCommand("analyze") {}
    bool run(...) override {
        // 1. Sample.
        // 2. Build histograms.
        // 3. Persist to system.statistics.<coll>.
        // 4. Reply with summary.
    }
};
```

## Files to create

- `src/mongo/db/commands/analyze.cpp`
- `src/mongo/db/query/stats/histogram.{cpp,h}`
- `src/mongo/db/query/stats/stats_catalog.{cpp,h}` — read/write the system namespace

## Files to modify

- `src/mongo/db/commands/SConscript`
- (optional) `src/mongo/db/query/cost_estimator.{cpp,h}` — consume histograms in cost estimation. Without this, `analyze` produces stats that nobody reads — the value is in the read side, but parser-only is a useful first step.

## Dependencies

- **Blocks:** future query-optimizer work that depends on histograms.
- **Blocked by:** none.

## EloqDoc-specific considerations

- **The optimizer side is the real value.** Without consumption in cost estimation, this command is just a stats-collector that nobody reads. Decide upfront whether to do the full feature (write + read) or the parser-only stub (writes stats; nothing consumes). All three analyses note this caveat.
- **Sampling implementation.** Reuse `$sample` (`document_source_sample.cpp`); don't reimplement.
- **Storage.** `system.statistics.<coll>` is a regular collection in EloqDoc. No special handling.

## Acceptance criteria

- [x] `db.runCommand({analyze: "c"})` succeeds and creates entries in `system.statistics.c`.
- [x] Stats include cardinality, min/max, histogram for scalar fields.
- [x] `sampleSize`/`sampleRate` honored.
- [x] Re-running replaces previous stats.
- [x] Reply summarizes fields analyzed and rows sampled.
- (If optimizer integration done) Cost estimates for `find` queries with histograms differ from estimates without.
- **Test entry point:** `tests/jstests/eloq_basic/analyze.js`.

## Implementation notes

Initial EloqDoc support lands as a compact `BasicCommand` in `src/mongo/db/commands/analyze.cpp`. It scans the target collection through the local direct client, analyzes top-level scalar fields by default, and persists one document per field into `system.statistics.<collection>`. When `key` is supplied, it may name either a top-level field or a dotted field path such as `meta.score`. Each stats document records the source namespace, field name, sampled document count, observed value count, cardinality, min/max, and a basic value-frequency histogram.

The initial sampling implementation honors `sampleSize` and `sampleRate` by limiting the number of documents read from the collection. It is deterministic rather than random and does not yet reuse `$sample`; that is an intentional first-slice simplification for the parser + stats-persistence milestone. The query optimizer does not consume these stats yet.

Because MongoDB 4.0 treats most `system.*` collections as protected client namespaces, this backport also allows `system.statistics.*` as a legal client system namespace so the command can replace and write stats through the existing write path.

## Notes from source analyses

`analysis_cc/forward_compat_eval.md`: "Minimal sampling stats possible; full value depends on optimizer stats/catalog." `analysis_gpt5.5`: "Implementable but optimizer must consume." `analysis_gpt5.4`: "Defer optimizer integration to Tier 3 unless your team owns the cost model."

**Recommendation:** ship parser + stats persistence as Tier 2 (~2 weeks). Defer optimizer consumption until there's clear demand and ownership.
