# Pure expressions batch — trig, regex, trim, conversions, bit-ops, misc

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 | 5.0 / 8.0 | 2–3 weeks (for the full batch; parallelizable) |

## Scope

A single coordinated effort to add ~36 missing aggregation expressions, all of which fit pattern C (REGISTER_EXPRESSION). They're grouped here because:
- The extension pattern is identical for all of them.
- Engineers can be assigned subgroups and work in parallel without conflicts.
- Reviewers benefit from seeing them as one coherent batch.

**Subgroups (consider one PR per subgroup):**

### Trigonometric (14 expressions, 5.0)
`$sin`, `$cos`, `$tan`, `$asin`, `$acos`, `$atan`, `$atan2`, `$sinh`, `$cosh`, `$tanh`, `$asinh`, `$acosh`, `$atanh`, `$degrees`, `$radians`. Each is a thin wrapper around `<cmath>` with NaN/Inf and Decimal128 handling.

### Regex (5 expressions, 5.0)
`$regexFind`, `$regexMatch`, `$regexFindAll`, `$replaceOne`, `$replaceAll`. Use the existing PCRE wrapper that `$regex` (match operator) uses; check `src/mongo/db/matcher/expression_leaf.cpp` for the wrapper class.

### Trim (3 expressions, 5.0)
`$trim`, `$ltrim`, `$rtrim`. Optional `chars` parameter; default is whitespace.

### Rounding (2 expressions, 5.0)
`$round`, `$trunc`. **Verify first whether `$trunc` already exists as math** — if it does, the new one is the variant with optional precision argument and you may need to update the existing parser to accept the second arg.

### Type conversions (10 expressions, 5.0)
`$convert`, `$toBool`, `$toDate`, `$toString`, `$toObjectId`, `$toInt`, `$toLong`, `$toDecimal`, `$toDouble`. The `$to*` family are sugar over `$convert`. Use the existing BSONElement type-conversion helpers (`src/mongo/bson/bsonelement.cpp`).

### Bitwise (4 expressions, 8.0)
`$bitAnd`, `$bitOr`, `$bitXor`, `$bitNot`. Operate on 32-bit/64-bit integers; return error on Decimal128/Double inputs.

### Custom JavaScript (2 expressions, 4.4 / 5.0)
`$function` (custom JS expression callable from any expression context) and `$accumulator` (custom JS accumulator). Both require a JS scope. **Verify whether the scripting subsystem is enabled in EloqDoc** — if `--noscripting` is the default, this group is parser-only with a runtime error.

### Misc (4 expressions, 5.0)
`$rand` (uniform [0,1)), `$binarySize`, `$bsonSize`, `$toHashedIndexKey` (internal — used by hashed-index optimizers).

## Extension pattern

Pattern C — `REGISTER_EXPRESSION` in `src/mongo/db/pipeline/expression*.cpp`.

```cpp
// In expression.cpp (or a new expression_trig.cpp grouping file):
class ExpressionSin final : public ExpressionFixedArity<ExpressionSin, 1> {
public:
    explicit ExpressionSin(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionFixedArity<ExpressionSin, 1>(expCtx) {}
    Value evaluate(const Document& root, Variables* variables) const final {
        Value arg = _children[0]->evaluate(root, variables);
        if (arg.nullish()) return Value(BSONNULL);
        return numericOrDecimalUnary(arg, [](double d){return std::sin(d);},
                                          [](Decimal128 d){return d.sin();});
    }
    const char* getOpName() const final { return "$sin"; }
};
REGISTER_EXPRESSION(sin, ExpressionSin::parse);
```

For variadic expressions (`$convert`, `$bitAnd` etc.) extend `ExpressionVariadic` instead.

## Files to create

One file per subgroup is reasonable:
- `src/mongo/db/pipeline/expression_trigonometric.cpp` (and `.h` if classes are exposed)
- `src/mongo/db/pipeline/expression_regex.cpp`
- `src/mongo/db/pipeline/expression_trim.cpp`
- `src/mongo/db/pipeline/expression_round.cpp`
- `src/mongo/db/pipeline/expression_convert.cpp`
- `src/mongo/db/pipeline/expression_bitwise.cpp`
- `src/mongo/db/pipeline/expression_function.cpp`
- `src/mongo/db/pipeline/expression_misc.cpp`

## Files to modify

- `src/mongo/db/pipeline/SConscript` — add new .cpp files
- `src/mongo/db/pipeline/expression.h` — only if base classes need new shared helpers
- `src/mongo/scripting/...` for `$function`/`$accumulator` if scripting needs hooks

## Dependencies

- **Blocks:** N-value expressions ([`sortarray-firstn-lastn-expressions.md`](./sortarray-firstn-lastn-expressions.md)) typically wait for `$sortArray` semantics; bit-ops are independent.
- **Blocked by:** none.

## EloqDoc-specific considerations

- **Scripting subsystem.** Verify `--noscripting` posture before delivering `$function` / `$accumulator` semantics. If scripting is disabled or stubbed in EloqDoc, ship the parser only and have `evaluate()` `uasserted` with `ErrorCodes::NoSuchKey` (`server-side scripting disabled`). This is a known EloqDoc deviation — see `src/mongo/scripting/` for current state.
- **Decimal128.** All numeric expressions must propagate Decimal128 if any operand is Decimal128; downcast to double otherwise. The existing math expressions (`$add`, `$multiply`) show the pattern.
- **Storage / locking.** None of these expressions touch storage. Safe wrt EloqDoc's no-op locking.

## Acceptance criteria

- Each new expression has a unit test under `tests/cpp_unit_tests/db/pipeline/expression_<group>_test.cpp` covering: normal case, null operand, Decimal128 operand, NaN/Inf where relevant, type-error case.
- Each new expression has a jstest under `tests/jstests/eloq_basic/expression_<name>.js` exercising it inside `$project`, `$match` with `$expr`, and `$group`.
- `db.runCommand({aggregate: "c", pipeline: [{$project: {x: {$sin: 0}}}]})` returns the expected scalar.
- For each subgroup, at least one regression check that pre-existing expressions on the same numeric types still produce identical results.
- **Mass-import opportunity:** the upstream MongoDB jstest suite under `jstests/aggregation/expressions/` covers all of these. Adapt the relevant ones (rename to fit eloq_basic suite layout) rather than writing from scratch.

## Notes from source analyses

All three analyses rate this group "Very Easily Doable". `analysis_cc/forward_compat_eval.md` cautions that `$convert` has tricky null/error semantics (`onError`/`onNull` parameters) — read the upstream spec carefully. `analysis_gpt5.5/raw/v5.0-aggregation_expressions.txt` is the source-of-truth list. **Don't skip `$round` if you have only `$trunc` as math** — they share semantics for default mode but `$round` is banker's rounding by default.
