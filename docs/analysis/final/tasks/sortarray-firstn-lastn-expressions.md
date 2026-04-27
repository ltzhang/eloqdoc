# `$sortArray` and N-value expressions (`$firstN`/`$lastN`/`$minN`/`$maxN`)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 | 7.0 | 3 days |

## Scope

Five aggregation expressions added in 7.0 that operate on array values without requiring a `$group`.

```js
{ $sortArray: { input: "$arr", sortBy: 1 } }                    // ascending sort
{ $sortArray: { input: "$arr", sortBy: { score: -1 } } }        // by sub-field, desc
{ $firstN: { n: 3, input: "$arr" } }                            // first 3 elements
{ $lastN:  { n: 3, input: "$arr" } }
{ $minN:   { n: 3, input: "$arr" } }                            // smallest 3
{ $maxN:   { n: 3, input: "$arr" } }
```

The N-value expressions have **same-named accumulator** counterparts (different code path; covered in [`accumulator-n-value-family.md`](./accumulator-n-value-family.md)). The expression form operates on a single array; the accumulator form operates across documents in `$group`/`$setWindowFields`.

## Extension pattern

Pattern C — `REGISTER_EXPRESSION`.

```cpp
class ExpressionSortArray final : public Expression {
    static intrusive_ptr<Expression> parse(...);
    Value evaluate(const Document& root, Variables*) const final;
    const char* getOpName() const final { return "$sortArray"; }
};
REGISTER_EXPRESSION(sortArray, ExpressionSortArray::parse);
```

## Files to create

- `src/mongo/db/pipeline/expression_sort_array.cpp`
- `src/mongo/db/pipeline/expression_n_value.cpp` (for the four N-value expressions)

## Files to modify

- `src/mongo/db/pipeline/SConscript`

## Dependencies

- **Blocks:** none.
- **Blocked by:** none — but the N-value expressions share comparison logic with the N-value accumulators ([`accumulator-n-value-family.md`](./accumulator-n-value-family.md)). Coordinate so the comparison helper lives in one place.

## EloqDoc-specific considerations

None. These are pure document/array operations.

## Implementation notes

- Implemented as a clean-room backport in
  `src/mongo/db/pipeline/expression_array_n.cpp`; newer MongoDB code was not
  copied.
- `$sortArray` supports whole-value ascending/descending sorting and object
  `sortBy` specifications with one or more dotted field paths. Missing field
  path values are normalized to BSON null for comparison so descending sorts put
  present numeric keys before missing keys.
- `$firstN`, `$lastN`, `$minN`, and `$maxN` are expression forms only; the
  same-named accumulator family remains separate work outside this task.
- Added `tests/jstests/eloq_basic/expression_array_n.js` covering scalar sorts,
  object field sorts, mixed BSON type ordering, null input, `n > input.length`,
  and invalid argument failures.

## Acceptance criteria

- `$sortArray` sorts numbers, strings, dates correctly.
- `$sortArray` with object `sortBy` sorts by sub-field.
- `$sortArray` with non-array input returns null (not error) — matches upstream.
- N-value expressions handle `n > input.length` (return all elements without padding).
- N-value expressions error on `n <= 0` or non-integer `n`.
- `$minN` / `$maxN` use BSON canonical comparison order (mixed types compare consistently).
- **Test entry point:** `tests/jstests/eloq_basic/expression_array_n.js`. Adapt from upstream `jstests/aggregation/expressions/sortArray.js`, `n_array_expressions.js`.

## Notes from source analyses

`analysis_gpt5.5/raw/v7.0-aggregation_expressions.txt` confirms these as 7.0 additions. `analysis_cc` notes the N-value expressions are "trivially separable from the accumulators" — the comparison code is shared, but registration is independent. Don't try to factor them into a single class.
