# N-value accumulator family (`$minN`, `$maxN`, `$firstN`, `$lastN`, `$top`, `$topN`, `$bottom`, `$bottomN`)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 5.0 / 6.0 | 1–2 weeks |

## Scope

Eight accumulators that retain N values per group instead of one.

| Accumulator | Behavior |
| ----------- | -------- |
| `$minN` | Bottom-N values by BSON comparison. |
| `$maxN` | Top-N values by BSON comparison. |
| `$firstN` | First N input values per group (insertion order). |
| `$lastN` | Last N input values per group. |
| `$top` | Single top value by a sort spec. Like `$first` but with explicit sort. |
| `$topN` | Top N by sort spec. |
| `$bottom` | Single bottom by sort spec. |
| `$bottomN` | Bottom N by sort spec. |

```js
db.c.aggregate([
  { $group: {
      _id: "$category",
      topThree: { $maxN: { input: "$score", n: 3 } },
      mostRecent: { $top: { sortBy: { ts: -1 }, output: "$value" } }
  }}
])
```

## Extension pattern

Pattern D — `REGISTER_ACCUMULATOR`. Internal data structure: bounded heap (priority queue) of size N.

```cpp
class AccumulatorMinN : public Accumulator {
public:
    void processInternal(const Value& v, bool merging) override {
        _heap.push(v);
        if (_heap.size() > _n) _heap.pop();      // keep N smallest
    }
    Value getValue(bool toBeMerged) override {
        // Drain heap into BSON array (sorted ascending).
    }
private:
    int _n;
    std::priority_queue<Value, std::vector<Value>, ValueComparator> _heap;
};
REGISTER_ACCUMULATOR(minN, AccumulatorMinN::create);
```

`$top`/`$topN`/`$bottom`/`$bottomN` are like `$minN`/`$maxN` but take a `sortBy` spec rather than comparing by the input itself. Implementation reuses the same heap structure with a custom comparator.

## Files to create

- `src/mongo/db/pipeline/accumulator_min_max_n.cpp` (`$minN`, `$maxN`)
- `src/mongo/db/pipeline/accumulator_first_last_n.cpp` (`$firstN`, `$lastN`)
- `src/mongo/db/pipeline/accumulator_top_bottom.cpp` (`$top`, `$topN`, `$bottom`, `$bottomN`)

## Files to modify

- `src/mongo/db/pipeline/SConscript`

## Dependencies

- **Blocks:** [`setwindowfields.md`](./setwindowfields.md) — these accumulators are usable as window outputs.
- **Blocked by:** none.

## EloqDoc-specific considerations

- **Memory.** N can be large; bound it. Stock MongoDB enforces a system limit on the heap size to prevent abuse. Match.
- **No storage interaction.** All state in pipeline memory.

## Acceptance criteria

- `$minN` / `$maxN` produce correct sorted arrays of N values.
- `$firstN` / `$lastN` produce values in insertion order, not sorted.
- `$top` / `$bottom` (single-value forms) match the equivalent `$topN: 1`.
- `$topN` / `$bottomN` with `sortBy` honor the sort and `output` projection.
- `n <= 0` rejected.
- `n > input.size()` returns all values without padding.
- BSON canonical comparison used consistently (mixed types comparable).
- **Test entry point:** `tests/jstests/eloq_basic/accumulator_n_value.js`. Adapt `jstests/aggregation/accumulators/{first,last,min,max,top,bottom}_n.js`.

## Notes from source analyses

All three analyses rate this Tier 2 with consistent scoping. `analysis_cc/forward_compat_eval.md` actually rates the original family Tier 1 (small, contained, no executor changes) — the Tier 2 framing here accounts for window-context integration. If shipping standalone (no `$setWindowFields` integration), 1 week is realistic; with window integration, 2 weeks.
