# `bulkWrite` command (cross-namespace, ordered/unordered, cursor reply)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 2 | 7.0 (8.0 enhancements) | 4–6 weeks |

## Scope

A new top-level write command introduced in 7.0 that accepts a heterogeneous list of insert/update/delete operations targeting potentially different namespaces in a single request.

```js
db.adminCommand({
  bulkWrite: 1,
  ops: [
    {insert: 0, document: {x: 1}},
    {update: 0, filter: {y: 5}, updateMods: {$set: {z: 10}}},
    {delete: 1, filter: {old: true}},
  ],
  nsInfo: [
    {ns: "db1.collA"},
    {ns: "db2.collB"},
  ],
  ordered: true,                    // 7.0
  let: {threshold: 100},            // 8.0
  errorsOnly: false,                // 8.0 — return only errors, not successes
  cursor: { batchSize: 100 }        // reply uses a cursor
})
```

Reply is a tailable cursor over per-op results: `{ok, idx, n, code?, errmsg?, upserted?}` with summary counters `nInserted`, `nMatched`, `nModified`, `nUpserted`, `nDeleted`, `nErrors`.

**Per all three source analyses, this is the single highest-value Tier 2 task.** Mongosh 8.0's `db.getMongo().bulkWrite()` requires it.

## Extension pattern

Pattern A (add a command). The command is administrative-namespace (`bulkWrite` is sent to `admin.$cmd` since it spans namespaces).

```cpp
// src/mongo/db/commands/bulk_write.cpp
class CmdBulkWrite : public BasicCommand {
public:
    CmdBulkWrite() : BasicCommand("bulkWrite") {}
    bool adminOnly() const override { return true; }
    bool run(OperationContext* opCtx, const std::string&,
             const BSONObj& cmdObj, BSONObjBuilder& result) override {
        // 1. Parse `ops` and `nsInfo`.
        // 2. For each op: dispatch to insertBatch / updateOne / updateMany / deleteOne / deleteMany on the named namespace.
        //    Re-use existing CRUD execution (don't reimplement).
        // 3. Collect per-op results.
        // 4. If ordered, halt on first error. If unordered, continue.
        // 5. Build cursor reply with summary counters.
    }
};
static CmdBulkWrite cmdBulkWrite;
```

## Files to create

- `src/mongo/db/commands/bulk_write.{cpp,h}`
- `src/mongo/db/commands/bulk_write_parser.{cpp,h}` — IDL-style parsing of `ops` and `nsInfo`
- `src/mongo/db/commands/bulk_write_cursor.{cpp,h}` — cursor reply construction

## Files to modify

- `src/mongo/db/commands/SConscript`
- `src/mongo/db/commands/run_aggregate.cpp` (or wherever cursor management lives) — `bulkWrite` uses the same cursor framework
- `src/mongo/db/auth/action_types.txt` — verify `bulkWrite` action exists or define it as a privilege superset (insert/update/delete on each named namespace)

## Dependencies

- **Blocks:** mongosh 8.0 `db.getMongo().bulkWrite()` (server-side).
- **Blocked by:**
  - [`update-delete-hint.md`](./update-delete-hint.md) — per-op `hint` field present.
  - [`let-runtime-constants.md`](./let-runtime-constants.md) — 8.0's `let` field.
  - [`pipeline-update-syntax.md`](./pipeline-update-syntax.md) — pipeline-form updates allowed in `updateMods`.

## EloqDoc-specific considerations

- **Cross-namespace transactions.** Stock MongoDB on a replica set runs `bulkWrite`'s ops as separate write transactions when not inside a multi-document transaction. EloqDoc's Tier 2 implementation should match this shape: dispatch each op through the existing insert/update/delete command path and let that path use its normal per-op Data Substrate transaction behavior unless the request is already inside an explicit user transaction. Do not make the whole `bulkWrite` command atomic as one Data Substrate transaction in the initial implementation.
- **No oplog ⇒ no oplog batching.** Stock MongoDB sometimes batches `bulkWrite` ops in a single oplog entry; not applicable here.
- **Authorization.** The auth check is per-op, against each namespace. The simplest implementation does the check up-front for all namespaces touched; alternatively, check before each op. Stock MongoDB does up-front.
- **Retryable writes.** `bulkWrite` ops support `stmtIds` for retryability. EloqDoc's retryable-writes story may or may not be aligned with stock; coordinate with whoever owns that.
- **Document semantic differences.** If any retryable-write, transaction, write-concern, or cursor batching behavior intentionally differs from MongoDB, document the difference in this task file before committing the implementation.

## Acceptance criteria

- Mixed insert/update/delete across two namespaces succeeds with correct per-op results.
- Ordered: errors halt processing; subsequent ops not attempted; `nErrors >= 1` and earlier counters reflect what completed.
- Unordered: errors don't halt; all ops attempted; per-op error reports collected.
- Cursor reply: large `ops` lists split across `getMore` calls.
- Summary counters (`nInserted`, `nMatched`, etc.) accurate.
- 8.0 `errorsOnly: true` omits successes from the cursor stream.
- 8.0 `let` flows into update/delete expressions.
- Per-op `hint` honored by the planner.
- Per-op pipeline-form updates work.
- Authorization fails if user lacks privilege on any named namespace.
- Mongosh 8.0 `db.getMongo().bulkWrite([...])` works end-to-end.
- **Test entry points:** `tests/jstests/eloq_basic/bulkwrite/` (multiple files: ordered, unordered, cursor, errorsOnly, let, hint, pipeline-update). Adapt `jstests/core/bulk_write.js` and friends.

## Notes from source analyses

All three analyses rate this Tier 2 as the highest-value backport. `analysis_cc/forward_compat_eval.md`: "Dispatch to existing CRUD per `nsInfo`; implement ordered/unordered, cursor reply, per-op errors, auth, retry stmt ids, write concern. ~4–6 weeks." `analysis_gpt5.5/feature-backport-evaluation.md`: same. `analysis_gpt5.4` calls out 8.0 enhancements (`let`, `errorsOnly`) as "small additive PRs after the 7.0 base lands."

**Recommended sequencing:** ship the 7.0 base first (2–3 weeks): parser, ordered/unordered dispatch, per-op result accounting, summary counters, authorization, and cursor reply. Add 8.0 enhancements in follow-up commits/PRs: command-level `let`, `errorsOnly`, per-op hint plumbing, and pipeline-form updates once their prerequisite tasks have landed. Mongosh's client-level helper depends on `bulkWrite` being on the wire — until it is, `db.collection.bulkWrite()` falls back to the legacy batch-write commands and works fine.
