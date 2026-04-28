# Window-only operators (`$rank`, `$denseRank`, `$documentNumber`, `$expMovingAvg`, `$shift`, `$locf`, `$integral`, `$derivative`, `$covariancePop`, `$covarianceSamp`)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 5.0 / 6.0 | 2 weeks |

## Scope

Operators that only make sense inside `$setWindowFields` — they read window-position metadata or compute stateful values that depend on window iteration order. Cannot be used as plain `$group` accumulators.

**Checkpoint status:** the first position-based set is implemented inside the initial
`$setWindowFields` buffered executor: `$documentNumber`, `$rank`, `$denseRank`, and `$shift`.
`$expMovingAvg` is also implemented for numeric inputs with both `N` and `alpha` forms, and
`$locf` is implemented for last-observation carry-forward inside a partition. `$integral`,
`$derivative`, and covariance operators remain deferred.

| Operator | Description |
| -------- | ----------- |
| `$rank` | Rank within partition; ties get the same rank, gaps after ties (1, 2, 2, 4, …). |
| `$denseRank` | Rank within partition; ties get same rank, no gaps (1, 2, 2, 3, …). |
| `$documentNumber` | Sequential position in partition (1, 2, 3, …). Always unique. |
| `$expMovingAvg` | Exponential moving average. Two forms: `N: <int>` (smoothing) or `alpha: <0..1>`. |
| `$shift` | Lag/lead by N positions in the partition. |
| `$locf` | Last observation carry-forward (for windowed gap-filling). |
| `$integral` | Numerical integration over window using trapezoidal rule. Optional `unit` for date axis. |
| `$derivative` | Numerical differentiation between first and last document in window. Optional `unit`. |
| `$covariancePop` | Population covariance over window. |
| `$covarianceSamp` | Sample covariance over window. |

## Extension pattern

Pattern D variant. These operators don't register with `REGISTER_ACCUMULATOR` (they're not group accumulators); they register with the **window-function** registry inside `$setWindowFields`'s parser.

```cpp
// In window_function_rank.cpp:
class WindowFunctionRank : public WindowFunctionState {
    void add(const Value&) override;
    void remove(const Value&) override;     // some operators don't support remove
    Value getValue() const override;
};
REGISTER_WINDOW_FUNCTION(rank, WindowFunctionRank::create);
```

## Files to create

One file per operator group (or one mega-file `window_functions.cpp`):
- `src/mongo/db/pipeline/window_function_rank.cpp` (rank, denseRank, documentNumber)
- `src/mongo/db/pipeline/window_function_exp_moving_avg.cpp`
- `src/mongo/db/pipeline/window_function_shift.cpp`
- `src/mongo/db/pipeline/window_function_locf.cpp`
- `src/mongo/db/pipeline/window_function_integral.cpp` (integral, derivative)
- `src/mongo/db/pipeline/window_function_covariance.cpp`

## Files to modify

- `src/mongo/db/pipeline/SConscript`
- `src/mongo/db/pipeline/window_executor.cpp` — register-and-dispatch by name

## Dependencies

- **Blocks:** none.
- **Blocked by:** [`setwindowfields.md`](./setwindowfields.md) — the executor must exist for these to plug into.

## EloqDoc-specific considerations

- All state lives in the executor's per-partition memory; no storage interaction.
- `$expMovingAvg` requires `Decimal128` propagation if any input is `Decimal128`; otherwise double.

## Acceptance criteria

- [x] `$rank`, `$denseRank`, `$documentNumber`, and `$shift` produce correct values against focused EloqDoc fixtures.
- [x] `$expMovingAvg` produces correct values with both `N` and `alpha` forms.
- [x] `$locf` carries the latest non-null value forward within each partition.
- Operators that don't support `remove()` (rank, denseRank, documentNumber) trigger window-recomputation correctly when window slides.
- Date-aware operators (`$integral`, `$derivative`) produce time-unit-correct values.
- [x] `$shift` with absent positions returns null (or default if specified).
- **Test entry point:** `tests/jstests/eloq_basic/agg_set_window_fields/position_operators.js`
  for the initial position-based slice, plus
  `tests/jstests/eloq_basic/agg_set_window_fields/exp_moving_avg.js` for `$expMovingAvg`
  and `tests/jstests/eloq_basic/agg_set_window_fields/locf.js` for `$locf`. Adapt
  `jstests/aggregation/sources/setWindowFields/` operator-specific tests as coverage grows.

Current `$expMovingAvg` semantic differences: this checkpoint computes numeric inputs through
double arithmetic and does not preserve Decimal128 result type yet.

## Notes from source analyses

`analysis_cc/forward_compat_eval.md` rates this group 2 weeks (after `$setWindowFields` itself). `analysis_gpt5.5` agrees. The key insight: deliver them in two PRs — the position-based ones (rank/denseRank/documentNumber/shift) are simple and unblock common dashboard use cases; the math-heavy ones (integral, derivative, covariance, expMovingAvg) can come second.
