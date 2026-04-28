# `$densify` aggregation stage

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 6.0 | 2 weeks |

## Scope

Inserts synthetic documents to fill gaps in a sorted numeric or date sequence. Common use case: dashboards that need a row per minute even when the source data has gaps.

**Checkpoint status:** explicit numeric bounds are implemented for unpartitioned and
`partitionByFields` input:
`{field, partitionByFields, range: {step, bounds: [min, max]}}`. Date units,
`bounds: "full"`, and `bounds: "partition"` remain deferred.

```js
db.events.aggregate([
  { $densify: {
      field: "ts",
      partitionByFields: ["sensorId"],
      range: { step: 1, unit: "minute", bounds: "partition" }
  }}
])
```

`bounds`: `"full"` (entire numeric/date range), `"partition"` (per-partition min..max), or an explicit `[min, max]` array.

Generated documents have **only** the partition fields and the densified field — other fields are absent (caller can `$fill` them after).

## Extension pattern

Pattern B. Streaming over a sorted-by-`field` input, the stage emits synthetic documents at each step boundary that has no real document.

```cpp
class DocumentSourceDensify final : public DocumentSource {
    // Per-partition state: lastObservedValue, nextExpectedStep
    // On real document: emit synthetic docs from nextExpectedStep up to current value
    // On EOF / partition switch: emit any remaining synthetic docs to the upper bound
};
```

## Files to create

- `src/mongo/db/pipeline/document_source_densify.{cpp,h}`

## Files to modify

- `src/mongo/db/pipeline/SConscript`

## Dependencies

- **Blocks:** none.
- **Blocked by:** [`date-arithmetic-expressions.md`](./date-arithmetic-expressions.md) (Tier 1) — date densification reuses unit semantics (`step + unit`).

## EloqDoc-specific considerations

- **Bounds: "full" mode** scans the input twice (once for min/max, once to densify). EloqDoc's pipeline executor must handle the rewind. Most implementations buffer in memory; bound the buffer.
- **No `partitionByFields` ⇒ single global partition.** Easy mode.
- **Mixed numeric / date input** — must be rejected with a clear error. Don't densify across types.

## Acceptance criteria

- [x] Numeric densification with explicit bounds produces evenly-spaced synthetic documents in gaps.
- Date densification with `unit` produces correct boundaries (DST handling matches `$dateAdd`).
- `bounds: "full"`, `"partition"`, and explicit `[min, max]` all work.
- [x] Empty input under `bounds: [min, max]` produces synthetic docs covering the entire range.
- [x] Generated docs have only `partitionByFields` + `field`; other fields absent.
- [x] `partitionByFields` with explicit numeric bounds densifies each sorted partition independently.
- [x] Out-of-order input fails with a clear error.
- **Test entry point:** `tests/jstests/eloq_basic/agg_densify.js`. Adapt `jstests/aggregation/sources/densify/`.

## Implementation notes

Initial support is a streaming stage in `document_source_densify.cpp`. It requires upstream input
to be sorted ascending by the densified numeric field, preserves real input documents, and emits
synthetic documents containing only the densified field. Explicit bounds allow the stage to emit a
complete range even when the source collection is empty.

`partitionByFields` is supported for explicit numeric bounds when the upstream input is sorted by
partition fields and then by the densified field. Synthetic partition documents contain only the
partition fields and the densified field.

Deferred semantic differences: MongoDB supports date densification with units and `bounds: "full"`
/ `"partition"`. EloqDoc currently rejects those forms with a parse error rather than accepting
partial semantics.

## Notes from source analyses

`analysis_cc/forward_compat_eval.md` rates 2 weeks. `analysis_gpt5.5` agrees. The trickiest piece is bounds semantics — read the upstream test suite carefully before implementing.
