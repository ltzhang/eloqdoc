# Manual Added Command Notes

These notes cover the small set of version-added commands that were not captured purely as “new IDL command names” in the adjacent-version diffs.

They are implementation-oriented and source-backed, intended to close the biggest remaining gap in the generated inventories.

## v4 -> v5

### `cst`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/cst_command.cpp`
- Purpose: test-only CST pipeline parsing/translation command
- Accepted fields:
  - `pipeline`: BSON object parsed by `BSONLexer` with `START_PIPELINE`
- Output fields:
  - `cst`: serialized CST array
  - `ds`: translated pipeline BSON
- Execution traits:
  - secondary allowed: always
  - write concern: not supported
  - auth: requires any action on any resource in the target DB

### `sbe`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/exec/sbe_cmd.cpp`
- Purpose: test-only command for manually constructing and executing an SBE tree
- Accepted fields:
  - `sbe`: string containing SBE query text
  - cursor command options parsed via `CursorRequest::parseCommandCursorOptions(...)`
    - includes normal cursor batch options such as `batchSize`
- Output:
  - standard cursor response object
- Execution traits:
  - secondary allowed: opt-in
  - write concern: not supported
  - auth: requires any action on any resource in the target DB

### `pinHistoryReplicated`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_commands.cpp`
- Purpose: test-only command that pins the oldest timestamp
- Accepted fields:
  - first command element value: timestamp to pin
  - `round`: optional boolean
- Output fields:
  - `requestedPinTs`
  - `pinTs`
- Execution traits:
  - secondary allowed: never
  - write concern: supported
  - admin only: true
  - authz checks disabled in practice because command is test-gated

### `testDeprecation`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_deprecation_command.cpp`
- Purpose: test command for API-version deprecation logic
- Accepted fields:
  - no command-specific fields beyond the command name
- Output:
  - appends current `APIParameters` info to the reply
- Execution traits:
  - API versions: `1`
  - deprecated API versions: `1`
  - secondary allowed: always
  - write concern: not supported
  - auth required: false

### `testVersion2`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_api_version_2_commands.cpp`
- Accepted fields:
  - no command-specific fields beyond the command name
- Output:
  - current `APIParameters`
- Execution traits:
  - API versions: `2`
  - secondary allowed: always
  - write concern: not supported

### `testVersions1And2`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_api_version_2_commands.cpp`
- Accepted fields:
  - no command-specific fields beyond the command name
- Output:
  - current `APIParameters`
- Execution traits:
  - API versions: `1`, `2`
  - secondary allowed: always
  - write concern: not supported

### `testDeprecationInVersion2`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_api_version_2_commands.cpp`
- Accepted fields:
  - no command-specific fields beyond the command name
- Output:
  - current `APIParameters`
- Execution traits:
  - API versions: `1`, `2`
  - deprecated API versions: `2`
  - secondary allowed: always
  - write concern: not supported

### `testRemoval`

- Introduced on `origin/v5.0`
- Source: `src/mongo/db/commands/test_api_version_2_commands.cpp`
- Accepted fields:
  - no command-specific fields beyond the command name
- Output:
  - current `APIParameters`
- Execution traits:
  - API versions: `1`
  - secondary allowed: always
  - write concern: not supported

## v5 -> v6

### `_configsvrRunRestore`

- Introduced on `origin/v6.0`
- Source: `src/mongo/db/s/config/configsvr_run_restore_command.cpp`
- Purpose: internal restore procedure command for restoring config metadata
- Accepted fields:
  - no explicit command-specific fields are parsed in the command body
  - command behavior depends on the presence of `local.system.collections_to_restore`
- Execution traits:
  - skips API version check
  - secondary allowed: never
  - admin only: true
  - write concern: not supported
  - auth: requires cluster `internal`
  - runtime preconditions:
    - standalone mode only
    - restore procedure mode only

### `_recvChunkReleaseCritSec`

- Introduced on `origin/v6.0`
- Source: `src/mongo/db/s/migration_destination_manager_legacy_commands.cpp`
- Purpose: internal migration-destination command to release the recipient critical section
- Accepted fields:
  - migration session fields parsed via `MigrationSessionId::extractFromBSON(cmdObj)`
- Execution traits:
  - write concern: supported, majority required
  - admin only: true
  - auth: cluster `internal`
  - operation context interrupts on stepdown/stepup

## v6 -> v7

### `_configsvrTransitionToDedicatedConfigServer`

- Introduced on `origin/v7.0`
- Source: `src/mongo/db/s/config/configsvr_transition_to_dedicated_config_server_command.cpp`
- Purpose: internal config-server command to transition from config shard to dedicated config server
- Accepted fields:
  - no explicit command-specific fields in the command body
- Execution traits:
  - skips API version check
  - secondary allowed: never
  - admin only: true
  - write concern: supported, majority required
  - auth: cluster `internal`
  - feature flags checked before execution
  - config-server role required

### `transitionToDedicatedConfigServer`

- Introduced on `origin/v7.0`
- Public/mongos-facing form exists in the transition-to-dedicated-config-server command family
- See the version command inventory and topology command sources for the public request/dispatch path

## v7 -> v8

### `_mongotConnPoolStats`

- Introduced on `origin/v8.0`
- Source: `src/mongo/db/commands/query_cmd/search_test_cmds.cpp`
- Purpose: test/internal stats for server-to-mongot connection pooling
- Accepted fields:
  - no command-specific fields required
- Output:
  - either gRPC network-interface stats or connection pool stats
- Execution traits:
  - admin only: true
  - secondary allowed: always
  - write concern: not supported
  - registered for shard and router, test only

### `_dropConnectionsToMongot`

- Introduced on `origin/v8.0`
- Source: `src/mongo/db/commands/query_cmd/search_test_cmds.cpp`
- Purpose: test/internal connection dropping between server and mongot
- Accepted fields:
  - `hostAndPort`: array of host:port strings
- Execution traits:
  - admin only: true
  - secondary allowed: always
  - write concern: not supported
  - registered for shard and router, test only

### `getShardingReady`

- Introduced on `origin/v8.0`
- Source: `src/mongo/db/sharding_environment/get_sharding_ready_command.cpp`
- Purpose: test-only command indicating whether sharding is ready
- Accepted fields:
  - no command-specific fields required
- Output:
  - `isReady`
- Execution traits:
  - admin only: false
  - secondary allowed: always
  - write concern: not supported
  - registered for shard only, test only

### `timeseriesCatalogBucketParamsChanged`

- Introduced on `origin/v8.0`
- Source: `src/mongo/db/commands/test_commands.cpp`
- Purpose: test command returning whether time-series bucket parameters changed
- Accepted fields:
  - namespace parsed via `CommandHelpers::parseNsCollectionRequired(dbName, cmdObj)`
  - effectively the command target collection plus DB context
- Output:
  - `changed` when available
- Execution traits:
  - admin only: false
  - secondary allowed: always
  - write concern: not supported

### `testCommandFeatureFlaggedOnLatestFCV83`

- Present on `origin/v8.0`
- Source: `src/mongo/db/commands/test_commands.cpp`
- Purpose: test-only feature-flagged command
- Accepted fields:
  - no command-specific fields required
- Output:
  - no structured payload beyond success
- Execution traits:
  - admin only: false
  - secondary allowed: always
  - write concern: supported
  - requires feature flag `gFeatureFlagBlender`
