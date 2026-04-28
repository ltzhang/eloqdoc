# `$percentile` and `$median` accumulators

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 7.0 | 2 weeks |

## Scope

Approximate percentile and median accumulators using the t-digest algorithm. Available as group accumulators, window operators, and (in 8.0) plain expressions.

**Checkpoint status:** `$percentile` and `$median` are implemented as `$group` accumulators for
numeric inputs with `method: "approximate"`. This checkpoint uses exact in-memory sorting for
compatibility on bounded result sets; the bounded-memory t-digest sketch, merge serialization, window
operator support, and plain-expression support remain deferred.

```js
db.c.aggregate([
  { $group: {
      _id: "$category",
      p50:    { $median:    { input: "$value", method: "approximate" } },
      p95p99: { $percentile: { input: "$value", p: [0.95, 0.99], method: "approximate" } }
  }}
])
```

**Method:** `"approximate"` is the only option in 7.0 (uses t-digest). Future MongoDB versions plan an `"exact"` mode but it's not in 7.0/8.0 — implement only approximate.

## Extension pattern

Pattern D. Internal data structure: t-digest sketch.

```cpp
class AccumulatorPercentile : public Accumulator {
public:
    void processInternal(const Value& v, bool merging) override {
        if (merging) {
            _digest.merge(/* deserialize partial digest */);
        } else {
            _digest.add(v.coerceToDouble());
        }
    }
    Value getValue(bool toBeMerged) override {
        if (toBeMerged) return Value(_digest.serialize());
        // Compute requested percentiles
        std::vector<Value> result;
        for (double p : _percentiles) result.push_back(Value(_digest.quantile(p)));
        return Value(result);
    }
private:
    TDigest _digest;
    std::vector<double> _percentiles;
};
REGISTER_ACCUMULATOR(percentile, AccumulatorPercentile::create);
REGISTER_ACCUMULATOR(median, AccumulatorMedian::create);
```

`$median` is a thin wrapper around `$percentile` with `p: [0.5]` and unwrapping the singleton result.

## Files to create

- `src/mongo/db/pipeline/accumulator_percentile.{cpp,h}`
- `src/mongo/util/t_digest.{cpp,h}` — vendored or inlined t-digest implementation

## Files to modify

- `src/mongo/db/pipeline/SConscript`
- `src/mongo/util/SConscript`

## Dependencies

- **Blocks:** [`setwindowfields.md`](./setwindowfields.md) — usable as window output.
- **Blocked by:** none.

## EloqDoc-specific considerations

- **Memory bound.** A t-digest is bounded by a `compression` parameter (default 100). Memory use is small (~5 KB per accumulator instance) regardless of input size — no spill needed.
- **Sharded merge** — not applicable (single-node EloqDoc), but implement the merge path correctly anyway (pre-aggregated digests serialize and merge) so the existing pipeline-merge path works for multi-pipeline cases.
- **Choice of t-digest implementation.** Several public implementations exist (e.g. `tdunning/t-digest` in Java, ports to C++). Vendor a small one or implement from the published paper. The choice affects accuracy and merge correctness — pick a well-tested one.

## Acceptance criteria

- [x] `$percentile` with single `p` returns a 1-element array.
- [x] `$percentile` with multiple `p` returns matching-length array.
- [x] `$median` returns a single number.
- Approximation error within published t-digest bounds against known distributions.
- [x] Empty input groups return null (matches upstream behavior).
- [x] Non-numeric input is skipped.
- Merge path produces digests equivalent to single-pass digests on combined input.
- [x] **Test entry point:** `tests/jstests/eloq_basic/accumulator_percentile.js`.
  Adapt `jstests/aggregation/accumulators/percentile.js`.

## Implementation notes

Initial support is registered in `accumulator_percentile.cpp`. The parser accepts the MongoDB
7.0 group-accumulator shape with `input`, `method: "approximate"`, and `$percentile.p` values in
`[0, 1]`. Numeric inputs are collected per group, sorted at result time, and interpolated exactly.
This intentionally differs from MongoDB's t-digest-backed approximate implementation, but preserves
the user-visible accumulator API for small and moderate groups.

Deferred semantic differences: this checkpoint is not memory-bounded, does not serialize/merge
partial digests, and does not expose `$percentile` / `$median` as window operators or plain
expressions.

## Notes from source analyses

All three rate this Tier 2 ~2 weeks. `analysis_cc/forward_compat_eval.md` flags the t-digest choice as the architectural decision: "Don't roll your own; vendor an audited implementation." Heed.
