# Clean-Room Backport Audit

Date: 2026-04-28

## Rule

Newer MongoDB source may be used to understand externally observable API behavior and compatibility semantics only. EloqDoc backport implementation code must be independently written against the EloqDoc/MongoDB 4.0.3 codebase. Do not copy implementation code from newer MongoDB source.

## Scope Audited

Compared branch `backport/tier1-phase1a` against `upstream/main` at `c1111704b0e2e03611554392f33b932a64cf0716`.

Implementation and test files audited:

- `src/mongo/db/api_parameters.h`
- `src/mongo/db/command_generic_argument.cpp`
- `src/mongo/db/commands.h`
- `src/mongo/db/commands/create_indexes.cpp`
- `src/mongo/db/modules/eloq/SConscript`
- `src/mongo/db/pipeline/*` changes and added files
- `src/mongo/db/query/parsed_distinct.*`
- `src/mongo/db/repl/replication_info.cpp`
- `src/mongo/db/service_entry_point_common.cpp`
- `src/mongo/platform/decimal128.*`
- `src/mongo/s/commands/pipeline_s.h`
- Added `tests/jstests/eloq_basic/*.js`

## Findings

### Must Rewrite Before Continuing

1. `src/mongo/db/pipeline/expression.cpp` rounding changes

   The `$round` / two-argument `$trunc` helper is structurally and textually close to newer MongoDB `evaluateRoundOrTrunc` logic, including error text, precision range handling, Decimal128 quantum construction, and conversion-flag handling. This should be rewritten clean-room from public/observable semantics and EloqDoc's existing numeric expression style.

2. `src/mongo/platform/decimal128.cpp` and `src/mongo/platform/decimal128.h` trigonometric additions

   The added Decimal128 trig declarations/wrappers and constants map closely to newer MongoDB Decimal128 additions. Even though they call bundled Intel decimal functions, the wrapper shape and constants should be rewritten or avoided in a clean-room way.

3. `src/mongo/db/pipeline/expression_trigonometric.cpp`

   This file currently carries a newer MongoDB SSPL header, which is not acceptable for EloqDoc's clean-room backport. The implementation should be rewritten with the EloqDoc repository's existing source header and independently structured expression classes/helpers.

### No Direct Copy Found In Manual/Scanner Review

The following files appear independently adapted to EloqDoc's 4.0.3 APIs. Automated exact-window scans and manual review did not find meaningful copied implementation blocks from newer MongoDB source:

- `src/mongo/db/api_parameters.h`
- `src/mongo/db/command_generic_argument.cpp`
- `src/mongo/db/commands/create_indexes.cpp`
- `src/mongo/db/pipeline/accumulator_count.cpp`
- `src/mongo/db/pipeline/document_source_documents.cpp`
- `src/mongo/db/pipeline/document_source_plan_cache_stats.cpp`
- `src/mongo/db/pipeline/document_source_stage_aliases.cpp`
- `src/mongo/db/pipeline/expression_bitwise.cpp`
- `src/mongo/db/pipeline/expression_field_access.cpp`
- `src/mongo/db/pipeline/expression_misc.cpp`
- `src/mongo/db/pipeline/pipeline_d.cpp`
- `src/mongo/db/query/parsed_distinct.*`
- `src/mongo/db/repl/replication_info.cpp`
- `src/mongo/db/service_entry_point_common.cpp`
- `src/mongo/s/commands/pipeline_s.h`

### Tests

The added `tests/jstests/eloq_basic/*.js` files did not show meaningful exact matches to newer MongoDB jstests. They appear to be local compatibility tests. Continue writing tests from public behavior and minimal repros, not by copying upstream test files.

### Documentation

The branch contains a large generated analysis corpus under `docs/analysis/`. This audit focused on implementation/test copy risk. If the project treats extracted command inventories or generated API tables as licensing-sensitive artifacts, review those docs separately before merging.

## Remediation

Completed on 2026-04-28:

1. Rewrote the `$round` / two-argument `$trunc` implementation in
   `src/mongo/db/pipeline/expression.cpp` using local EloqDoc helpers for place parsing,
   decimal quantization, and result type restoration.
2. Removed the added Decimal128 trigonometric wrapper API and constants from
   `src/mongo/platform/decimal128.*`.
3. Rewrote `src/mongo/db/pipeline/expression_trigonometric.cpp` with the repository's AGPL
   source header and an EloqDoc-local implementation structure.
4. Documented the EloqDoc decimal trigonometric semantic difference in
   `docs/analysis/final/tasks/pure-expressions-batch.md`.

Verification after rewrite:

- Build: `install-core` with `-j16` completed with exit code 0.
- Direct JS:
  - `tests/jstests/eloq_basic/expression_round_trunc.js`
  - `tests/jstests/eloq_basic/expression_trigonometric.js`
- Targeted resmoke: both rewritten-expression tests passed.
- Full resmoke: `eloq_basic` passed 36/36 after clearing stale local runtime test databases.
- Hygiene: `git diff --check` passed.

## Required Next Step

Before continuing Tier 1 implementation:

1. Keep subsequent backports clean-room: use newer MongoDB source only for externally
   observable semantics/API shape/tests, not implementation code.
2. When EloqDoc intentionally differs from MongoDB semantics, document the difference in
   the relevant task spec and implementation notes.
