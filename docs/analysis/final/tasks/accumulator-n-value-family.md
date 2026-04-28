# N-value accumulator family (`$minN`, `$maxN`, `$firstN`, `$lastN`, `$top`, `$topN`, `$bottom`, `$bottomN`)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 5.0 / 6.0 | 1–2 weeks |

## Scope

Eight accumulators that retain N values per group instead of one.

**Checkpoint status:** `$firstN`, `$lastN`, `$minN`, `$maxN`, `$top`, `$topN`,
`$bottom`, and `$bottomN` are implemented for `$group`. Current `{top,bottom}` support
handles simple source-field `sortBy` specifications and buffers each group's candidate values
before sorting/truncating on output. `$firstN`, `$lastN`, `$minN`, and `$maxN` are also
implemented as `$setWindowFields` outputs over the existing document and numeric-range windows.
Window support for `$top`, `$topN`, `$bottom`, `$bottomN`, `$percentile`, and `$median` remains
deferred.

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

- `src/mongo/db/pipeline/accumulator_n_value.cpp` (`$firstN`, `$lastN`, `$minN`, `$maxN`)
- `src/mongo/db/pipeline/accumulator_top_bottom.cpp` (`$top`, `$topN`, `$bottom`, `$bottomN`)

## Files to modify

- `src/mongo/db/pipeline/SConscript`

## Dependencies

- **Blocks:** [`setwindowfields.md`](./setwindowfields.md) — these accumulators are usable as window outputs.
- **Blocked by:** none.

## EloqDoc-specific considerations

- **Memory.** N can be large; bound it. Stock MongoDB enforces a system limit on the heap size to prevent abuse. Match.
- **Checkpoint implementation.** `$minN` / `$maxN` currently collect all values for the group
  and sort/truncate on output. Replacing that with bounded heaps remains part of the deferred
  memory-bound work.
- **No storage interaction.** All state in pipeline memory.

## Acceptance criteria

- [x] `$minN` / `$maxN` produce correct sorted arrays of N values.
- [x] `$firstN` / `$lastN` produce values in insertion order, not sorted.
- [x] `$top` / `$bottom` (single-value forms) match the equivalent `$topN: 1`.
- [x] `$topN` / `$bottomN` with `sortBy` honor the sort and `output` projection.
- [x] `$firstN` / `$lastN` / `$minN` / `$maxN` work as `$setWindowFields` outputs.
- [x] `n <= 0` rejected.
- [x] `n > input.size()` returns all values without padding.
- [x] BSON canonical comparison used consistently (mixed types comparable).
- [x] **Test entry point:** `tests/jstests/eloq_basic/accumulator_n_value.js`.
  Adapted coverage covers `{first,last,min,max}_n` and `{top,bottom}_n` group accumulators.
  `tests/jstests/eloq_basic/agg_set_window_fields/n_value_operators.js` covers the window
  integration slice for `{first,last,min,max}N`.

## Implementation notes

`$top`, `$topN`, `$bottom`, and `$bottomN` use a small parser rewrite in
`accumulation_statement.cpp` so the existing unary accumulator execution path receives an object
containing evaluated sort keys, the literal sort specification, the evaluated output, and `n`.
The accumulator buffers candidate values per group, sorts them by the requested `sortBy` order, and
returns either a single output value or the first `n` outputs.

Deferred semantic differences: this checkpoint is not heap-bounded, does not support dotted
`sortBy` field names in the parser rewrite, and does not yet expose `{top,bottom}N` as window
operators.

## Notes from source analyses

All three analyses rate this Tier 2 with consistent scoping. `analysis_cc/forward_compat_eval.md` actually rates the original family Tier 1 (small, contained, no executor changes) — the Tier 2 framing here accounts for window-context integration. If shipping standalone (no `$setWindowFields` integration), 1 week is realistic; with window integration, 2 weeks.
