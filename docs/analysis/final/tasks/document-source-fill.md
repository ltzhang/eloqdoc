# `$fill` aggregation stage

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 6.0 | 2 weeks |

## Scope

Fills missing or null values in a sorted document stream. Two methods:

- **`locf`** — last observation carried forward
- **`linear`** — linear interpolation between surrounding non-null values
- **literal value** — set to a constant if missing

```js
db.metrics.aggregate([
  { $fill: {
      partitionBy: "$sensorId",
      sortBy: { ts: 1 },
      output: {
          temperature: { method: "linear" },
          pressure:    { method: "locf" },
          location:    { value: "unknown" }
      }
  }}
])
```

Documents must be sorted within each partition; `$fill` does not sort itself (caller responsibility).

**Checkpoint status:** unpartitioned streaming fill is implemented for literal `value` replacement
and `method: "locf"`. `linear`, partitioned fill, and sort-order validation remain deferred.

## Extension pattern

Pattern B. The stage maintains per-partition state across documents; for `linear`, it must buffer documents until both the prior and next non-null observations are known.

```cpp
class DocumentSourceFill final : public DocumentSource {
    GetNextResult getNext() final {
        // For locf: maintain last non-null per partition; substitute null fields.
        // For linear: buffer until next non-null is seen, then emit interpolated values.
        // For value: simple substitution.
    }
};
REGISTER_DOCUMENT_SOURCE(fill, ...);
```

## Files to create

- `src/mongo/db/pipeline/document_source_fill.{cpp,h}`

## Files to modify

- `src/mongo/db/pipeline/SConscript`

## Dependencies

- **Blocks:** none.
- **Blocked by:** [`window-operators.md`](./window-operators.md) — `$fill` shares the partition-tracking helper with `$setWindowFields`'s `$locf` window operator. Implement the helper once, reuse.

## EloqDoc-specific considerations

- **Memory.** Linear interpolation may require buffering many documents if non-null values are sparse. Bound the buffer (default 100 MB matching aggregate); error or spill to disk above that. Spill behavior is shared with [`allowdiskuse-find.md`](./allowdiskuse-find.md) — same code path.
- **Sort precondition.** The stage doesn't sort; verify users hit a clear error if input isn't sorted. Some implementations are tolerant; stock MongoDB enforces sort precondition.

## Acceptance criteria

- [x] `locf` mode forward-fills null values for unpartitioned input.
- `linear` mode interpolates numerical fields between observations.
- [x] Literal-value mode replaces nulls with the supplied value.
- Per-partition state resets between partitions.
- Date interpolation works (treats Date as numeric milliseconds).
- Out-of-order input within a partition produces an error.
- **Test entry point:** `tests/jstests/eloq_basic/agg_fill.js`. Adapt `jstests/aggregation/sources/fill/`.

## Implementation notes

Initial support is a streaming stage in `document_source_fill.cpp`. It preserves each input
document, replacing missing or null output fields according to either a literal `value` rule or the
last non-null value seen for `method: "locf"`.

Deferred semantic differences: MongoDB supports partitioned fill, linear interpolation, and stricter
sort validation. EloqDoc currently rejects partitioned and linear forms with parse errors and trusts
the caller-provided stream order for the unpartitioned LOCF/value checkpoint.

## Notes from source analyses

`analysis_gpt5.5/raw/v6.0-aggregation_stages.txt` confirms 6.0 addition. `analysis_cc/forward_compat_eval.md` rates 2 weeks: "Sorted stream, partition, date semantics — straightforward but easy to get the linear-interpolation edge cases wrong." Adapt upstream tests verbatim.
