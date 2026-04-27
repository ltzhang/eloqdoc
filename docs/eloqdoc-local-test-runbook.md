# EloqDoc Local Compatibility Test Runbook

This document records the local setup for running EloqDoc and multiple MongoDB reference versions in single-node mode, then running smoke tests and MongoDB JS/resmoke tests against them.

The goal is to measure user-visible MongoDB API compatibility for EloqDoc. Focus on commands, query behavior, aggregation behavior, shell/client interactions, wire responses, and error handling. Storage-engine internals are not the target unless they block a user-facing feature.

EloqDoc binary-run reference:

https://www.eloqdata.com/eloqdoc/install-from-binary

MongoDB archive reference:

https://www.mongodb.com/try/download/community-edition/releases/archive

## Baseline

Host:

- Linux x86_64
- Ubuntu 24.04

EloqDoc binaries:

- `/home/lintaoz/eloq/bin/bin/eloqdoc`
- `/home/lintaoz/eloq/bin/bin/eloqdoc-cli`

Observed EloqDoc version:

- EloqDoc `v0.2.6`
- Compatible with MongoDB `v4.0.3`

## Runtime Workspace

The local runtime workspace is:

```text
/home/lintaoz/eloq/runtime
```

Layout:

```text
runtime/
  compat/             # local compatibility libraries
  downloads/          # downloaded MongoDB archives and compatibility packages
  etc/                # local EloqDoc and data-substrate config
  mongodb/            # extracted MongoDB server versions
  data/               # server dbpaths
  logs/               # server logs
  pids/               # pid files
  results/            # smoke/resmoke results
  run/                # transient runtime files
  scripts/            # helper scripts
  tmp/                # resmoke dbpath scratch area
```

Runtime data is disposable and should stay out of `eloqdoc/build/` and `bin/`.

## Installed Reference Servers

MongoDB Community Server tarballs are extracted under:

```text
/home/lintaoz/eloq/runtime/mongodb
```

Installed versions:

- `4.0.3`
- `4.2.25`
- `4.4.29`
- `5.0.31`
- `6.0.25`
- `7.0.21`
- `8.0.10`

MongoDB `4.0.3` through `5.0.31` need OpenSSL 1.1 on Ubuntu 24.04. The compatibility package was extracted locally to:

```text
/home/lintaoz/eloq/runtime/compat/libssl1.1
```

The helper scripts set `LD_LIBRARY_PATH` for old MongoDB versions only. System libraries were not modified.

To re-download or repair extracted MongoDB binaries:

```bash
runtime/scripts/download-mongodb.sh
```

## Ports

Fixed local ports:

```text
27017  EloqDoc
27040  MongoDB 4.0.3
27042  MongoDB 4.2.25
27044  MongoDB 4.4.29
27050  MongoDB 5.0.31
27060  MongoDB 6.0.25
27070  MongoDB 7.0.21
27080  MongoDB 8.0.10
```

Check running listeners:

```bash
for p in 27017 27040 27042 27044 27050 27060 27070 27080; do
  ss -ltn "sport = :$p"
done
```

## EloqDoc Config

EloqDoc is started using the config-driven flow from EloqData's binary install docs.

Config files:

- `/home/lintaoz/eloq/runtime/etc/eloqdoc.conf`
- `/home/lintaoz/eloq/runtime/etc/data_substrate.cnf`

The server script runs:

```bash
/home/lintaoz/eloq/bin/bin/eloqdoc \
  --config=/home/lintaoz/eloq/runtime/etc/eloqdoc.conf \
  --data_substrate_config=/home/lintaoz/eloq/runtime/etc/data_substrate.cnf \
  --pidfilepath=/home/lintaoz/eloq/runtime/pids/eloqdoc.pid \
  --fork
```

## Helper Scripts

Available scripts:

- `runtime/scripts/download-mongodb.sh`
- `runtime/scripts/start-eloqdoc.sh`
- `runtime/scripts/stop-eloqdoc.sh`
- `runtime/scripts/start-mongo.sh VERSION [--replset]`
- `runtime/scripts/stop-mongo.sh VERSION`
- `runtime/scripts/smoke.sh TARGET`
- `runtime/scripts/run-js.sh TARGET JS_FILE`
- `runtime/scripts/run-resmoke.sh TARGET SUITE_OR_TEST [EXTRA_RESMOKE_ARGS...]`

Targets use these names:

- `eloqdoc`
- `mongo-4.0.3`
- `mongo-4.2.25`
- `mongo-4.4.29`
- `mongo-5.0.31`
- `mongo-6.0.25`
- `mongo-7.0.21`
- `mongo-8.0.10`

## Starting Servers

Start EloqDoc:

```bash
runtime/scripts/start-eloqdoc.sh
```

Start MongoDB 4.0.3:

```bash
runtime/scripts/start-mongo.sh 4.0.3
```

Start another MongoDB version:

```bash
runtime/scripts/start-mongo.sh 8.0.10
```

Start MongoDB in single-node replica set mode when needed:

```bash
runtime/scripts/start-mongo.sh 6.0.25 --replset
```

Most API compatibility tests should start in standalone mode. Use replica set mode only for features that require it, such as transactions, retryable writes, or change streams.

## Stopping Servers

Stop EloqDoc:

```bash
runtime/scripts/stop-eloqdoc.sh
```

Stop a MongoDB reference server:

```bash
runtime/scripts/stop-mongo.sh 4.0.3
```

Logs and data are left in place for debugging.

## Smoke Tests

Run smoke tests:

```bash
runtime/scripts/smoke.sh eloqdoc
runtime/scripts/smoke.sh mongo-4.0.3
runtime/scripts/smoke.sh mongo-8.0.10
```

The smoke test checks:

- `hello` or `isMaster`
- `buildInfo`
- drop database
- insert
- find
- update
- create index
- simple aggregation
- delete

Results are written under:

```text
runtime/results/<timestamp>/<target>/smoke/
```

## Running JS Tests Directly

Run one JS file through the EloqDoc shell against a target server:

```bash
runtime/scripts/run-js.sh eloqdoc /path/to/test.js
runtime/scripts/run-js.sh mongo-4.0.3 /path/to/test.js
```

This is useful for minimal feature-specific tests that do not need the full resmoke harness.

## Running Resmoke Tests

Run a single MongoDB JS test against EloqDoc:

```bash
runtime/scripts/run-resmoke.sh eloqdoc tests/jstests/core/insert1.js
```

Run a single MongoDB JS test against MongoDB 4.0.3:

```bash
runtime/scripts/run-resmoke.sh mongo-4.0.3 tests/jstests/core/insert1.js
```

Run an EloqDoc resmoke suite against EloqDoc:

```bash
runtime/scripts/run-resmoke.sh eloqdoc eloq_basic
```

The wrapper runs from `/home/lintaoz/eloq/eloqdoc` and connects to an already-running server through `--shellPort`. It uses:

```text
/home/lintaoz/eloq/eloqdoc/scripts/buildscripts/resmoke.py
```

Results are written under:

```text
runtime/results/<timestamp>/<target>/resmoke-<suite-or-test>/
```

Each result directory contains:

- `command.txt`
- `stdout.log`
- `stderr.log`
- `summary.md`

## Verified Setup

Verified on 2026-04-27:

Smoke tests passed for:

- `eloqdoc`
- `mongo-4.0.3`
- `mongo-4.2.25`
- `mongo-4.4.29`
- `mongo-5.0.31`
- `mongo-6.0.25`
- `mongo-7.0.21`
- `mongo-8.0.10`

Resmoke validation passed:

```bash
runtime/scripts/run-resmoke.sh eloqdoc tests/jstests/core/insert1.js
```

Current setup summary:

```text
runtime/results/summary.md
```

## Test Progression

Run tests in tiers so failures are useful.

### Tier 0: Smoke

Run smoke tests against EloqDoc and the relevant MongoDB reference versions.

### Tier 1: EloqDoc-Curated Suites

Start with suites already curated for EloqDoc:

- `eloq_basic`
- `eloq_core`
- `eloq_core2`
- `core_op_query_eloq`

Example:

```bash
runtime/scripts/run-resmoke.sh eloqdoc eloq_basic
```

### Tier 2: MongoDB 4.0 API Baseline

Run MongoDB 4.0-era suites from EloqDoc:

- `4.0_dbaas_core`
- `4.0_dbaas_aggregation`
- `4.0_dbaas_decimal`
- `4.0_dbaas_json_schema`
- `4.0_dbaas_geospatial`
- `4.0_dbaas_fts`

Also run selected tests from:

- `tests/jstests/core`
- `tests/jstests/aggregation`
- `tests/jstests/decimal`
- `tests/jstests/core/json_schema`

### Tier 3: Backport Feature Tests

Use `analysis/final/tasks/` to choose feature-specific tests. Start with standalone user-level features:

- `hello` command behavior
- common command fields
- `distinct` with `hint`
- update/delete `hint`
- `find` `allowDiskUse`
- aggregation aliases: `$set`, `$unset`, `$replaceWith`
- aggregation `$merge`
- aggregation `$unionWith`
- pipeline-style update syntax
- date arithmetic expressions
- `$getField`, `$setField`, `$unsetField`
- `$sortArray`, `$firstN`, `$lastN`
- `$count` accumulator
- `$documents`

For each feature:

1. Run the upstream MongoDB test if it is portable.
2. Run the same test against the MongoDB reference version that introduced the behavior.
3. Run against EloqDoc.
4. If the upstream test depends on unrelated infrastructure, write a minimal JS compatibility test that isolates the API behavior.

### Tier 4: Replica Set Only If Needed

Do not run transaction and consistency suites by default. If needed later, use single-node replica set mode for:

- transactions
- retryable writes
- change streams
- causal consistency
- read concern majority behavior

Candidate suites:

- `core_txns`
- `4.0_dbaas_core_txns`
- selected `change_streams*.yml`
- selected `replsets` tests that do not require multi-node behavior

## Failure Classification

Classify failures in each `summary.md` as one of:

- `pass`
- `test-harness-failure`
- `unsupported-storage-internal`
- `unsupported-server-topology`
- `missing-user-api`
- `behavior-mismatch`
- `crash`
- `unknown`

This keeps compatibility work focused on user-visible MongoDB behavior.
