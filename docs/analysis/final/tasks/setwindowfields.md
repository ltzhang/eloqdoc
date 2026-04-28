# `$setWindowFields` aggregation stage

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 5.0 / 7.0 (full) | 4–6 weeks (largest single Tier 2 task) |

## Scope

Window-function aggregation: compute aggregates over a sliding "window" of adjacent documents within a partition, without collapsing the partition into a single document.

**Checkpoint status:** buffered executor slices landed. EloqDoc supports `partitionBy`,
`sortBy`, `documents` windows, numeric `range` windows over a single numeric sort key,
and standard output operators `$sum`, `$avg`, `$count`, `$min`, `$max`, `$first`, and
`$last`. It also supports position-based window operators `$documentNumber`, `$rank`,
`$denseRank`, and `$shift`, plus numeric `$expMovingAvg`. Date range units,
spill/removable optimizations, and math-heavy window-only operators remain deferred.

```js
db.sales.aggregate([
  { $setWindowFields: {
      partitionBy: "$store",
      sortBy: { date: 1 },
      output: {
          rollingTotal: { $sum: "$qty", window: { range: [-7, 0], unit: "day" } },
          rank:         { $rank: {} },
          movingAvg:    { $avg: "$price", window: { documents: [-3, "current"] } }
      }
  }}
])
```

**Components:**
- **Partitioning** by one or more fields (optional).
- **Sort** within each partition (optional but required for some operators).
- **Window specification:** `documents: [from, to]` (positional) or `range: [from, to], unit: <unit>` (value-based).
- **Operators:** standard accumulators ($sum, $avg, etc.) PLUS window-only operators (`$rank`, `$expMovingAvg`, etc. — see [`window-operators.md`](./window-operators.md)).

## Extension pattern

Pattern B + significant new executor infrastructure.

```cpp
class DocumentSourceSetWindowFields final : public DocumentSource {
    GetNextResult getNext() final;
private:
    std::vector<FieldRef> _partitionBy;
    BSONObj _sortBy;
    std::vector<WindowOutputSpec> _outputs;          // one per output field
    PartitionIterator _partitionIter;                // streams docs partition-by-partition
};

class WindowExecutor {
    // Per output field:
    //  - Maintain the accumulator state.
    //  - Maintain a sliding window over the partition.
    //  - On each document: update accumulator with newcomers / departures.
};
```

The window executor must support:
1. **Sliding documents-window** — efficient incremental update (add new doc, remove leaving doc).
2. **Sliding range-window** — efficient binary search to find window boundaries.
3. **Removable accumulators** — accumulators that support remove() (sum, avg, count). Non-removable accumulators (min, max) require recomputation or a more sophisticated structure (deque).
4. **Window-only operators** that don't fit the standard accumulator interface (rank, denseRank, documentNumber, expMovingAvg).

## Files to create

- `src/mongo/db/pipeline/document_source_set_window_fields.{cpp,h}`
- `src/mongo/db/pipeline/window_executor.{cpp,h}`
- `src/mongo/db/pipeline/window_function_*.cpp` — one per window operator (see [`window-operators.md`](./window-operators.md))
- `src/mongo/db/pipeline/partition_iterator.{cpp,h}` — partition-aware document streaming

## Files to modify

- `src/mongo/db/pipeline/SConscript`
- `src/mongo/db/pipeline/accumulator.{h,cpp}` — extend Accumulator to support optional `processInternalRemove()` for removable variants

## Dependencies

- **Blocks:** [`window-operators.md`](./window-operators.md) — the window-only operators register against this stage's output.
- **Blocked by:**
  - [`accumulator-n-value-family.md`](./accumulator-n-value-family.md) — N-value accumulators usable as window outputs.
  - [`percentile-median-accumulators.md`](./percentile-median-accumulators.md) — same.
  - [`date-arithmetic-expressions.md`](./date-arithmetic-expressions.md) (Tier 1) — `range + unit` reuses unit semantics.

## EloqDoc-specific considerations

- **Memory bounds.** A single partition larger than memory can blow up the executor. Bound buffer size; spill to disk for large windows. Reuse the spill mechanism from [`allowdiskuse-find.md`](./allowdiskuse-find.md).
- **Sort precondition.** The stage typically depends on its input being sorted by the `sortBy` key. Stock MongoDB inserts an implicit `$sort` if not present; verify this works with EloqDoc's pipeline optimizer.
- **No replica-set parallelism.** Stock MongoDB sometimes runs `$setWindowFields` per shard; EloqDoc's single-node model means the entire stage runs locally — simpler.

## Acceptance criteria

- [x] Documents-window: `[-3, "current"]` produces correct moving aggregate over last 4 documents.
- [x] Numeric range-window: `[-2, 0]` correctly slides over non-uniform numeric sort values.
- Range-window: `[-7, 0], unit: "day"` correctly slides over time-based windows even with non-uniform document timestamps.
- [x] Window-only operators (`$rank`, `$denseRank`, `$documentNumber`) produce correct values within each partition.
- [x] `$shift` produces correct lag/lead values.
- [x] `$expMovingAvg` produces correct values with both `N` and `alpha` forms.
- [x] Removable accumulators ($sum, $avg, $count) maintain correct state under sliding-window updates.
- [x] Non-removable accumulators ($min, $max) work correctly (may be slower).
- [x] Empty windows return null per accumulator's null-policy (`$count` returns `0`).
- **Test entry point:** `tests/jstests/eloq_basic/agg_set_window_fields/`. Adapt `jstests/aggregation/sources/setWindowFields/` extensively.

## Implementation notes

The current `document_source_set_window_fields.cpp` implementation buffers all input documents,
stable-sorts them by `partitionBy` and `sortBy`, then recomputes each requested `documents` or
numeric `range` window per output row. This is correct for small/medium compatibility workloads
and keeps the first slices simple, but it is intentionally not spill-capable and does not yet
implement incremental removable accumulator state.

Numeric `range` windows require exactly one `sortBy` key and all values in that key must be
numeric. `range` with `unit` remains unsupported, so date/time range windows are still a follow-up
slice along with implicit sort planning, large-partition memory bounds, and math-heavy window-only
operators.

## Notes from source analyses

All three analyses flag this as the largest single-stage task. `analysis_cc/forward_compat_eval.md`: "Window framing, partitioning, accumulator integration." `analysis_gpt5.5`: "Partition/sort/window executor and memory/spill behavior." `analysis_gpt5.4` recommends sequencing it after the standalone N-value and percentile accumulators land — the executor reuses those.

Suggest delivering as **three sub-PRs**:
1. Stage parsing + partition iteration + standard accumulators (no removable optimization, no window operators) — produces correct but potentially slow results.
2. Removable-accumulator optimization (sum, avg, count) — performance.
3. Window-only operators (rank, expMovingAvg, etc.) — features.
