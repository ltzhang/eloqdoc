# `mongosh` Client-Side Workbook

Sibling checkout used: `/home/lintaoz/work/mongosh`

## Primary source anchors

- `packages/shell-api/src/database.ts`
- `packages/shell-api/src/collection.ts`
- `packages/shell-api/src/mongo.ts`
- `packages/shell-api/src/decorators.ts`
- `packages/service-provider-node-driver/src/node-driver-service-provider.ts`

Generated appendices:

- `generated/mongosh_shell_api_inventory.md`
- `generated/mongosh_method_signatures.md`

## Execution model

`mongosh` is not a thin copy of the in-tree legacy shell.

The current call stack is:

1. shell-visible method on `Mongo`, `Database`, or `Collection`
2. `_runCommand()`, `_runReadCommand()`, `_runAdminCommand()`, or a service-provider helper
3. `ServiceProvider.runCommandWithCheck()` or a typed service-provider method
4. Node driver execution through the `node-driver-service-provider`

This matters for implementation because:

- many modern shell helpers are not handwritten command strings in a JS file
- they are structured TypeScript methods with version metadata and typed parameters

## Compatibility metadata model

`mongosh` methods are decorated with metadata such as:

- `@serverVersions([...])`
- `@apiVersions([...])`
- `@topologies([...])`

This metadata is the clearest client-side compatibility catalog in the whole review.

Two supporting appendices were generated:

- `generated/mongosh_shell_api_inventory.md`: method name, file, line, version gates
- `generated/mongosh_method_signatures.md`: method signatures plus decorators

## High-signal compatibility behaviors

### `Database.hello()`

- gated to `5.0+`
- sends `{ hello: 1 }`
- on `CommandNotFound`, falls back to `isMaster()`

This is the cleanest current client-side expression of the v4/v5 handshake transition.

### `Database.listCommands()`

- sends `{ listCommands: 1 }`
- normalizes:
  - `slaveOk` -> `secondaryOk`
  - `slaveOverrideOk` -> `secondaryOverrideOk`

This is the client-side compatibility bridge for the server-side v4 -> v5 metadata rename.

### `Mongo.bulkWrite()`

- gated to `8.0+`
- implemented through `ServiceProvider.clientBulkWrite(...)`

This is the clearest current `mongosh` marker for an 8.0-only client API.

## Version-gated methods that matter for the v4 -> v8 comparison

### 4.4-era markers

- `Collection.hideIndex()` (`4.4+`)
- `Collection.unhideIndex()` (`4.4+`)
- `Database.sql()` (`4.4+`)

### 5.0-era markers

- `Database.hello()` (`5.0+`)
- `Database.rotateCertificates()` (`5.0+`)

### 6.0-era markers

- `Collection.getSearchIndexes()` (`6.0+`)
- `Collection.createSearchIndex()` (`6.0+`)
- `Collection.createSearchIndexes()` (`6.0+`)
- `Collection.dropSearchIndex()` (`6.0+`)
- `Collection.updateSearchIndex()` (`6.0+`)

### 7.0-era markers

- `Collection.analyzeShardKey()` (`7.0+`)
- `Collection.configureQueryAnalyzer()` (`7.0+`)
- `Collection.checkMetadataConsistency()` (`7.0+`)
- `Database.checkMetadataConsistency()` (`7.0+`)

### 8.0-era markers

- `Mongo.bulkWrite()` (`8.0+`)
- `Collection.getShardLocation()` (`8.0.10+`)

## Practical use when porting APIs forward

If you are implementing newer APIs on top of older server code, use `mongosh` for three things:

1. discover the modern shell-visible helper names and signatures
2. see which helpers are advertised for which server generations
3. copy the compatibility glue for mixed-version behavior, especially:
   - `hello()` fallback
   - `listCommands()` field normalization

## Limits

- This workbook is based on the current `mongosh` checkout, not a full per-tag archaeology.
- The decorators are the best compatibility signal, but some runtime behavior still lives in method bodies and the service-provider layer.
- For exact historical `mongosh` release-by-release archaeology, more tag-by-tag inspection would still be required.
