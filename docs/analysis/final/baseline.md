# EloqDoc 4.0.3 Baseline — What's Already There

This document inventories what EloqDoc currently supports, so engineers don't reimplement existing code, and documents the **seven extension patterns** every backport relies on. All counts and registrations were verified by grepping the EloqDoc source tree at the time of writing.

---

## 1. Existing aggregation stages (28)

Registered via `REGISTER_DOCUMENT_SOURCE` and `REGISTER_MULTI_STAGE_ALIAS` in `src/mongo/db/pipeline/`:

`$addFields`, `$bucketAuto`, `$collStats`, `$currentOp`, `$facet`, `$geoNear`, `$graphLookup`, `$group`, `$indexStats`, `$_internalInhibitOptimization`, `$_internalSplitPipeline`, `$limit`, `$listLocalSessions`, `$listSessions`, `$lookup`, `$match`, `$mergeCursors`, `$out`, `$project`, `$redact`, `$replaceRoot`, `$sample`, `$skip`, `$sort`, `$unwind`

**Aliases:** `$bucket`, `$changeStream`, `$count`, `$sortByCount`

> **Note:** `$count`, `$bucket`, `$sortByCount`, and `$changeStream` are already registered as multi-stage aliases. Don't re-add them. The 5.0 stages users expect that are *missing* and need to be added are listed in the version-evolution and Tier 1 docs.

---

## 2. Existing aggregation expressions (~91)

Registered via `REGISTER_EXPRESSION` (verified count: 91 distinct names). Coverage:

- **Math:** `$abs`, `$add`, `$ceil`, `$divide`, `$exp`, `$floor`, `$ln`, `$log`, `$log10`, `$mod`, `$multiply`, `$pow`, `$sqrt`, `$subtract`
- **Comparison:** `$cmp`, `$eq`, `$gt`, `$gte`, `$lt`, `$lte`, `$ne`
- **Boolean / control flow:** `$and`, `$or`, `$not`, `$cond`, `$ifNull`, `$switch`
- **Array / set:** `$allElementsTrue`, `$anyElementTrue`, `$arrayElemAt`, `$arrayToObject`, `$concatArrays`, `$filter`, `$in`, `$indexOfArray`, `$isArray`, `$map`, `$objectToArray`, `$range`, `$reduce`, `$reverseArray`, `$setDifference`, `$setEquals`, `$setIntersection`, `$setIsSubset`, `$setUnion`, `$size`, `$slice`, `$zip`
- **String:** `$concat`, `$indexOfBytes`, `$indexOfCP`, `$split`, `$strcasecmp`, `$strLenBytes`, `$strLenCP`, `$substrBytes`, `$substrCP`, `$substr`, `$toLower`, `$toUpper`
- **Date components:** `$dateFromParts`, `$dateFromString`, `$dateToParts`, `$dateToString`, `$dayOfMonth`, `$dayOfWeek`, `$dayOfYear`, `$hour`, `$isoDayOfWeek`, `$isoWeek`, `$isoWeekYear`, `$millisecond`, `$minute`, `$month`, `$second`, `$week`, `$year`
- **Variable / literal:** `$let`, `$literal`, `$const`
- **Type:** `$type`, `$isNumber` (verify in source)
- **Meta:** `$meta`

**Notably MISSING** (and therefore in scope for backport):
- Trig functions (14): `$sin`, `$cos`, `$tan`, `$asin`, `$acos`, `$atan`, `$atan2`, `$sinh`, `$cosh`, `$tanh`, `$asinh`, `$acosh`, `$atanh`, `$degrees`, `$radians`
- Regex expressions: `$regexFind`, `$regexMatch`, `$regexFindAll`, `$replaceOne`, `$replaceAll`
- Trim/case: `$trim`, `$ltrim`, `$rtrim`
- Rounding: `$round`, `$trunc` (note: `$trunc` may already exist as math; verify before adding)
- Conversions: `$convert`, `$toBool`, `$toDate`, `$toString`, `$toObjectId`, `$toInt`, `$toLong`, `$toDecimal`, `$toDouble`
- Custom JS: `$function`, `$accumulator`
- Misc: `$rand`, `$binarySize`, `$bsonSize`, `$toHashedIndexKey`
- Date arithmetic: `$dateAdd`, `$dateDiff`, `$dateSubtract`, `$dateTrunc`
- Field access: `$getField`, `$setField`, `$unsetField`
- Array manipulation: `$sortArray`, `$firstN`, `$lastN`, `$minN`, `$maxN` (as expressions)
- Bitwise: `$bitAnd`, `$bitOr`, `$bitXor`, `$bitNot`

---

## 3. Existing accumulators (11)

Registered via `REGISTER_ACCUMULATOR`:

`$addToSet`, `$avg`, `$first`, `$last`, `$max`, `$mergeObjects`, `$min`, `$push`, `$stdDevPop`, `$stdDevSamp`, `$sum`

**Missing** (in scope for backport):
- `$count` (group-context — distinct from the stage alias)
- N-value: `$minN`, `$maxN`, `$firstN`, `$lastN`, `$top`, `$topN`, `$bottom`, `$bottomN`
- Approximate: `$percentile`, `$median`
- Custom: `$accumulator`
- Window-only: `$rank`, `$denseRank`, `$documentNumber`, `$expMovingAvg`, `$covariancePop`, `$covarianceSamp`, `$integral`, `$derivative`, `$shift`, `$locf`

---

## 4. Existing index types (6)

Implemented as `IndexAccessMethod` subclasses in `src/mongo/db/index/`:

`btree`, `hash` (hashed), `2d`, `s2` (2dsphere), `fts` (text), `haystack` (geoHaystack — deprecated upstream)

**Missing** (in scope):
- `wildcard` (4.2 feature — never landed in this fork)
- `clustered` (5.0+)
- `columnstore` (6.0; deprecated in 8.0 — explicitly out of scope, see `tier-3-hard.md`)
- `queryable_encrypted_range` (7.0 — internal to FLE2)
- `2dsphere_bucket` (6.0 — internal to time-series)

---

## 5. Existing collection options

`CollectionOptions` (`src/mongo/db/catalog/collection_options.h`):

- `uuid` (CollectionUUID)
- `capped` + `cappedSize`, `cappedMaxDocs`
- `autoIndexId` (DEFAULT/YES/NO)
- `temp`
- `storageEngine`, `indexOptionDefaults`
- `idIndex`
- `validator`, `validationAction`, `validationLevel`
- `collation`
- `viewOn`, `pipeline` (views ARE supported)
- MMAPv1-era leftover fields: `flags`, `initialNumExtents`, `initialExtentSizes`

**Missing** (in scope):
- `timeseries` (5.0)
- `clusteredIndex` (5.0)
- `recordPreImages` (5.0) / `changeStreamPreAndPostImages` (6.0)
- `encryptedFields` (6.0)
- `recordIdsReplicated` (8.0)

> **Verified 2026-04-27:** none of {`timeseries`, `clusteredIndex`, `changeStreamPreAndPostImages`, `encryptedFields`, `recordIdsReplicated`} appears in `CollectionOptions::parse` (`collection_options.cpp`).

---

## 6. Existing match-expression operators

Defined as the `MatchType` enum in `src/mongo/db/matcher/expression.h`. Coverage matches stock MongoDB 4.0:

- **Logical:** `$and`, `$or`, `$nor`, `$not`
- **Comparison:** `$eq`, `$ne`, `$lt`, `$lte`, `$gt`, `$gte`, `$in`, `$nin`
- **Element:** `$exists`, `$type`
- **Evaluation:** `$mod`, `$regex`, `$where`, `$expr`, `$jsonSchema` (verify), `$text`
- **Array:** `$size`, `$elemMatch`, `$all`
- **Bitwise:** `$bitsAllSet`, `$bitsAllClear`, `$bitsAnySet`, `$bitsAnyClear`
- **Geospatial:** `$geoWithin`, `$geoIntersects`, `$near`, `$geoNear`

`$expr` is present, which means aggregation expressions can be used inside match queries — relevant for `$let` / `runtimeConstants` plumbing.

---

## 7. Authentication & authorization

Files in `src/mongo/db/auth/` confirm:

- **SCRAM-SHA-1** — `sasl_scram_server_conversation.cpp`
- **SCRAM-SHA-256** — same file (4.0 baseline supports both)
- **PLAIN** — `sasl_plain_server_conversation.cpp`
- Role-based access control via `AuthorizationManager` / `AuthorizationSession`

**Missing** (in scope):
- `rotateCertificates` command (4.4) — Tier 1
- OIDC (`MONGODB-OIDC`) authentication (7.0) — Tier 3
- LDAP-backed roles (4.4) — out of scope

---

## 8. Stable API plumbing

**Verified absent.** No `api_parameters` files in `src/mongo/idl/`; no `apiVersion`/`apiStrict` field handling in command framework. This is a Tier 1 task (parser-permissive mode) initially, escalating to Tier 3 if full strict-mode enforcement is wanted.

---

## The seven extension patterns

Every backport in this folder uses one of these. Memorize which one applies to your task before opening a file.

### Pattern A — Add a command

Subclass `BasicCommand` (or `Command`) in `src/mongo/db/commands/<your_command>.cpp`:

```cpp
class MyNewCmd : public BasicCommand {
public:
    MyNewCmd() : BasicCommand("myNewCommand") {}
    bool run(OperationContext*, const std::string&,
             const BSONObj&, BSONObjBuilder&) override { ... }
    // override: secondaryAllowed(), supportsWriteConcern(), etc.
};
static MyNewCmd cmdInstance;  // static-init registration
```

Add the .cpp to `src/mongo/db/commands/SConscript`. **No macro required.** Dispatch goes through `CommandHelpers::findCommand` in `service_entry_point_common.cpp`.

### Pattern B — Add an aggregation stage

Verified at `src/mongo/db/pipeline/document_source.h:87-127`:

```cpp
// In document_source_foo.cpp
REGISTER_DOCUMENT_SOURCE(foo,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourceFoo::createFromBson);
```

For a stage that *expands* into multiple existing stages (e.g. an alias), use `REGISTER_MULTI_STAGE_ALIAS` instead. Both rely on `MONGO_INITIALIZER` so registration happens at static-init time.

The class itself extends `DocumentSource` and overrides `getNext()`, `serialize()`, `getSourceName()`, etc.

### Pattern C — Add an aggregation expression

```cpp
// In expression_foo.cpp
REGISTER_EXPRESSION(foo, ExpressionFoo::parse);
```

Class extends `Expression` (typically through `ExpressionFixedArity<N>` or `ExpressionVariadic`). Override `evaluate(const Document&, Variables*) const` and `getOpName()`.

### Pattern D — Add an accumulator

Verified at `src/mongo/db/pipeline/accumulation_statement.h:47-51`:

```cpp
REGISTER_ACCUMULATOR(foo, AccumulatorFoo::create);
```

Class extends `Accumulator` and implements `processInternal()`, `getValue()`, `reset()`.

### Pattern E — Add an index type

Subclass `IndexAccessMethod` in `src/mongo/db/index/<name>_access_method.{h,cpp}`. Overrides:

- `getKeys()` — produce index keys from a document
- `insert()`, `remove()`, `update()`
- `validate()`, `touch()`

Register the type-string in `index_names.h`. Add planner support in `src/mongo/db/query/planner_*.cpp` if the new type needs to be selected by the optimizer.

### Pattern F — Add a match-expression operator

Add a value to the `MatchType` enum in `src/mongo/db/matcher/expression.h`. Subclass `MatchExpression` (typically through `LeafMatchExpression`, `PathMatchExpression`, etc.) in `src/mongo/db/matcher/expression_<group>.{h,cpp}`. Wire the operator string into `ExpressionParser::parse`.

### Pattern G — Add a collection option

1. Add the field to `struct CollectionOptions` in `src/mongo/db/catalog/collection_options.h`.
2. Add parse logic in `CollectionOptions::parse()` (`collection_options.cpp`).
3. Add serialization in `appendBSON()`.
4. Add storage-engine plumbing if the option affects on-disk layout (this is typically what makes options Tier 2 or Tier 3).

---

## EloqDoc-specific deviations from stock MongoDB 4.0.3

These constraints affect **how** features are implemented, not whether they can be:

1. **No global locking.** `src/mongo/db/concurrency/eloq_locker_noop.h` is a no-op; concurrency is handled by Data Substrate's MVCC. Code that asserts on lock state, schedules around lock acquisition, or assumes lock-ordering invariants must be reviewed when ported.
2. **No oplog-based replication.** Anything that hooks into `OpObserver` to emit oplog entries (e.g. plan-cache invalidation, change-stream events) needs adaptation to Data Substrate's log service. Don't assume an oplog exists.
3. **Storage engine is EloqStore (RocksDB-derived) + S3 + memory cache**, not WiredTiger. Index implementations must speak the EloqDoc storage abstractions (see `src/mongo/db/modules/eloq/eloq_kv_engine.{h,cpp}`, `eloq_record_store.{h,cpp}`, `eloq_index.{h,cpp}`).
4. **No replica set / no `mongos` sharding layer.** Distributed semantics are provided by Data Substrate. Commands that exist purely for replica-set or sharding coordination (e.g. `voteCommitIndexBuild`, `setIndexCommitQuorum`) can usually be parser-only stubs or no-ops.
5. **`commitQuorum` simplifies dramatically.** Per `analysis_cc/forward_compat_eval.md`, the parser-only "primary-only" semantics suffices — no two-phase coordination needed.
6. **Custom storage engine code lives under `src/mongo/db/modules/eloq/`.** Storage-touching backports (clustered collections, columnstore, time-series) MUST be reviewed against this directory's interfaces.

---

## Reuse before re-implementing

Before starting any task, search for partial implementations:

```bash
# Aggregation stages
grep -r "REGISTER_DOCUMENT_SOURCE\|REGISTER_MULTI_STAGE_ALIAS" src/mongo/db/pipeline/

# Accumulators
grep -r "REGISTER_ACCUMULATOR" src/mongo/db/pipeline/

# Expressions
grep -r "REGISTER_EXPRESSION" src/mongo/db/pipeline/

# Index types
ls src/mongo/db/index/*_access_method.h

# Commands
ls src/mongo/db/commands/
```

If a feature is already half-implemented (sometimes a stub exists from upstream merges), the task spec says so. Otherwise assume it's not present.
