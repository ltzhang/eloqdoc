# MongoDB Test Case Inventory

This document inventories public MongoDB-related test suites and test-case
sources, including API tests, regression tests, unit tests, integration tests,
driver conformance tests, and external compatibility/correctness suites.

It is not a generated list of every individual test file. The MongoDB server,
driver, shell, and tooling repositories contain thousands of individual tests
that change frequently. This file instead identifies the canonical repositories
and directories where those test cases live.

## 1. MongoDB Server

Repository: <https://github.com/mongodb/mongo>

### JavaScript Server Tests

Primary server integration, API, and regression tests live under:

- `jstests/`

Important `jstests` subdirectories include:

- `jstests/aggregation`
- `jstests/auth`
- `jstests/change_stream_fsm`
- `jstests/change_streams`
- `jstests/client_encrypt`
- `jstests/concurrency`
- `jstests/core`
- `jstests/core_sharding`
- `jstests/core_standalone`
- `jstests/decimal`
- `jstests/disk`
- `jstests/extensions`
- `jstests/fail_point`
- `jstests/fle2`
- `jstests/hooks`
- `jstests/httpinterface`
- `jstests/libs`
- `jstests/libunwind`
- `jstests/multiVersion`
- `jstests/noPassthrough`
- `jstests/noPassthroughWithMongod`
- `jstests/ocsp`
- `jstests/product_limits`
- `jstests/query_golden`
- `jstests/query_golden_sharding`
- `jstests/readonly`
- `jstests/replsets`
- `jstests/resmoke_selftest`
- `jstests/selinux`
- `jstests/serial_run`
- `jstests/serverless`
- `jstests/sharding`
- `jstests/slow1`
- `jstests/ssl`
- `jstests/sslSpecial`
- `jstests/ssl_linear`
- `jstests/ssl_x509`
- `jstests/suites`
- `jstests/third_party`
- `jstests/watchdog`
- `jstests/with_mongot`
- `jstests/write_path`

### Resmoke Test Suites

MongoDB server JavaScript tests are commonly run through `resmoke.py`.
The suite definitions live under:

- `buildscripts/resmokeconfig/suites/`

This directory contains hundreds of suite YAML files, including suites for:

- aggregation
- auth
- audit
- backup/restore
- benchmarks
- bulk write
- causal consistency
- change streams
- client-side encryption
- clustered collections
- concurrency
- initial sync
- multi-version testing
- query
- replication
- retryable writes
- sharding
- transactions
- upgrade/downgrade testing

### C++ Unit and Component Tests

C++ unit and component tests are distributed throughout `src/mongo/**`,
typically as `*_test.cpp` files and test targets.

Important test-related roots include:

- `src/mongo/unittest`
- `src/mongo/dbtests`
- `src/mongo/base`
- `src/mongo/bson`
- `src/mongo/client`
- `src/mongo/crypto`
- `src/mongo/db`
- `src/mongo/embedded`
- `src/mongo/executor`
- `src/mongo/idl`
- `src/mongo/logv2`
- `src/mongo/otel`
- `src/mongo/platform`
- `src/mongo/replay`
- `src/mongo/rpc`
- `src/mongo/s`
- `src/mongo/scripting`
- `src/mongo/shell`
- `src/mongo/stdx`
- `src/mongo/tools`
- `src/mongo/transport`
- `src/mongo/util`
- `src/mongo/watchdog`

### Build, Harness, and Tooling Tests

MongoDB server build and test-harness tests include:

- `buildscripts/tests`
- `buildscripts/resmokelib`
- `buildscripts/idl`
- `buildscripts/bazel_testbuilds`
- `buildscripts/golden_test.py`
- `buildscripts/package_test.py`
- `evergreen/*_test.py`

### Bundled Third-Party Test Suites

The server repository also vendors third-party projects that may include their
own tests:

- `src/third_party/wiredtiger`
- `src/third_party/JSON-Schema-Test-Suite`
- `src/third_party/abseil-cpp`
- `src/third_party/asio`
- `src/third_party/benchmark`
- `src/third_party/boost`
- `src/third_party/folly`
- `src/third_party/gperftools`
- `src/third_party/grpc`
- `src/third_party/json-c`
- `src/third_party/mozjs`
- `src/third_party/opentelemetry-cpp`
- `src/third_party/prometheus-cpp`
- `src/third_party/protobuf`
- `src/third_party/rnp`
- `src/third_party/s2`
- `src/third_party/zstandard`

## 2. MongoDB Driver Specification Tests

Repository: <https://github.com/mongodb/specifications>

This is the canonical source for cross-driver API and behavior tests. Official
drivers consume these tests to verify that each language driver behaves
consistently.

Important specification and test areas include:

- `source/auth`
- `source/bson-binary-encrypted`
- `source/bson-binary-uuid`
- `source/bson-binary-vector`
- `source/bson-corpus`
- `source/bson-decimal128`
- `source/bson-objectid`
- `source/causal-consistency`
- `source/change-streams`
- `source/client-backpressure`
- `source/client-side-encryption`
- `source/client-side-operations-timeout`
- `source/collation`
- `source/collection-management`
- `source/command-logging-and-monitoring`
- `source/compression`
- `source/connection-monitoring-and-pooling`
- `source/connection-string`
- `source/connections-survive-step-down`
- `source/crud`
- `source/dbref`
- `source/enumerate-collections`
- `source/enumerate-databases`
- `source/extended-json`
- `source/faas-automated-testing`
- `source/gridfs`
- `source/index-management`
- `source/initial-dns-seedlist-discovery`
- `source/load-balancers`
- `source/logging`
- `source/max-staleness`
- `source/message`
- `source/mongodb-handshake`
- `source/ocsp-support`
- `source/open-telemetry`
- `source/polling-srv-records-for-mongos-discovery`
- `source/read-write-concern`
- `source/retryable-reads`
- `source/retryable-writes`
- `source/run-command`
- `source/server-discovery-and-monitoring`
- `source/server-selection`
- `source/serverless-testing`
- `source/sessions`
- `source/socks5-support`
- `source/transactions`
- `source/transactions-convenient-api`
- `source/unified-test-format`
- `source/uri-options`
- `source/versioned-api`

High-value API test categories:

- CRUD tests
- retryable reads and writes
- transactions
- sessions
- change streams
- GridFS
- connection strings and URI options
- server discovery and monitoring
- server selection
- connection pooling
- command monitoring
- client-side encryption
- load balancer support
- versioned API behavior
- BSON and Extended JSON corpus tests

## 3. Official Driver Test Suites

Official MongoDB drivers each have their own unit, integration, API, and
specification-runner tests. Most also consume the shared tests from
`mongodb/specifications`.

### Python / PyMongo

Repository: <https://github.com/mongodb/mongo-python-driver>

Test roots:

- `test`
- `integration_tests`
- `mypy_test.ini`

### Java, Kotlin, and Scala Drivers

Repository: <https://github.com/mongodb/mongo-java-driver>

Test roots:

- `testing`
- module-level `src/test` directories
- `driver-benchmarks`

### Go Driver

Repository: <https://github.com/mongodb/mongo-go-driver>

Test roots:

- `*_test.go` files throughout the repository
- `testdata`
- `etc`

### C#/.NET Driver

Repository: <https://github.com/mongodb/mongo-csharp-driver>

Test roots:

- `tests`
- `specifications`
- `benchmarks`
- module-level test projects under `src`

### Node.js Driver

Repository: <https://github.com/mongodb/node-mongodb-native>

Test roots:

- `test`
- `etc`
- unit, integration, TypeScript, and specification tests

### Ruby Driver

Repository: <https://github.com/mongodb/mongo-ruby-driver>

Test roots:

- `spec`
- `.rspec`

### C Driver

Repository: <https://github.com/mongodb/mongo-c-driver>

Test roots:

- `src/libmongoc/tests`
- `src/libbson/tests`
- `src/common/tests`
- `etc`

### C++ Driver

Repository: <https://github.com/mongodb/mongo-cxx-driver>

Test roots:

- `src/bsoncxx/test`
- `src/mongocxx/test`
- `benchmark`
- `etc`

### PHP Library

Repository: <https://github.com/mongodb/mongo-php-library>

Test roots:

- `tests`
- `benchmark`

### PHP Extension / Driver

Repository: <https://github.com/mongodb/mongo-php-driver>

Test roots:

- `tests`
- `src`

### Swift Driver

Repository: <https://github.com/mongodb/mongo-swift-driver>

Test roots:

- `Tests`
- `etc`

### Rust Driver

Repository: <https://github.com/mongodb/mongo-rust-driver>

Test roots:

- Rust unit tests embedded in `src/**/*.rs`
- integration and specification tests

## 4. Shell, GUI, BSON, Crypto, and Tools

### mongosh

Repository: <https://github.com/mongodb-js/mongosh>

Test types:

- package-level unit tests
- integration tests
- end-to-end shell tests
- API behavior tests for shell helpers

### Compass

Repository: <https://github.com/mongodb-js/compass>

Test types:

- package-level unit tests
- React/UI tests
- integration tests
- end-to-end application tests
- query, schema, CRUD, aggregation, connection, and explain-plan tests

### BSON JavaScript

Repository: <https://github.com/mongodb/js-bson>

Test types:

- BSON corpus tests
- Extended JSON tests
- unit tests
- serialization/deserialization compatibility tests

### libmongocrypt

Repository: <https://github.com/mongodb/libmongocrypt>

Test types:

- unit tests
- crypto tests
- client-side field level encryption tests
- Queryable Encryption / FLE behavior tests

### MongoDB Database Tools

Repository: <https://github.com/mongodb/mongo-tools>

Test roots:

- `test`
- `integration`
- `manual-tests`
- `bsondump`
- `mongodump`
- `mongodump_passthrough`
- `mongoexport`
- `mongofiles`
- `mongoimport`
- `mongorestore`
- `mongostat`
- `mongotop`

Covered tools:

- `mongodump`
- `mongorestore`
- `mongoimport`
- `mongoexport`
- `bsondump`
- `mongostat`
- `mongotop`
- `mongofiles`

## 5. Kubernetes and Operators

### Community Kubernetes Operator

Repository: <https://github.com/mongodb/mongodb-kubernetes-operator>

Test categories:

- controller unit tests
- reconciliation tests
- Kubernetes resource generation tests
- end-to-end deployment tests
- Helm/chart or manifest validation tests

### Enterprise Kubernetes Operator

Repository: <https://github.com/mongodb/mongodb-enterprise-kubernetes>

Test categories:

- enterprise deployment lifecycle tests
- Ops Manager / Cloud Manager integration tests
- replica set and sharded cluster operator tests
- backup and automation integration tests

### Atlas Kubernetes Operator

Repository: <https://github.com/mongodb/mongodb-atlas-kubernetes>

Test categories:

- controller tests
- Atlas API integration tests
- CRD validation tests
- end-to-end Kubernetes tests

## 6. External Compatibility, Correctness, and Stress Suites

### Jepsen MongoDB Tests

Repository: <https://github.com/jepsen-io/mongodb>

Test focus:

- distributed-system correctness
- elections and failover
- network partitions
- consistency anomalies
- replica set behavior under faults

### YCSB MongoDB Binding

Repository: <https://github.com/brianfrankcooper/YCSB>

Test focus:

- benchmark workloads
- CRUD throughput and latency
- workload A/B/C/D/E/F style database tests
- MongoDB binding compatibility

### FerretDB Compatibility Tests

Repository: <https://github.com/FerretDB/FerretDB>

Test focus:

- MongoDB wire/API compatibility
- query behavior
- aggregation behavior
- BSON behavior
- command compatibility

### Percona Server for MongoDB

Repository: <https://github.com/percona/percona-server-mongodb>

Test focus:

- downstream MongoDB server regression tests
- build tests
- integration tests
- storage/security/backup feature tests

### Amazon DocumentDB Compatibility Material

Public Amazon DocumentDB repositories and samples may contain MongoDB API
compatibility checks and examples. These are useful for compatibility research,
but they are not canonical MongoDB test suites.

## Recommended Starting Set

For broad MongoDB behavior coverage, start with:

1. `mongodb/mongo:jstests`
2. `mongodb/mongo:buildscripts/resmokeconfig/suites`
3. `mongodb/mongo:src/mongo/**/*_test.cpp`
4. `mongodb/specifications:source/**`
5. Official driver test suites for the target language
6. `mongodb/mongo-tools`
7. `mongodb-js/mongosh`
8. `mongodb/libmongocrypt`
9. `jepsen-io/mongodb`

For API conformance specifically, prioritize:

1. `mongodb/specifications`
2. the official driver test suite for the client language
3. `mongodb/mongo:jstests/core`
4. `mongodb/mongo:jstests/core_sharding`
5. `mongodb/mongo:jstests/replsets`
6. `mongodb/mongo:jstests/sharding`
7. `mongodb/mongo:jstests/aggregation`
8. `mongodb/mongo:jstests/change_streams`
9. `mongodb/mongo:jstests/transactions` if present in the checked branch, or
   transaction-related resmoke suites otherwise

