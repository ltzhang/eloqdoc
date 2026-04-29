# MongoDB v4 -> v8 Comparison

## Cross-version matrix

| Version | Parser / Envelope Model | Stable API Fields | IDL Commands | Manual Commands | Legacy Shell Flags |
|---|---|---|---:|---:|---:|
| v4 | direct `execCommandDatabase(...)` flow | none | 50 | 215 | 36 |
| v5 | `HandleRequest` / `RunCommandImpl`, API params initialized before parse | `apiVersion`, `apiStrict`, `apiDeprecationErrors` | 131 | 150 | 39 |
| v6 | same Stable-API model as v5 | same as v5 | 165 | 134 | 36 |
| v7 | request metadata read before parse | same as v5 | 216 | 120 | 37 |
| v8 | `CommonRequestArgs::parse(...)` + `validateAPIParameters(...)` | same fields, cleaner validation path | 258 | 119 | 37 |

## Parser and handler evolution

### v4 baseline

- parsing lives in `execCommandDatabase(...)`
- no Stable API parsing
- `command->parse(...)` runs before request metadata and session info initialization

### v4 -> v5

- first major request-envelope change
- `initializeAPIParameters(...)` runs before command parse
- `APIParameters` becomes part of `OperationContext`
- `RunCommandImpl` / `HandleRequest` restructure the command path

### v5 -> v6

- envelope stays the same
- most work is command-surface growth, not parser redesign

### v6 -> v7

- second-order parser change
- `readRequestMetadata(...)` moves ahead of `command->parse(...)`

### v7 -> v8

- second major request-envelope change
- `CommonRequestArgs::parse(...)` becomes the normalization layer
- `validateAPIParameters(...)` becomes explicit

## Stable API timeline

### v4

- no Stable API request fields

### v5

- introduces:
  - `apiVersion`
  - `apiStrict`
  - `apiDeprecationErrors`
- introduces server parameters:
  - `requireApiVersion`
  - `acceptApiVersion2` (test-only)

### v6 and v7

- preserve the v5 rules
- use the same basic `initializeAPIParameters(...)` flow

### v8

- preserves the same fields and semantics
- moves validation into `validateAPIParameters(...)`
- explicitly handles `bulkWrite` namespace inspection for `system.js` under strict mode

## `listCommands` metadata evolution

### v4

- shell and server metadata use `slaveOk`

### v5+

- server advertises `secondaryOk`
- optional `secondaryOverrideOk` appears
- `apiVersions` and `deprecatedApiVersions` become visible

This matters for both server compatibility and shell output compatibility.

## Legacy shell flag evolution

Exact inventories and diffs:

- `generated/v4_shell_flags.md`
- `generated/v5_shell_flags.md`
- `generated/v6_shell_flags.md`
- `generated/v7_shell_flags.md`
- `generated/v8_shell_flags.md`
- `generated/v4_to_v5_shell_flag_diff.md`
- `generated/v5_to_v6_shell_flag_diff.md`
- `generated/v6_to_v7_shell_flag_diff.md`
- `generated/v7_to_v8_shell_flag_diff.md`

Timeline:

- v4: no Stable API CLI flags
- v5: adds `apiVersion`, `apiStrict`, `apiDeprecationErrors`
- v6: removes legacy compatibility transport/write/read mode flags, adds `crashOnInvalidBSONError`
- v7: adds `oidcAccessToken`
- v8: no in-tree shell flag delta from v7

## `mongosh` client-surface timeline

Standalone `mongosh` appendices:

- `generated/mongosh_shell_api_inventory.md`
- `generated/mongosh_method_signatures.md`

High-signal server-version gates in current `mongosh`:

- `4.4+`
  - `Collection.hideIndex()`
  - `Collection.unhideIndex()`
  - `Database.sql()`
- `5.0+`
  - `Database.hello()`
  - `Database.rotateCertificates()`
- `6.0+`
  - search index helpers on `Collection`
- `7.0+`
  - `Collection.analyzeShardKey()`
  - `Collection.configureQueryAnalyzer()`
  - `Collection.checkMetadataConsistency()`
  - `Database.checkMetadataConsistency()`
- `8.0+`
  - `Mongo.bulkWrite()`
- `8.0.10+`
  - `Collection.getShardLocation()`

Compatibility behaviors worth copying:

- `Database.hello()` falls back to `isMaster()` on `CommandNotFound`
- `Database.listCommands()` normalizes `slaveOk` -> `secondaryOk` and `slaveOverrideOk` -> `secondaryOverrideOk`

## Command-surface gaps between versions

Use the adjacent generated diffs as the full source of truth:

- `generated/v4_to_v5_idl_diff.md`
- `generated/v5_to_v6_idl_diff.md`
- `generated/v6_to_v7_idl_diff.md`
- `generated/v7_to_v8_idl_diff.md`
- `generated/v4_to_v5_manual_diff.md`
- `generated/v5_to_v6_manual_diff.md`
- `generated/v6_to_v7_manual_diff.md`
- `generated/v7_to_v8_manual_diff.md`

The counts alone show the size of the gaps:

- v8 has `258` IDL commands versus `165` in v6
- v8 has `258` IDL commands versus `131` in v5
- v8 has `258` IDL commands versus `50` in v4

Implementation guidance:

- the fastest way to miss an API is to only read the narrative docs
- the fastest way to avoid missing one is to apply every adjacent generated diff in order

## Recommended porting order from v4 to v8

1. Port v5 request-envelope changes first:
   - `initializeAPIParameters(...)`
   - `APIParameters`
   - `requireApiVersion`
   - `acceptApiVersion2`
   - `secondaryOk` metadata
   - shell API flags and helpers
2. Port the v5 -> v6 command-surface growth.
3. Port the v6 -> v7 request-metadata ordering change.
4. Port the v6 -> v7 command-surface growth.
5. Port the v7 -> v8 `CommonRequestArgs` and `validateAPIParameters(...)` split.
6. Port the v7 -> v8 command-surface growth.
7. Add the `mongosh` helpers that correspond to the server generation you need to expose.

## Remaining caveat

The IDL appendices are field-level exhaustive. The manual-command appendices are exhaustive for discovery, but not universal field-level schemas. For any command that only appears in the manual inventory, use the listed source file as the authoritative implementation reference.
