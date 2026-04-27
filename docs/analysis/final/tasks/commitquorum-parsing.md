# `commitQuorum` (parser-only stub) on `createIndexes`

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 (parser stub) / 2 (full two-phase semantics) | 4.4 / 5.0 | 1 day for stub |

## Scope

Accept the `commitQuorum` field on `createIndexes` and treat it as a no-op (use primary-only / immediate-commit semantics). EloqDoc has no replication coordination layer (Data Substrate handles distributed durability), so the two-phase index-build protocol is not applicable.

```js
db.runCommand({
  createIndexes: "c",
  indexes: [{key:{a:1}, name:"a_1"}],
  commitQuorum: "majority"   // accepted, ignored
})
```

`commitQuorum` accepts: integer (count of voting members), `"majority"`, `"votingMembers"`, or a tag-set name.

**In scope (Tier 1):**
- Parse `commitQuorum` in `createIndexes`.
- Validate the value is a recognized type.
- Ignore semantically.

**Out of scope (Tier 2 if pursued):**
- `setIndexCommitQuorum` admin command.
- `voteCommitIndexBuild` internal command.
- The full two-phase protocol.

## Extension pattern

Command-field handling — modify the existing `createIndexes` parser.

```cpp
// In CmdCreateIndexes::run or wherever indexes are parsed:
if (auto cqEl = cmdObj["commitQuorum"]) {
    // Validate type
    if (cqEl.type() != BSONType::String && !cqEl.isNumber()) {
        uasserted(ErrorCodes::TypeMismatch, "commitQuorum must be string or integer");
    }
    // Don't enforce; log at debug level.
    LOG(2) << "commitQuorum=" << cqEl << " ignored (single-node)";
}
```

## Files to modify

- `src/mongo/db/commands/create_indexes.cpp` — parse and validate

## Dependencies

- **Blocks:** none.
- **Blocked by:** none.

## EloqDoc-specific considerations

Per `analysis_cc/forward_compat_eval.md`: "since you're ignoring replication internals, you may implement a simplified version: always commit when the primary finishes (equivalent to `commitQuorum: 0`)." That's exactly what this stub does. If EloqDoc later adds Data Substrate-aware quorum, escalate to Tier 2 and build on top.

This is intentionally **not semantically equivalent to MongoDB**. In MongoDB, `commitQuorum`
controls when a replicated two-phase index build may commit after enough replica-set members are
ready. EloqDoc does not implement that MongoDB replication protocol in this Tier 1 stub, so
`commitQuorum` is accepted for client compatibility, validated for basic type correctness, and then
ignored. Index builds keep EloqDoc's existing immediate/Data Substrate-backed behavior regardless
of the supplied quorum value.

## Acceptance criteria

- `createIndexes` with `commitQuorum: 0|1|"majority"|"votingMembers"` succeeds.
- `commitQuorum: true` (boolean) fails with `TypeMismatch`.
- Created index is identical regardless of `commitQuorum` value.
- **Test entry point:** `tests/jstests/eloq_basic/commit_quorum_stub.js`.

## Notes from source analyses

`analysis_cc/forward_compat_eval.md` explicitly recommends this parser-only stub as the right call for a single-node fork: "Accept and parse `commitQuorum` but always use primary-only semantics → effectively category 1." `analysis_gpt5.5` rates the full feature Tier 2 but agrees the stub is Tier 1 and likely sufficient.
