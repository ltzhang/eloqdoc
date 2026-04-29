# `hello` command (alias of `isMaster`)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 | 5.0 | 1 day |

## Scope

Implement `hello` as an alias of the existing `isMaster` command. The reply shape is identical except that the field `ismaster` is also serialized as `isWritablePrimary` (preferred new name). Modern drivers in `apiStrict: true` mode require `hello` and refuse to call `isMaster`.

**In scope:**
- Register `hello` command, dispatching to the same handler as `isMaster`.
- Add `isWritablePrimary` field to the reply (mirror of `ismaster`).
- Accept `helloOk: true` field in the request and echo `helloOk: true` in the reply (signals to the driver that the server understands the new handshake).

**Out of scope:**
- Cluster topology details that don't apply to EloqDoc's single-node model.

## Extension pattern

Pattern A (add a command). The simplest implementation subclasses the existing `isMaster` command class and re-registers under the new name; or shares the `run()` body via a free function.

```cpp
// src/mongo/db/commands/hello.cpp
class CmdHello : public BasicCommand {
public:
    CmdHello() : BasicCommand("hello") {}
    bool run(OperationContext* opCtx, const std::string& dbName,
             const BSONObj& cmdObj, BSONObjBuilder& result) override {
        // Reuse the shared helper that isMaster calls.
        appendIsMasterReply(opCtx, cmdObj, &result);
        // New names:
        result.appendBool("isWritablePrimary", true /* or replica state */);
        result.appendBool("helloOk", true);
        return true;
    }
};
static CmdHello cmdHello;
```

## Files to create

- `src/mongo/db/commands/hello.cpp`
- (header not needed if class is local to the .cpp)

## Files to modify

- `src/mongo/db/commands/isMaster.cpp` (or `is_master.cpp` — confirm filename) — extract the body into a shared helper that both `isMaster` and `hello` can call. Add `helloOk: true` echo to both.
- `src/mongo/db/commands/SConscript` — register the new .cpp.

## Dependencies

- **Blocks:** any modern driver test that uses `apiStrict: true`.
- **Blocked by:** none, but pairs naturally with [`stable-api-envelope.md`](./stable-api-envelope.md).

## EloqDoc-specific considerations

- The `ismaster`/`isWritablePrimary` field always returns `true` in EloqDoc's single-node model — confirm this matches the existing `isMaster` implementation. If `isMaster` already returns dynamic state from Data Substrate's view of the cluster, mirror that exactly.
- `topologyVersion` (5.0 field): if `isMaster` already includes it, mirror; else omit (drivers tolerate its absence).

## Acceptance criteria

- `db.runCommand({hello: 1})` returns a reply with `isWritablePrimary: true` and `helloOk: true`.
- The reply shape is otherwise byte-identical to `db.runCommand({isMaster: 1})` plus the two new fields.
- A 5.0+ Node driver connecting with `serverApi: { version: "1", strict: true }` completes the handshake.
- Sending `{isMaster: 1, helloOk: true}` echoes `helloOk: true` in the reply (so old-name callers can opt in to the new field).
- **Test entry point:** `tests/jstests/eloq_basic/hello_command.js`.

## Notes from source analyses

All three analyses rate this Tier 1 / "very easily doable". `analysis_gpt5.5` notes the `helloOk` echo behavior is required for the driver to know it's safe to use `hello` going forward — don't skip it.
