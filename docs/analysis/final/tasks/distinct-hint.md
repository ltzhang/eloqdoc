# `distinct.hint`

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 (parse + wire) / 2 (full enforcement) | 8.0 | 1–2 days for parse-and-wire |

## Scope

Add the `hint` field to the `distinct` command, mirroring the existing `find.hint`/`aggregate.hint` semantics. `find` and `aggregate` already accept hints in EloqDoc 4.0.3; the work here is plumbing the existing planner-hint mechanism into the `distinct` execution path.

```js
db.c.distinct("category", { active: true }, { hint: { active: 1, category: 1 } })
```

Hint can be an index name (string) or an index spec (object).

## Extension pattern

Command-field handling. Modify the existing `distinct` command implementation (`src/mongo/db/commands/distinct.cpp`).

```cpp
// In DistinctCommand::run / parsing:
BSONObj hint;
if (auto hintEl = cmdObj["hint"]) {
    hint = hintEl.isABSONObj() ? hintEl.Obj() : BSON("$hint" << hintEl.str());
}
// Pass hint through to getExecutorDistinct or the equivalent planner entry.
```

## Files to modify

- `src/mongo/db/commands/distinct.cpp` — accept and propagate `hint`
- `src/mongo/db/query/get_executor.cpp` (or wherever `getExecutorDistinct` lives) — accept hint in the planner-input struct

## Dependencies

- **Blocks:** none.
- **Blocked by:** none.

## EloqDoc-specific considerations

EloqDoc's planner integration with Data Substrate may already accept hints for `find` — replicate that path. If the planner currently ignores hints (defensive, single-engine), then this is genuinely Tier 2: you'd need to teach it to honor them.

## Acceptance criteria

- `db.c.distinct("f", {}, {hint: "myIdx"})` succeeds.
- `db.c.distinct("f", {}, {hint: {f: 1}})` succeeds.
- A non-existent index name returns `BadValue`.
- Explain plan (if available for distinct) shows the hinted index used.
- **Test entry point:** `tests/jstests/eloq_basic/distinct_hint.js`.

## Notes from source analyses

`analysis_gpt5.5/feature-backport-evaluation.md` rates this "Very easily doable to parse; needs dev to enforce". `analysis_cc` agrees. The escalation to Tier 2 is only relevant if EloqDoc's planner doesn't already honor hints for `find` — most likely it does, making this a Tier 1 task.
