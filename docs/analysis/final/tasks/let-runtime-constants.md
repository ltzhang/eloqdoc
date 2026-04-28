# `let` and `runtimeConstants` on find/update/delete/aggregate

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 5.0 | 1–2 weeks |

## Scope

Two related features that flow user-bound and server-bound variables into expression evaluation:

- **`let`** (user-facing): a request field carrying a `{varName: <expr>}` map. Variables are bound for the lifetime of the request and accessible as `$$varName` inside `$expr`, aggregation expressions, pipeline-form updates, etc.

  ```js
  db.c.find({$expr: {$gt: ["$qty", "$$threshold"]}}, {let: {threshold: 100}})
  db.c.update({$expr: {$lt: ["$balance", "$$min"]}}, [{$set: {flagged: true}}], {let: {min: 0}})
  ```

- **`runtimeConstants`** (internal — driver-set): server-bound values like `localNow` and `clusterTime`. Mostly internal, but parsing must accept them so that 5.0+ drivers can set them on commands that get retried under transactions.

Applies to: `find`, `update`, `delete`, `aggregate`, `findAndModify` (5.0+).

## Extension pattern

Command framework + expression-context plumbing.

```cpp
// Parse `let` in each command:
BSONObj letVars;
if (auto el = cmdObj["let"]) letVars = el.Obj();

// Construct ExpressionContext with the user variables bound:
auto expCtx = make_intrusive<ExpressionContext>(opCtx, ...);
for (auto&& el : letVars) {
    auto varExpr = Expression::parseOperand(expCtx, el, ...);
    Value v = varExpr->evaluate(Document{}, &expCtx->variables);
    expCtx->variables.setConstantValue(expCtx->variablesParseState.defineVariable(el.fieldName()), v);
}
```

The variables system already exists in EloqDoc 4.0 (`$$ROOT`, `$$CURRENT`, `$let` expression). The work is exposing it at the **command** level rather than just inside aggregation expressions.

## Files to modify

- `src/mongo/db/commands/find_cmd.cpp` (or `find.cpp`) — parse `let`, plumb into ExpressionContext
- `src/mongo/db/commands/write_commands/write_commands.cpp` (or wherever update/delete parsing is) — same for write paths
- `src/mongo/db/commands/run_aggregate.cpp` (or `pipeline_command.cpp`) — same
- `src/mongo/db/commands/find_and_modify.cpp` — same
- `src/mongo/db/pipeline/expression_context.{h,cpp}` — expose API for command-level variable binding (likely already there; just adds entry points)
- Verify match-expression evaluation (`src/mongo/db/matcher/expression_expr.cpp` for `$expr`) sees the bound variables — should be free since `$expr` runs through the same evaluation path

## Dependencies

- **Blocks:** [`document-source-merge.md`](./document-source-merge.md) (some `$merge` semantics use let), [`pipeline-update-syntax.md`](./pipeline-update-syntax.md) (pipeline-form update is the most common consumer).
- **Blocked by:** none.

## EloqDoc-specific considerations

- **Variable visibility into the storage engine.** Most user `let` values are scalars/objects evaluated once at the start of the request. They never reach the storage engine — they're substituted into expressions. No concurrency concern.
- **`runtimeConstants.localNow`.** Must be evaluated once per request and used consistently. EloqDoc's clock source for "now" must be stable across the request — confirm.
- **Implementation scope decision.** Implement `let` consistently across all documented command entry points in one milestone: `find`, `aggregate`, `update`, `delete`, and `findAndModify`. Partial command coverage is confusing for drivers and should be avoided unless a narrower compatibility subset is explicitly documented.
- **Semantic-difference rule.** If EloqDoc accepts `runtimeConstants` but does not expose every MongoDB internal runtime constant with identical behavior, document the difference here with the MongoDB behavior, the EloqDoc behavior, and the compatibility rationale.

## Acceptance criteria

- `db.c.find({$expr: {$eq: ["$x", "$$v"]}}, {let: {v: 42}})` returns matches.
- `db.c.update({$expr: {$gt: ["$qty", "$$lim"]}}, [{$set: {hi: true}}], {let: {lim: 10}})` updates correctly.
- `db.c.aggregate([{$match: {$expr: {$eq: ["$f", "$$x"]}}}], {let: {x: 1}})` returns matches.
- Undefined `$$varName` (not in `let`) yields `Variable not found` error matching upstream wording.
- `runtimeConstants` is accepted but doesn't surface to user-visible behavior (treated as internal).
- **Test entry point:** `tests/jstests/eloq_basic/let_in_commands.js`. Adapt `jstests/core/let_*.js` (large suite).

## Notes from source analyses

All three analyses rate this Tier 2 with the same caveat: "must flow variables into match expression and pipeline expression contexts." `analysis_cc` notes the variables infrastructure already exists — the work is exposure at the command boundary, not building from scratch. `analysis_gpt5.5` adds: "trivial-looking until you discover that `findAndModify`'s update phase needs the same plumbing as `update`." Don't forget `findAndModify`.
