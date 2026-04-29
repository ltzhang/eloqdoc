# `update.hint`, `delete.hint`

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 5.0 | 1 week |

## Scope

Add the `hint` field to `update` and `delete` commands (plus their per-op forms in `bulkWrite`/legacy batch writes), mirroring the existing `find.hint` semantics. The query planner must honor the hint when selecting the access path for the update/delete's filter.

```js
db.c.update({ active: true }, { $inc: { hits: 1 } }, { hint: { active: 1 } })
db.c.deleteMany({ stale: true }, { hint: "stale_idx" })
```

## Extension pattern

Command-field + planner. The `find.hint` path already exists; replicate it for the write commands.

```cpp
// In ParsedUpdate / ParsedDelete:
BSONObj hint;
if (auto el = cmdObj["hint"]) hint = (el.isABSONObj() ? el.Obj() : BSON("$hint" << el.str()));
// pass into the planner-input struct used by getExecutorUpdate / getExecutorDelete
```

The work is in `getExecutorUpdate` and `getExecutorDelete` plumbing, not in inventing new mechanism.

## Files to modify

- `src/mongo/db/commands/write_commands/write_commands.cpp` (per-command parsing) and `src/mongo/db/ops/parsed_update.{h,cpp}`, `parsed_delete.{h,cpp}` — accept `hint`
- `src/mongo/db/query/get_executor_update.cpp`, `get_executor_delete.cpp` — pass hint through to the planner
- `src/mongo/db/query/query_planner_params.h` — verify hint already present (likely yes, used by find)

## Dependencies

- **Blocks:** [`bulkwrite-command.md`](./bulkwrite-command.md) — `bulkWrite` per-op forms include `hint`.
- **Blocked by:** none.

## EloqDoc-specific considerations

- Same caveat as `distinct-hint.md`: verify EloqDoc's planner currently honors hints for `find`. If yes, this is straightforward. If the EloqDoc planner ignores hints (single-engine simplification), Tier 2 becomes Tier 3.
- Update/delete plans differ subtly from find plans — the planner returns an "update plan" wrapping a child query plan. The hint applies to the child.

## Acceptance criteria

- [x] `update`/`delete` with valid `hint` (string or object) execute against the hinted index when applicable.
- [x] Invalid hint (nonexistent index name, mismatched key pattern) returns `BadValue`.
- [x] `hint` honored when the predicate is non-trivial; explain output reflects it.
- [x] `_id` update/delete with an explicit non-`_id` hint bypasses the IDHACK path and honors
  the requested hint (matches MongoDB 8.0 behavior).
- **Test entry point:** `tests/jstests/eloq_basic/write_hint.js`. Adapt `jstests/core/update_hint.js`, `jstests/core/delete_hint.js`.

## Implementation status

Implemented in `6baabc77`:

- Write command IDL accepts per-op `hint` on update and delete entries.
- String hints are normalized to the existing query planner shape, `{$hint: <name>}`.
- Update/delete request plumbing carries the hint through normal execution and explain setup.
- Runtime and explain-shape coverage is in `tests/jstests/eloq_basic/write_hint.js`.

The `_id`/unique-index edge was rechecked against MongoDB 8.0.10: an explicit non-`_id` hint is
allowed, and the write honors the hinted plan instead of requiring the `_id` index. The focused test
now verifies hinted UPDATE/DELETE explain shapes for `_id` predicates, normal execution, invalid hint
rejection, and parse validation.

## Notes from source analyses

All three rate Tier 2. `analysis_gpt5.5` notes: "Requires planner enforcement and error behavior; parser alone is insufficient." That's the difference from `distinct-hint.md` which is more permissive about non-enforcement.
