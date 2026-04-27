# Stable API envelope (`apiVersion`, `apiStrict`, `apiDeprecationErrors`)

| Tier | Version added | Effort estimate |
| ---- | ------------- | --------------- |
| 1 (parser-permissive) | 5.0 | 2–3 days |

## Scope

Accept the three Stable API request fields (`apiVersion: "1"`, `apiStrict: bool`, `apiDeprecationErrors: bool`) on every command without erroring. The fields are **stripped and ignored** at this tier — semantically the server behaves identically regardless of values.

**In scope (Tier 1):**
- Parse all three fields permissively in the common command pre-parse path.
- Accept `apiVersion: "1"` (the only valid value at present); reject other values with `APIVersionError` (matches MongoDB's wire-level behavior so drivers don't silently fall through).
- Don't propagate semantics into individual commands; ignore `apiStrict` and `apiDeprecationErrors`.

**Out of scope (escalates to Tier 3):**
- Full strict-mode enforcement: per-command `apiVersions()` declarations, deprecated-field rejection, `unstable_field` annotations, server parameters `requireApiVersion` / `acceptApiVersion2`.
- Stability annotations on every command's IDL.

## Extension pattern

Common command field handling. Modify the command pre-parse path in `service_entry_point_common.cpp` (or wherever `parseCommonCommandFields` lives in this codebase — search for where `$db`, `lsid`, `txnNumber` are stripped).

```cpp
// In the common pre-parse:
if (auto apiVersion = cmdObj["apiVersion"]) {
    if (apiVersion.type() != BSONType::String || apiVersion.str() != "1") {
        uasserted(ErrorCodes::APIVersionError,
                  str::stream() << "Unrecognized API version: " << apiVersion.toString());
    }
    // strip; do not propagate
}
// repeat for apiStrict (bool) and apiDeprecationErrors (bool)
```

## Files to create

- `src/mongo/db/api_parameters.h` (small struct holding parsed values, for future use)

## Files to modify

- `src/mongo/db/service_entry_point_common.cpp` — strip fields in `runCommandImpl` or equivalent
- `src/mongo/db/commands.cpp` — ensure `BasicCommand::run` does not see the fields
- `src/mongo/db/error_codes.err` (or `.idl`) — add `APIVersionError` if absent (verify presence first; it may already be there as a generic code)

## Dependencies

- **Blocks:** `hello-command.md` (clients sending `apiVersion: "1"` expect both `hello` and Stable API to be accepted together).
- **Blocked by:** none.

## EloqDoc-specific considerations

None at Tier 1. The fields are stripped before they reach any code that touches Data Substrate or storage. If escalating to Tier 3 in the future, note that EloqDoc's command framework predates the IDL stability annotations system from upstream — that escalation is a much larger effort than upstream's because the IDL infrastructure isn't there.

## Acceptance criteria

- Sending `{find: "c", apiVersion: "1"}` succeeds and behaves identically to without the field.
- Sending `{find: "c", apiVersion: "1", apiStrict: true, apiDeprecationErrors: true}` succeeds.
- Sending `{find: "c", apiVersion: "2"}` fails with `APIVersionError`.
- Sending `{find: "c", apiVersion: 1}` (number, not string) fails with `APIVersionError` or a type error.
- Driver handshake from a 5.0+ Node/Python driver completes successfully.
- **Test entry points:** create `tests/jstests/eloq_basic/stable_api_permissive.js` covering the four cases above; existing upstream tests under `jstests/api_version*` are too strict for this tier.

## Notes from source analyses

`analysis_gpt5.5/feature-backport-evaluation.md` rates the permissive form as "Very easily doable" and the strict form as "Very hard to do" — the same split applied here. `analysis_cc/forward_compat_eval.md` agrees and notes the permissive form is required for any modern driver to work, even if you never plan to enforce strict mode. Don't conflate these two pieces of work.
