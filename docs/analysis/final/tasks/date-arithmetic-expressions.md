# Date arithmetic expressions (`$dateAdd`, `$dateDiff`, `$dateSubtract`, `$dateTrunc`)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 (parser easy, semantics tricky) | 6.0 | 1 week |

## Scope

Four date-math expressions added in MongoDB 6.0. They share a common parameter shape:

- `startDate: <expr>` — base date
- `unit: <string>` — one of `"year"`, `"quarter"`, `"month"`, `"week"`, `"day"`, `"hour"`, `"minute"`, `"second"`, `"millisecond"`
- `amount: <expr>` (only `$dateAdd`/`$dateSubtract`) — integer count
- `binSize: <expr>` (only `$dateTrunc`) — integer count
- `timezone: <expr>` (optional) — IANA TZ name or fixed offset

```js
{ $dateAdd: { startDate: "$ts", unit: "day", amount: 7, timezone: "America/New_York" } }
{ $dateDiff: { startDate: "$a", endDate: "$b", unit: "hour" } }
{ $dateSubtract: { startDate: "$ts", unit: "month", amount: 1 } }
{ $dateTrunc: { date: "$ts", unit: "day", binSize: 1, timezone: "UTC", startOfWeek: "monday" } }
```

## Extension pattern

Pattern C — `REGISTER_EXPRESSION`. Each is a regular `Expression` subclass parsing its own object syntax (not a fixed-arity expression).

```cpp
// expression_date_arithmetic.cpp
class ExpressionDateAdd final : public Expression {
public:
    static intrusive_ptr<Expression> parse(...);
    Value evaluate(const Document& root, Variables*) const final;
    const char* getOpName() const final { return "$dateAdd"; }
private:
    intrusive_ptr<Expression> _startDate, _amount, _timezone;
    TimeUnit _unit;
};
REGISTER_EXPRESSION(dateAdd, ExpressionDateAdd::parse);
```

## Files to create

- `src/mongo/db/pipeline/expression_date_arithmetic.cpp` (and `.h`)
- `src/mongo/util/time_unit.h` — enum + parse helpers (`year`/`quarter`/.../`millisecond`) if not already in `src/mongo/util/`

## Files to modify

- `src/mongo/db/pipeline/SConscript` — add new .cpp
- (potentially) `src/mongo/db/query/datetime/date_time_support.cpp` — extend with arithmetic helpers if the library used by `$dateFromParts` doesn't already cover unit-bumping arithmetic

## Dependencies

- **Blocks:** `$fill` (Tier 2, may use `$dateTrunc` internally), `$densify` (Tier 2, uses unit-and-bound semantics).
- **Blocked by:** none.

## EloqDoc-specific considerations

None at the storage/concurrency level. The trickiness is purely **temporal correctness**:

- DST handling — adding "1 day" to `2026-03-08T01:00 America/New_York` must yield `2026-03-09T01:00`, even though that's 23 wall-clock hours.
- Month/quarter/year arithmetic must clamp end-of-month: `Jan 31 + 1 month = Feb 28/29`.
- `startOfWeek` (only on `$dateTrunc`) defaults to Sunday but accepts any day-of-week string.
- `binSize` interacts with `unit`: `binSize: 5, unit: "minute"` truncates to 5-minute boundaries.

These are well-tested by the upstream MongoDB jstest suite — adapt those tests verbatim.

## Acceptance criteria

- All four expressions parse and evaluate correctly across the nine supported units.
- DST transition tests pass for `America/New_York`, `Europe/London`, `Australia/Sydney`.
- Month-end clamping tests pass for `$dateAdd` and `$dateSubtract`.
- `$dateTrunc` with `binSize` produces expected truncation boundaries.
- Null / non-Date `startDate` returns BSON null (not error), matching upstream behavior.
- **Test entry points:** `tests/cpp_unit_tests/db/pipeline/expression_date_arithmetic_test.cpp` and `tests/jstests/eloq_basic/expression_date_arithmetic.js`. The upstream jstest `jstests/aggregation/expressions/date_*.js` set covers most cases — adapt directly.

## Notes from source analyses

`analysis_gpt5.5/feature-backport-evaluation.md` rates this "Very easily doable to needs dev" — flagging the timezone semantics as the only real risk. `analysis_cc/forward_compat_eval.md` agrees this is Category 1 but warns: "DST and month-end semantics are subtle; copy upstream tests verbatim, don't write your own." Heed that. `analysis_gpt5.5/raw/v6.0-aggregation_expressions.txt` is the canonical 6.0 list confirming no other date arithmetic was added in 6.0.
