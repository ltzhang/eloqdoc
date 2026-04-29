# `$getField`, `$setField`, `$unsetField` expressions

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 | 7.0 | 3 days |

## Scope

Three expressions added in 7.0 that allow accessing fields with names that are not legal as path expressions (e.g. fields containing `.` or starting with `$`).

```js
{ $getField: { field: "user.email", input: "$doc" } }
{ $setField: { field: "$_id", value: 42, input: "$$ROOT" } }
{ $unsetField: { field: "secret", input: "$$ROOT" } }
```

`field` can be a literal string or an expression that resolves to a string. `input` defaults to `$$CURRENT` if omitted (for `$getField`; `$setField` requires it explicitly).

## Extension pattern

Pattern C — `REGISTER_EXPRESSION`.

```cpp
class ExpressionGetField final : public Expression {
    static intrusive_ptr<Expression> parse(...);
    Value evaluate(const Document& root, Variables*) const final;
    const char* getOpName() const final { return "$getField"; }
};
REGISTER_EXPRESSION(getField, ExpressionGetField::parse);
```

## Files to create

- `src/mongo/db/pipeline/expression_field_access.cpp` (one file for all three)

## Files to modify

- `src/mongo/db/pipeline/SConscript`

## Dependencies

- **Blocks:** none.
- **Blocked by:** none. Pairs naturally with the rest of the Tier 1 expressions batch.

## EloqDoc-specific considerations

None. These are pure document-level operations.

## Acceptance criteria

- `$getField` returns the value of a literal-named field, even when the name contains `.` or starts with `$`.
- `$getField` returns `MISSING` for absent fields (allowing chaining with `$ifNull`).
- `$setField` with an existing field replaces it; with a new field adds it.
- `$setField` with `value: "$$REMOVE"` removes the field (semantically equivalent to `$unsetField`).
- `$unsetField` is a thin alias for `$setField` with `$$REMOVE`.
- Type-error tests: `field` not a string → error.
- **Test entry point:** `tests/jstests/eloq_basic/expression_field_access.js`. Adapt from upstream `jstests/aggregation/expressions/getField.js` etc.

## Notes from source analyses

All three analyses rate this Category 1. `analysis_gpt5.5/raw/v7.0-aggregation_expressions.txt` confirms these are the only field-access additions in 7.0. The trap (per `analysis_cc`) is forgetting that `$getField` does *not* descend into nested fields when the name contains `.` — it treats the dot as part of the field name, which is the whole point of the expression.
