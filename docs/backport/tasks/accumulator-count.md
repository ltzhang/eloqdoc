# `$count` accumulator (group-context)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 | 5.0 | 1 day |

## Scope

Add `$count` as an accumulator usable inside `$group` (and later, `$setWindowFields`). Distinct from the existing **stage alias** `$count` — that's a multi-stage rewrite to `$group` + `$project`. The accumulator form lets you count alongside other aggregations:

```js
db.c.aggregate([
  { $group: { _id: "$category", n: {$count: {}}, total: {$sum: "$qty"} } }
])
```

Equivalent to `{$sum: 1}` — but more readable, and it's the form modern drivers and the shell may emit.

## Extension pattern

Pattern D — `REGISTER_ACCUMULATOR`.

```cpp
// accumulator_count.cpp
class AccumulatorCount final : public Accumulator {
public:
    explicit AccumulatorCount(...);
    void processInternal(const Value& input, bool merging) final { ++_count; }
    Value getValue(bool toBeMerged) final { return Value(_count); }
    void reset() final { _count = 0; }
    const char* getOpName() const final { return "$count"; }
    static intrusive_ptr<Accumulator> create(...);
private:
    long long _count = 0;
};
REGISTER_ACCUMULATOR(count, AccumulatorCount::create);
```

The accumulator takes no input expression (`{}` is the only valid argument).

## Files to create

- `src/mongo/db/pipeline/accumulator_count.cpp`

## Files to modify

- `src/mongo/db/pipeline/SConscript`
- `AccumulationStatement::parseAccumulationStatement` (or wherever the accumulator parser dispatches) — `$count` takes `{}` as its argument, not a normal value expression. Verify the parser handles this; if not, special-case it.

## Dependencies

- **Blocks:** `$setWindowFields` ([`setwindowfields.md`](./setwindowfields.md), Tier 2) uses `$count` as a window operator too — but the same accumulator class works in both contexts.
- **Blocked by:** none.

## EloqDoc-specific considerations

None. Accumulator runs in pipeline executor memory; no storage interaction.

## Acceptance criteria

- `{$group: {_id: "$x", n: {$count: {}}}}` produces correct counts per group.
- `{$count: 1}` (any non-empty argument) is rejected with a clear parse error.
- Numeric overflow: the upstream behavior is to use `long long` and saturate; verify here.
- **Test entry point:** `tests/jstests/eloq_basic/accumulator_count.js`. Adapt `jstests/aggregation/accumulators/count.js`.

## Notes from source analyses

All three rate this Tier 1. `analysis_gpt5.5/raw/v5.0-accumulators.txt` lists `$count` as the only accumulator addition in 5.0 (the N-value family arrived in 6.0).
