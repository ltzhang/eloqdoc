# MongoDB API Implementation Index

This is the implementation-grade entry point for the v4.4 -> v8.0 review.

If your goal is “start from older code and port the newer API surface without missing anything”, read the docs in this order:

1. `v4.md` for the baseline parser/execution path and the pre-Stable-API shell behavior.
2. `v5.md` for the largest contract break: Stable API request fields, `RunCommandImpl`, `secondaryOk`, and legacy-shell API flags.
3. `v6.md` for the command-surface expansion and shell flag cleanup.
4. `v7.md` for the request-metadata ordering change and the query-analysis / metadata-consistency additions.
5. `v8.md` for `CommonRequestArgs`, explicit API validation, and the 8.0-only command families.
6. `comparison.md` for the full cross-version matrix and porting sequence.
7. `mongosh.md` for the standalone client-side surface from `/home/lintaoz/work/mongosh`.
8. `full/v5_reference.md`, `full/v6_reference.md`, `full/v7_reference.md`, or `full/v8_reference.md` if you want a single per-version document with the inventories and diffs inlined.

## Generated appendices

These are the exhaustive inventories and diffs that back the narrative docs.

- Per-version IDL command catalogs:
  - `generated/v4_idl_commands.md`
  - `generated/v5_idl_commands.md`
  - `generated/v6_idl_commands.md`
  - `generated/v7_idl_commands.md`
  - `generated/v8_idl_commands.md`
- Per-version manual/non-IDL command catalogs:
  - `generated/v4_manual_commands.md`
  - `generated/v5_manual_commands.md`
  - `generated/v6_manual_commands.md`
  - `generated/v7_manual_commands.md`
  - `generated/v8_manual_commands.md`
- Adjacent-version command diffs:
  - `generated/v4_to_v5_idl_diff.md`
  - `generated/v5_to_v6_idl_diff.md`
  - `generated/v6_to_v7_idl_diff.md`
  - `generated/v7_to_v8_idl_diff.md`
  - `generated/v4_to_v5_manual_diff.md`
  - `generated/v5_to_v6_manual_diff.md`
  - `generated/v6_to_v7_manual_diff.md`
  - `generated/v7_to_v8_manual_diff.md`
- Manual-command source notes for version-added non-IDL commands:
  - `generated/manual_added_command_notes.md`
- Legacy in-tree shell flag inventories and diffs:
  - `generated/v4_shell_flags.md`
  - `generated/v5_shell_flags.md`
  - `generated/v6_shell_flags.md`
  - `generated/v7_shell_flags.md`
  - `generated/v8_shell_flags.md`
  - `generated/v4_to_v5_shell_flag_diff.md`
  - `generated/v5_to_v6_shell_flag_diff.md`
  - `generated/v6_to_v7_shell_flag_diff.md`
  - `generated/v7_to_v8_shell_flag_diff.md`
- `mongosh` inventories:
  - `generated/mongosh_shell_api_inventory.md`
  - `generated/mongosh_method_signatures.md`
- Single-file per-version bundles:
  - `full/v4_reference.md`
  - `full/v5_reference.md`
  - `full/v6_reference.md`
  - `full/v7_reference.md`
  - `full/v8_reference.md`

## Counts

| Version | IDL Commands | Manual / Non-IDL Commands | Legacy Shell Flags |
|---|---:|---:|---:|
| v4 | 50 | 215 | 36 |
| v5 | 131 | 150 | 39 |
| v6 | 165 | 134 | 36 |
| v7 | 216 | 120 | 37 |
| v8 | 258 | 119 | 37 |

## How to port one version forward

For each adjacent transition, use the same workflow:

1. Start from the older version’s full IDL and manual command catalogs.
2. Apply the adjacent IDL diff to pick up every added, removed, or field-changed typed command.
3. Apply the adjacent manual diff to pick up constructor-defined commands that are not covered by IDL.
4. Port the service-entry-point changes described in the target version doc.
5. Port the shell flag and helper changes described in the target version doc.
6. Check `mongosh.md` and the `mongosh` generated appendices for helper methods that expose the new server APIs.

## What is fully exhaustive and what still requires source inspection

- The IDL appendices are field-level exhaustive for commands described in IDL.
- The manual command appendices are exhaustive for command discovery by name/class/source file, but they are not universal field-level reconstructions.
- The focused `manual_added_command_notes.md` closes the biggest gap for new non-IDL commands introduced across adjacent versions.
- For any command that is only present in the manual inventory, the source file listed in that appendix remains authoritative.
