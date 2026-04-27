# EloqDoc Backport Project

This workspace is for backporting selected user-facing MongoDB features from newer MongoDB releases into EloqDoc.

EloqDoc is MongoDB-compatible and is based on MongoDB 4.0.3. It replaces the storage layer with Eloq's storage engine, which handles transactions, distributed sharding, and efficient storage. Because those concerns are owned by the Eloq storage engine, backport work in this repository should focus on the user-level MongoDB API surface rather than reimplementing storage-engine internals.

Instruction on how to run EloqDoc can be found at https://www.eloqdata.com/eloqdoc/install-from-binary

## Repository Layout

- `eloqdoc/` is the only git repository used for development work. Source changes, tests, and durable project documents should be committed there.
- `eloqdoc/docs/` contains the backport project documents, including the local test runbook, test-case inventory, and analysis notes.
- `/home/lintaoz/eloq/CLAUDE.md` is a hard link to `eloqdoc/docs/CLAUDE.md`. It exists at the workspace root so agents started from `/home/lintaoz/eloq` see the same instructions, but the tracked copy lives inside the EloqDoc repo.
- `dependencies/` contains already-built dependencies and external libraries. Treat it as local workspace support, not as development output to commit.
- `bin/` is the local install prefix for EloqDoc builds.
- `runtime/` contains local server data, logs, downloaded MongoDB binaries, helper scripts, and test results. It is disposable runtime state and should not be committed.
- `mongo/` and `mongosh/` may be useful references when comparing behavior with newer MongoDB code, but the target product is EloqDoc. Do not commit them into the EloqDoc repo.

## Git and Fork Workflow

The active EloqDoc checkout is `/home/lintaoz/eloq/eloqdoc`.

Remote convention:

- `origin` is the personal fork: `git@github.com:ltzhang/eloqdoc.git`
- `upstream` is the official repo: `git@github.com:eloqdata/eloqdoc.git`

Use feature branches in the EloqDoc repo and push them to `origin`. Open pull requests from the fork back to upstream when ready.

Submodules must remain submodules. Do not vendor their contents into the EloqDoc repo. In git status and commits, submodule paths should remain gitlinks rather than expanded trees.

Do not track build output. The EloqDoc repo already ignores `/build`, and local build/install/runtime locations should remain outside tracked source unless explicitly requested.

## Development Focus

When backporting a MongoDB feature:

1. Preserve EloqDoc's MongoDB 4.0.3 codebase assumptions unless the feature requires a focused compatibility change.
2. Prioritize behavior visible through MongoDB commands, query language, aggregation, shell/client interactions, wire protocol responses, and error handling.
3. Avoid changing Eloq storage-engine behavior unless the user-facing feature cannot be implemented without a minimal storage contract change.
4. Prefer small, reviewable changes that follow the existing MongoDB 4.0.3/EloqDoc style.
5. Use newer MongoDB sources as references for API behavior, but adapt the implementation to the older 4.0.3 architecture.

## Build

The upstream build notes are at:

https://github.com/eloqdata/eloqdoc/blob/main/docs/how-to-compile.md

All dependencies and external libraries have already been built in this workspace. In normal development, only EloqDoc itself needs to be rebuilt.

Use this install prefix:

```bash
export INSTALL_PREFIX=/home/lintaoz/eloq/bin
```

From `eloqdoc/`, build and install core binaries with:

```bash
env WITH_DATA_STORE=ELOQDSS_ROCKSDB \
python scripts/buildscripts/scons.py \
    MONGO_VERSION=4.0.3 \
    VARIANT_DIR=RelWithDebInfo \
    CXXFLAGS="-Wno-nonnull -Wno-class-memaccess -Wno-interference-size -Wno-redundant-move" \
    --build-dir=#build \
    --prefix=$INSTALL_PREFIX \
    --disable-warnings-as-errors \
    -j6 \
    install-core
```

## Notes for Agents

- Work from `/home/lintaoz/eloq/eloqdoc` when building or modifying EloqDoc.
- Treat `INSTALL_PREFIX=/home/lintaoz/eloq/bin` as the default local install destination.
- Do not spend time rebuilding third-party dependencies unless there is clear evidence they changed or are missing.
- For compatibility work, verify behavior at the MongoDB API level whenever possible.
