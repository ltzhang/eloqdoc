# `find.allowDiskUse`

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 5.0 | 2–3 weeks |

## Scope

Allow blocking sort operations in `find` to spill to disk when the in-memory sort buffer (default 100 MB) is exhausted. `aggregate.allowDiskUse` already exists; this extends the same mechanism to `find`.

```js
db.c.find({}).sort({large_field: 1}).allowDiskUse()
db.runCommand({find: "c", filter: {}, sort: {x:1}, allowDiskUse: true})
```

Without this flag, a large sort fails with `OperationFailed` ("Sort operation used more than the maximum 33554432 bytes…").

## Extension pattern

Command-field + executor.

```cpp
// In FindCommand parsing:
bool allowDiskUse = cmdObj["allowDiskUse"].trueValue();
// Pass to executor; SortStage observes and enables spill mode.

// SortStage already exists in EloqDoc — the work is teaching it to spill
// when allowDiskUse is true.
```

## Files to modify

- `src/mongo/db/commands/find_cmd.cpp` — parse `allowDiskUse`
- `src/mongo/db/query/canonical_query.h` and `.cpp` — carry the flag
- `src/mongo/db/exec/sort.cpp` — implement disk spill (if not already present for aggregate)
- Possibly `src/mongo/db/sorter/sorter.h` — the sort framework supports spill; verify the `find` executor uses it

## Dependencies

- **Blocks:** none.
- **Blocked by:** none.

## EloqDoc-specific considerations

- **Spill location.** The existing aggregate spill writes to a temp directory. EloqDoc may have a different temp-file convention (Data Substrate-managed, S3-backed). Verify; the spill must work in deployed EloqDoc environments where local disk may be ephemeral.
- **Memory limits.** EloqDoc may already have stricter sort memory limits if running in containers. The default 100 MB constant is `internalQueryMaxBlockingSortMemoryUsageBytes` — confirm it's tunable in EloqDoc.
- **Aggregation already has it.** If `aggregate.allowDiskUse` works in EloqDoc today (verify), this is a smaller task — wire `find` into the same SortStage spill path. If aggregate also doesn't actually spill, this is much larger.

## Acceptance criteria

- A `find` with sort exceeding 100 MB *fails* without `allowDiskUse`.
- The same query *succeeds* with `allowDiskUse: true`.
- After completion, temp files are cleaned up.
- Cursor pause/resume during a spilled sort still produces correct output.
- **Test entry point:** `tests/jstests/eloq_basic/find_allow_disk_use.js`.

## Notes from source analyses

`analysis_gpt5.5/feature-backport-evaluation.md`: "Parser easy; real support needs blocking sort spill behavior in query executor." `analysis_cc` agrees and rates 2 weeks. The hidden cost is auditing whether EloqDoc's sort path matches stock MongoDB's — if there's a custom sort path for Data Substrate, this could escalate.
