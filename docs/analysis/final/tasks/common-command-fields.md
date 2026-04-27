# Common command field passthrough (`comment`, `collectionUUID`, sampling flags)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 | 5.0+ | 2 days |

## Scope

Modern drivers (5.0+) attach several "envelope" fields to most commands — fields that are not part of the command's core semantics but are used for telemetry, namespace verification, and routing. Without permissive parsing, EloqDoc rejects these commands entirely.

**Fields to accept-and-ignore at the command framework level:**

| Field | Origin | Action |
| ----- | ------ | ------ |
| `comment` | 4.4 (5.0 universal) | Already accepted on most ops? Verify; if not, accept everywhere. |
| `collectionUUID` | 5.0 | Strip and ignore (verification feature; without sharding/UUID-routing it's safe to drop). |
| `isTimeseriesNamespace` | 5.0 | Strip; cannot apply (no time-series). |
| `mirrored` | 7.0 | Strip (query analyzer telemetry — not implemented). |
| `sampleId` | 7.0 | Strip (query analyzer telemetry). |
| `$readPreference`, `$clusterTime` | already 4.0 | already handled. |

**Out of scope:**
- Actually performing UUID-based namespace verification (would require Data Substrate cooperation; defer to Tier 4 or a future task if needed).

## Extension pattern

Common command field handling — same code site as Stable API envelope. Modify the command pre-parse path (`service_entry_point_common.cpp` or `commands.cpp::CommandHelpers`).

```cpp
// In runCommandImpl / CommandHelpers::filterCommandRequestForPassthrough:
static const std::set<std::string> kIgnoredEnvelopeFields = {
    "collectionUUID", "isTimeseriesNamespace", "mirrored", "sampleId",
};
BSONObjBuilder stripped;
for (auto&& el : cmdObj) {
    if (kIgnoredEnvelopeFields.count(el.fieldNameStringData().toString())) continue;
    stripped.append(el);
}
// pass `stripped.obj()` to command.run()
```

## Files to create

None.

## Files to modify

- `src/mongo/db/service_entry_point_common.cpp` — extend the field-stripping list
- `src/mongo/db/commands.cpp` (`CommandHelpers`) — ditto if there's a separate filter
- (`src/mongo/db/curop.cpp` if `comment` propagation is desired into profiling)

## Dependencies

- **Blocks:** any 5.0+ jstest that exercises a command — drivers append these fields.
- **Blocked by:** none.

## EloqDoc-specific considerations

`collectionUUID` is the trickiest — in stock MongoDB, it asserts that the target namespace's UUID matches the supplied one. Stripping it is safe in EloqDoc *only if* you don't have a use case where a stale namespace name maps to a different UUID. For a single-node instance with no concurrent rename, stripping is fine. If concurrent renames are a concern, escalate to Tier 2 with a real check against EloqDoc's catalog UUID.

## Implementation notes

- Implemented as Tier 1 command-envelope passthrough in
  `src/mongo/db/command_generic_argument.cpp` and `src/mongo/db/commands.h`.
- This is intentionally **not full MongoDB `collectionUUID` semantics**. MongoDB
  verifies that the requested namespace still matches the supplied UUID. EloqDoc
  accepts and strips `collectionUUID`, `isTimeseriesNamespace`, `mirrored`, and
  `sampleId`; the command then executes by namespace using existing behavior.
- The telemetry fields `mirrored` and `sampleId` also remain no-ops because
  EloqDoc does not implement MongoDB's query analyzer telemetry in this tier.
- These semantic differences are accepted for Tier 1 client compatibility. A
  future stricter implementation should verify `collectionUUID` against
  EloqDoc's catalog and add telemetry plumbing only if those features become
  product requirements.
- Covered by `tests/jstests/eloq_basic/envelope_field_passthrough.js`.

## Acceptance criteria

- Every existing command-level jstest still passes after this change (no regressions).
- A request with `{find: "c", comment: "hello", collectionUUID: UUID()}` succeeds and behaves identically to `{find: "c"}`.
- A request with unknown envelope fields (e.g. `{find: "c", apiVersion: "1", lsid: {...}, txnNumber: NumberLong(1)}`) is unaffected by this change (those have their own handlers).
- **Test entry point:** `tests/jstests/eloq_basic/envelope_field_passthrough.js`.

## Notes from source analyses

`analysis_cc/forward_compat_eval.md` flags `collectionUUID` as a real correctness feature in stock MongoDB but agrees stripping is safe for non-sharded forks. `analysis_gpt5.5` rates the entire group as "Very easily doable" but notes that omitting `comment` propagation into the profiler is a missed opportunity for ops. Add `comment` to `CurOp` if not already done.
