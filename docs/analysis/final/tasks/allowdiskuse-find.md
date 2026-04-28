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

- [x] `find` accepts `allowDiskUse: true` as a boolean command field.
- [x] Shell helper `db.c.find().sort(...).allowDiskUse()` sends the command field.
- [x] A `find` with sort exceeding 100 MB *fails* without `allowDiskUse`.
- [x] The same query *succeeds* with `allowDiskUse: true`.
- [x] After completion, temp files are cleaned up.
- [x] Cursor pause/resume during a spilled sort still produces correct output.
- **Test entry point:** `tests/jstests/eloq_basic/find_allow_disk_use.js`.

## Implementation notes

EloqDoc support now includes both the command-compatibility slice and executor spill behavior.
`QueryRequest` parses, validates, serializes, and preserves the boolean `allowDiskUse` field, and
the legacy shell exposes `DBQuery.prototype.allowDiskUse()` for clients that use cursor helpers.

For blocking find sorts, `QueryPlannerAnalysis` carries the command flag into `SortNode`, and
`stage_builder` passes it into `SortStageParams`. When enabled, `SortStage` uses the existing
disk-backed sorter with `internalQueryExecMaxBlockingSortBytes` as the memory threshold and
`dbpath/_tmp` as the spill directory. The sorter stores sort keys plus working-set ids, avoiding
assumptions about storage-engine `RecordId` serialization. The stage keeps the existing
`WorkingSetMember` return path by resolving spilled working-set ids back to their buffered members.

Known limitation: the implementation externalizes sort keys and working-set ids, but still keeps
the matching `WorkingSetMember`s in memory until they are returned. This matches the current
executor shape and fixes client-visible `allowDiskUse` behavior for large blocking sort keys, but it
is not a full top-K or fetch-on-output memory refactor.

## Notes from source analyses

`analysis_gpt5.5/feature-backport-evaluation.md`: "Parser easy; real support needs blocking sort spill behavior in query executor." `analysis_cc` agrees and rates 2 weeks. The hidden cost is auditing whether EloqDoc's sort path matches stock MongoDB's — if there's a custom sort path for Data Substrate, this could escalate.
