# Time-Series Basic Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a Tier 2 MVP for MongoDB-style time-series collections: create, insert, find/aggregate, listCollections visibility, and bucket-level TTL.

**Architecture:** Store each user time-series collection as catalog metadata plus a physical `system.buckets.<collection>` collection. Writes into the logical namespace are intercepted and packed into bucket documents; reads are translated to the bucket namespace, optionally pruned by bucket control fields, unpacked by `$_internalUnpackBucket`, and then filtered using the original measurement predicate.

**Tech Stack:** MongoDB 4.0.3 C++ catalog/command/write/pipeline code, EloqDoc storage engine, SCons, resmoke-style jstests under `tests/jstests/eloq_basic/timeseries/`.

---

## Scope

This plan implements the basic Tier 2 slice from `docs/analysis/final/tasks/time-series-collections.md`.

In scope:
- `db.createCollection("m", {timeseries: {timeField: "t"}})` with `timeField`, optional `metaField`, optional `granularity`, and optional `expireAfterSeconds`.
- Creation of `system.buckets.m` as the physical collection.
- `listCollections` exposing the logical collection options and the buckets collection.
- Inserts into `m` routed to bucket documents.
- Basic unpacking through `$_internalUnpackBucket`.
- `find` and `aggregate` on `m` returning measurement documents.
- Bucket-level TTL that removes buckets whose `control.max.<timeField>` is older than `expireAfterSeconds`.

Deferred to later phases:
- Measurement updates and deletes.
- Secondary indexes over measurement fields and meta fields.
- Bucket compression, bucket reopening after restart, sharded time-series, clustered physical layout, and full upstream planner parity.

## Design Decisions

- Use collection options, not a normal view, as the authoritative logical time-series marker. Normal views reject writes, while time-series collections must accept inserts.
- Create buckets as ordinary collections first. Clustered collection support can improve storage locality later without blocking correctness.
- Keep the bucket catalog deliberately conservative: one open bucket per `(namespace, meta value, rounded time)` and a small fixed document limit. This avoids hard storage-side dependencies.
- For MVP read routing, prefer aggregation translation over teaching the find executor to unpack buckets directly. `find` can be converted to an equivalent aggregation in the command layer.
- Store `expireAfterSeconds` as a time-series collection option. In this 4.0.3 tree it is otherwise index metadata only, so collection-level time-series TTL must be added explicitly.
- Implement bucket TTL with a dedicated `src/mongo/db/ttl.cpp` scan path over time-series collection options. Do not rely on the existing TTL-index cache for collection-level time-series TTL.
- Use clean-room implementation only. Newer MongoDB source may inform externally visible behavior and test expectations, but implementation must be adapted independently to this 4.0.3 tree.

## File Map

- Modify `src/mongo/db/catalog/collection_options.h`: add `TimeseriesOptions`, `expireAfterSeconds`, and helper predicates.
- Modify `src/mongo/db/catalog/collection_options.cpp`: parse, validate, serialize, and compare `timeseries` and `expireAfterSeconds`.
- Create `src/mongo/db/timeseries/timeseries_options.h`.
- Create `src/mongo/db/timeseries/timeseries_options.cpp`.
- Create `src/mongo/db/timeseries/timeseries_namespace.h`.
- Create `src/mongo/db/timeseries/timeseries_namespace.cpp`.
- Create `src/mongo/db/timeseries/bucket_catalog.h`.
- Create `src/mongo/db/timeseries/bucket_catalog.cpp`.
- Create `src/mongo/db/timeseries/insert_router.h`.
- Create `src/mongo/db/timeseries/insert_router.cpp`.
- Create `src/mongo/db/timeseries/query_translator.h`.
- Create `src/mongo/db/timeseries/query_translator.cpp`.
- Modify `src/mongo/db/SConscript`: add the new time-series library to the server build.
- Modify `src/mongo/db/catalog/create_collection.cpp`: create buckets collection beside logical collection.
- Modify `src/mongo/db/commands/dbcommands.cpp`: reject invalid `timeseries` command combinations before create.
- Modify `src/mongo/db/commands/list_collections.cpp`: ensure `timeseries` remains visible in options.
- Modify `src/mongo/db/ops/write_ops_exec.cpp`: route inserts targeting logical time-series collections.
- Create `src/mongo/db/pipeline/document_source_internal_unpack_bucket.h`.
- Create `src/mongo/db/pipeline/document_source_internal_unpack_bucket.cpp`.
- Create `src/mongo/db/pipeline/document_source_internal_unpack_bucket_test.cpp`.
- Modify `src/mongo/db/pipeline/SConscript`: add the new pipeline stage and unit test.
- Modify `src/mongo/db/commands/find_cmd.cpp`: translate finds on time-series collections to aggregate commands.
- Modify `src/mongo/db/commands/pipeline_command.cpp` or `src/mongo/db/commands/run_aggregate.cpp`: translate top-level aggregate commands on time-series collections.
- Modify `src/mongo/db/ttl.cpp`: add a collection-option time-series bucket expiration pass beside the existing TTL-index pass.
- Create tests under `tests/jstests/eloq_basic/timeseries/`.

## Build And Test Commands

Use `-j16` for all builds:

```bash
python scripts/buildscripts/scons.py install-core --dbg=on -j16
```

Run focused jstests with the repository's local runbook settings:

```bash
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/create_options.js
```

Run focused C++ unit tests when added:

```bash
python scripts/buildscripts/scons.py build/opt/mongo/db/pipeline/document_source_internal_unpack_bucket_test -j16
```

Expected passing output for test commands is exit code 0 with no failed tests.

---

### Task 1: Parse And Persist Time-Series Options

**Files:**
- Modify: `src/mongo/db/catalog/collection_options.h`
- Modify: `src/mongo/db/catalog/collection_options.cpp`
- Create: `src/mongo/db/timeseries/timeseries_options.h`
- Create: `src/mongo/db/timeseries/timeseries_options.cpp`
- Modify: `src/mongo/db/SConscript`
- Test: `tests/jstests/eloq_basic/timeseries/create_options.js`

- [ ] **Step 1: Add the failing jstest**

Create `tests/jstests/eloq_basic/timeseries/create_options.js`:

```javascript
(function() {
    "use strict";

    const dbName = "eloq_ts_create_options";
    const testDB = db.getSiblingDB(dbName);
    assert.commandWorked(testDB.dropDatabase());

    assert.commandWorked(testDB.createCollection("metrics", {
        timeseries: {timeField: "t", metaField: "tags", granularity: "minutes"},
        expireAfterSeconds: 3600
    }));

    const infos = testDB.getCollectionInfos({name: "metrics"});
    assert.eq(1, infos.length, tojson(infos));
    assert.eq("t", infos[0].options.timeseries.timeField, tojson(infos[0]));
    assert.eq("tags", infos[0].options.timeseries.metaField, tojson(infos[0]));
    assert.eq("minutes", infos[0].options.timeseries.granularity, tojson(infos[0]));
    assert.eq(3600, infos[0].options.expireAfterSeconds, tojson(infos[0]));

    assert.commandFailedWithCode(
        testDB.createCollection("badMissingTimeField", {timeseries: {metaField: "tags"}}),
        ErrorCodes.InvalidOptions);

    assert.commandFailedWithCode(
        testDB.createCollection("badTimeFieldType", {timeseries: {timeField: 5}}),
        ErrorCodes.TypeMismatch);

    assert.commandFailedWithCode(
        testDB.createCollection("badGranularity",
                                {timeseries: {timeField: "t", granularity: "weeks"}}),
        ErrorCodes.InvalidOptions);
})();
```

- [ ] **Step 2: Run the failing test**

Run:

```bash
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/create_options.js
```

Expected: FAIL because `timeseries` is rejected as an unknown collection option.

- [ ] **Step 3: Add option model helpers**

Create `src/mongo/db/timeseries/timeseries_options.h`:

```cpp
#pragma once

#include <string>

#include "mongo/base/status.h"
#include "mongo/db/jsobj.h"

namespace mongo {
namespace timeseries {

struct TimeseriesOptions {
    std::string timeField;
    std::string metaField;
    std::string granularity = "seconds";

    bool hasMetaField() const {
        return !metaField.empty();
    }
};

Status parseTimeseriesOptions(const BSONElement& elem, TimeseriesOptions* out);
void appendTimeseriesOptions(BSONObjBuilder* builder, const TimeseriesOptions& options);

}  // namespace timeseries
}  // namespace mongo
```

Create `src/mongo/db/timeseries/timeseries_options.cpp`:

```cpp
#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/timeseries_options.h"

#include "mongo/base/error_codes.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {
namespace timeseries {

Status parseTimeseriesOptions(const BSONElement& elem, TimeseriesOptions* out) {
    if (elem.type() != mongo::Object) {
        return {ErrorCodes::TypeMismatch, "'timeseries' has to be a document."};
    }

    TimeseriesOptions parsed;
    BSONForEach(field, elem.Obj()) {
        auto name = field.fieldNameStringData();
        if (name == "timeField") {
            if (field.type() != mongo::String) {
                return {ErrorCodes::TypeMismatch, "'timeseries.timeField' has to be a string."};
            }
            parsed.timeField = field.String();
        } else if (name == "metaField") {
            if (field.type() != mongo::String) {
                return {ErrorCodes::TypeMismatch, "'timeseries.metaField' has to be a string."};
            }
            parsed.metaField = field.String();
        } else if (name == "granularity") {
            if (field.type() != mongo::String) {
                return {ErrorCodes::TypeMismatch, "'timeseries.granularity' has to be a string."};
            }
            parsed.granularity = field.String();
        } else if (name == "bucketMaxSpanSeconds" || name == "bucketRoundingSeconds") {
            if (!field.isNumber()) {
                return {ErrorCodes::TypeMismatch,
                        str::stream() << "'timeseries." << name << "' has to be numeric."};
            }
        } else {
            return {ErrorCodes::InvalidOptions,
                    str::stream() << "'timeseries." << name << "' is not supported."};
        }
    }

    if (parsed.timeField.empty()) {
        return {ErrorCodes::InvalidOptions, "'timeseries.timeField' is required."};
    }

    if (parsed.granularity != "seconds" && parsed.granularity != "minutes" &&
        parsed.granularity != "hours") {
        return {ErrorCodes::InvalidOptions,
                "'timeseries.granularity' must be 'seconds', 'minutes', or 'hours'."};
    }

    *out = parsed;
    return Status::OK();
}

void appendTimeseriesOptions(BSONObjBuilder* builder, const TimeseriesOptions& options) {
    BSONObjBuilder ts(builder->subobjStart("timeseries"));
    ts.append("timeField", options.timeField);
    if (!options.metaField.empty()) {
        ts.append("metaField", options.metaField);
    }
    ts.append("granularity", options.granularity);
    ts.doneFast();
}

}  // namespace timeseries
}  // namespace mongo
```

- [ ] **Step 4: Wire options into `CollectionOptions`**

In `src/mongo/db/catalog/collection_options.h`, include the helper header and add the optional fields:

```cpp
#include "mongo/db/timeseries/timeseries_options.h"
```

```cpp
boost::optional<timeseries::TimeseriesOptions> timeseries;
boost::optional<long long> expireAfterSeconds;
```

In `CollectionOptions::parse()`, handle `timeseries` and `expireAfterSeconds` before the unknown-field rejection:

```cpp
} else if (fieldName == "timeseries") {
    timeseries::TimeseriesOptions parsed;
    Status status = timeseries::parseTimeseriesOptions(e, &parsed);
    if (!status.isOK()) {
        return status;
    }
    timeseries = parsed;
} else if (fieldName == "expireAfterSeconds") {
    if (!e.isNumber()) {
        return {ErrorCodes::TypeMismatch, "'expireAfterSeconds' has to be numeric."};
    }
    if (e.numberLong() < 0) {
        return {ErrorCodes::BadValue, "'expireAfterSeconds' must be non-negative."};
    }
    expireAfterSeconds = e.numberLong();
```

After the parse loop, reject collection-level `expireAfterSeconds` unless this is a time-series collection:

```cpp
if (expireAfterSeconds && !timeseries) {
    return {ErrorCodes::InvalidOptions,
            "'expireAfterSeconds' is only supported with time-series collections."};
}
```

In `CollectionOptions::appendBSON()`, serialize it:

```cpp
if (timeseries) {
    timeseries::appendTimeseriesOptions(builder, *timeseries);
}
if (expireAfterSeconds) {
    builder->append("expireAfterSeconds", *expireAfterSeconds);
}
```

In `CollectionOptions::matchesStorageOptions()`, compare the three `timeseries` fields and `expireAfterSeconds` when either side has time-series options.

- [ ] **Step 5: Add library target**

Modify `src/mongo/db/SConscript` to build a `timeseries` library from:

```python
env.Library(
    target='timeseries',
    source=[
        'timeseries/timeseries_options.cpp',
    ],
    LIBDEPS=[
        '$BUILD_DIR/mongo/base',
        '$BUILD_DIR/mongo/db/common',
    ],
)
```

Add the library to the database/catalog command target that already links `collection_options.cpp`.

- [ ] **Step 6: Build and run the focused test**

Run:

```bash
python scripts/buildscripts/scons.py install-core --dbg=on -j16
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/create_options.js
```

Expected: both commands exit 0.

- [ ] **Step 7: Commit**

```bash
git add src/mongo/db/catalog/collection_options.h src/mongo/db/catalog/collection_options.cpp src/mongo/db/timeseries/timeseries_options.h src/mongo/db/timeseries/timeseries_options.cpp src/mongo/db/SConscript tests/jstests/eloq_basic/timeseries/create_options.js
git commit -m "Add time-series collection options"
```

### Task 2: Create Bucket Namespace Beside Logical Collection

**Files:**
- Create: `src/mongo/db/timeseries/timeseries_namespace.h`
- Create: `src/mongo/db/timeseries/timeseries_namespace.cpp`
- Modify: `src/mongo/db/catalog/create_collection.cpp`
- Modify: `src/mongo/db/commands/dbcommands.cpp`
- Test: `tests/jstests/eloq_basic/timeseries/create_buckets.js`

- [ ] **Step 1: Add the failing jstest**

Create `tests/jstests/eloq_basic/timeseries/create_buckets.js`:

```javascript
(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_create_buckets");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {timeseries: {timeField: "t"}}));

    const logical = testDB.getCollectionInfos({name: "metrics"});
    const buckets = testDB.getCollectionInfos({name: "system.buckets.metrics"});
    assert.eq(1, logical.length, tojson(logical));
    assert.eq(1, buckets.length, tojson(buckets));

    assert.commandFailed(testDB.createCollection("system.buckets.user",
                                                 {timeseries: {timeField: "t"}}));
})();
```

- [ ] **Step 2: Run the failing test**

Run:

```bash
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/create_buckets.js
```

Expected: FAIL because the buckets collection is not created.

- [ ] **Step 3: Add namespace helpers**

Create `src/mongo/db/timeseries/timeseries_namespace.h`:

```cpp
#pragma once

#include "mongo/db/namespace_string.h"

namespace mongo {
namespace timeseries {

NamespaceString makeBucketNamespace(const NamespaceString& logicalNss);
bool isBucketNamespace(const NamespaceString& nss);

}  // namespace timeseries
}  // namespace mongo
```

Create `src/mongo/db/timeseries/timeseries_namespace.cpp`:

```cpp
#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/timeseries_namespace.h"

namespace mongo {
namespace timeseries {

NamespaceString makeBucketNamespace(const NamespaceString& logicalNss) {
    return NamespaceString(logicalNss.db(), "system.buckets." + logicalNss.coll());
}

bool isBucketNamespace(const NamespaceString& nss) {
    return nss.coll().startsWith("system.buckets.");
}

}  // namespace timeseries
}  // namespace mongo
```

- [ ] **Step 4: Verify create-collection transaction behavior**

Before modifying create, verify EloqDoc's `Database::userCreateNS()` and storage-engine `createCollection()` behavior. This tree has an insert-path comment stating that "create collection operation commits transaction"; if two `createCollection()` calls cannot be made atomic, the implementation must be idempotent and able to repair a missing bucket collection.

Run:

```bash
rg -n "create collection operation commits transaction|createCollection\\(" src/mongo/db/modules/eloq src/mongo/db/catalog src/mongo/db/storage/kv
```

Expected: identify whether bucket creation can share the logical collection's `WriteUnitOfWork`. If not, keep Task 2's create path best-effort and add the reconciliation helper described in Step 5.

- [ ] **Step 5: Create bucket collection after logical collection**

In `src/mongo/db/catalog/create_collection.cpp`, reject time-series collection names under `system.buckets.*` before the `writeConflictRetry` lambda. `userAllowedCreateNS()` already rejects this namespace class, but keep this check near command validation as defense-in-depth:

```cpp
if (options["timeseries"]) {
    uassert(ErrorCodes::InvalidOptions,
            "time-series collections cannot be created under system.buckets",
            !timeseries::isBucketNamespace(nss));
}
```

After the logical collection is created and before the unit of work commits, check `collectionOptions.timeseries`. If present, create `timeseries::makeBucketNamespace(nss)` with ordinary `CollectionOptions` and `autoIndexId = NO` only if the buckets namespace does not already exist.

Implementation shape:

```cpp
if (collectionOptions.timeseries) {
    auto bucketNss = timeseries::makeBucketNamespace(nss);
    CollectionOptions bucketOptions;
    bucketOptions.autoIndexId = CollectionOptions::NO;
    if (!db->getCollection(opCtx, bucketNss)) {
        Collection* bucketColl =
            db->createCollection(opCtx, bucketNss.ns(), bucketOptions, false, BSONObj());
        uassert(ErrorCodes::OperationFailed,
                str::stream() << "failed to create bucket collection " << bucketNss.ns(),
                bucketColl);
    }
}
```

Add a helper such as `timeseries::ensureBucketCollection()` and call it from:
- the explicit create path in this task;
- the insert router before writing buckets;
- the read translator before querying buckets, or fail with a clear repairable error.

If Step 4 confirms that logical and bucket collection creation are not atomic in EloqDoc, add a startup or first-use reconciliation pass that scans collections with `options.timeseries`, creates missing `system.buckets.<name>` collections, and leaves existing buckets untouched.

- [ ] **Step 6: Build and run create tests**

Run:

```bash
python scripts/buildscripts/scons.py install-core --dbg=on -j16
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/create_options.js tests/jstests/eloq_basic/timeseries/create_buckets.js
```

Expected: exit 0.

- [ ] **Step 7: Commit**

```bash
git add src/mongo/db/timeseries/timeseries_namespace.h src/mongo/db/timeseries/timeseries_namespace.cpp src/mongo/db/catalog/create_collection.cpp src/mongo/db/commands/dbcommands.cpp src/mongo/db/SConscript tests/jstests/eloq_basic/timeseries/create_buckets.js
git commit -m "Create bucket collections for time-series namespaces"
```

### Task 3: Implement Conservative Bucket Inserts

**Files:**
- Create: `src/mongo/db/timeseries/bucket_catalog.h`
- Create: `src/mongo/db/timeseries/bucket_catalog.cpp`
- Create: `src/mongo/db/timeseries/insert_router.h`
- Create: `src/mongo/db/timeseries/insert_router.cpp`
- Modify: `src/mongo/db/ops/write_ops_exec.cpp`
- Modify: `src/mongo/db/SConscript`
- Test: `tests/jstests/eloq_basic/timeseries/insert_buckets.js`

- [ ] **Step 1: Add the failing jstest**

Create `tests/jstests/eloq_basic/timeseries/insert_buckets.js`:

```javascript
(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_insert_buckets");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {
        timeseries: {timeField: "t", metaField: "tags", granularity: "minutes"}
    }));

    const docs = [
        {t: ISODate("2025-01-01T00:00:01Z"), tags: {host: "a"}, v: 1},
        {t: ISODate("2025-01-01T00:00:05Z"), tags: {host: "a"}, v: 2},
        {t: ISODate("2025-01-01T00:01:01Z"), tags: {host: "b"}, v: 3}
    ];
    assert.commandWorked(testDB.metrics.insertMany(docs));

    assert.eq(0, testDB.metrics.count({}), "logical namespace should not store raw docs directly");
    const bucketDocs = testDB.system.buckets.metrics.find().toArray();
    assert.gte(bucketDocs.length, 2, tojson(bucketDocs));
    bucketDocs.forEach(bucket => {
        assert(bucket.control, tojson(bucket));
        assert(bucket.control.min.t, tojson(bucket));
        assert(bucket.control.max.t, tojson(bucket));
        assert(bucket.data.t, tojson(bucket));
    });
})();
```

- [ ] **Step 2: Run the failing test**

Run:

```bash
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/insert_buckets.js
```

Expected: FAIL because inserts currently go to the logical collection.

- [ ] **Step 3: Add bucket catalog interface**

Create `bucket_catalog.h` with a process-local catalog keyed by namespace, serialized meta BSON, and rounded time:

```cpp
#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

#include "mongo/db/jsobj.h"
#include "mongo/db/namespace_string.h"

namespace mongo {
namespace timeseries {

struct BucketKey {
    NamespaceString logicalNss;
    BSONObj meta;
    long long roundedMillis = 0;
};

struct BucketHandle {
    OID id;
    std::size_t count = 0;
};

class BucketCatalog {
public:
    static BucketCatalog& get();
    BucketHandle getOrCreateBucket(const BucketKey& key);
    void recordInsert(const BucketKey& key);

private:
    std::mutex _mutex;
    std::unordered_map<std::string, BucketHandle> _openBuckets;
};

}  // namespace timeseries
}  // namespace mongo
```

In `bucket_catalog.cpp`, build `_openBuckets` keys from namespace, rounded time, and a canonicalized meta object. Do not append `BSONObj::objdata()` directly for object-valued meta fields because BSON field order is not canonical. A simple MVP helper is:

```cpp
BSONObj canonicalizeMetaForKey(const BSONObj& meta) {
    std::map<std::string, BSONElement> fields;
    BSONForEach(elem, meta) {
        fields.emplace(elem.fieldName(), elem);
    }

    BSONObjBuilder bob;
    for (const auto& entry : fields) {
        bob.appendAs(entry.second, entry.first);
    }
    return bob.obj();
}
```

Use `canonicalizeMetaForKey(meta).toString(false)` or an equivalent stable serialization when forming the string key. This intentionally treats `{a: 1, b: 2}` and `{b: 2, a: 1}` as the same bucket meta value for MVP routing.

- [ ] **Step 4: Implement insert routing**

Create `insert_router.h/cpp` with:

```cpp
Status routeInsert(OperationContext* opCtx,
                   const NamespaceString& logicalNss,
                   const CollectionOptions& options,
                   const std::vector<InsertStatement>& input,
                   std::vector<InsertStatement>* bucketStatements);
```

Behavior:
- Require `timeField` to exist and be BSON Date.
- Extract `metaField` value when configured; otherwise use an empty object.
- Round dates by granularity: seconds = 1000 ms, minutes = 60000 ms, hours = 3600000 ms.
- Build bucket documents with:

```javascript
{
  _id: ObjectId(),
  control: {version: 1, min: {t: date}, max: {t: date}, count: N},
  meta: <meta field value>,
  data: {
    t: {"0": ISODate(...)},
    v: {"0": 1}
  }
}
```

For the first MVP, each input document may produce a replacement bucket document with one measurement if in-place bucket update is risky. If updating existing buckets is implemented in the same task, keep `control.count <= 100` and serialize per-bucket modifications with `BucketCatalog`.

- [ ] **Step 5: Intercept inserts**

In `write_ops_exec.cpp`, after acquiring the target collection and before `insertDocuments()`, read its `CollectionOptions`. If `options.timeseries` is present:
- Resolve the buckets namespace.
- Call `timeseries::ensureBucketCollection()` so crash recovery from a missing buckets namespace is automatic on first use.
- Acquire the buckets collection directly with `AutoGetCollection(opCtx, bucketNss, MODE_IX)`.
- Convert measurement statements to bucket statements through `routeInsert()`.
- Insert into the buckets collection by calling `collection->insertDocuments(...)` inside the current insert path's write-conflict handling and a `WriteUnitOfWork`.
- Report one successful result per input measurement.

Do not recursively call `performInserts()` for the routed bucket write. `performInserts()` begins with `userAllowedWriteNS(wholeOp.getNamespace())`, and `userAllowedWriteNS()` delegates to `userAllowedCreateNS()`, which rejects `system.buckets.*` because it is not on the explicit `system.*` allowlist in `src/mongo/db/ops/insert.cpp`.

Reject direct client inserts into `system.buckets.*` unless they come from internal routing.

- [ ] **Step 6: Build and run insert tests**

Run:

```bash
python scripts/buildscripts/scons.py install-core --dbg=on -j16
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/create_buckets.js tests/jstests/eloq_basic/timeseries/insert_buckets.js
```

Expected: exit 0.

- [ ] **Step 7: Commit**

```bash
git add src/mongo/db/timeseries/bucket_catalog.h src/mongo/db/timeseries/bucket_catalog.cpp src/mongo/db/timeseries/insert_router.h src/mongo/db/timeseries/insert_router.cpp src/mongo/db/ops/write_ops_exec.cpp src/mongo/db/SConscript tests/jstests/eloq_basic/timeseries/insert_buckets.js
git commit -m "Route time-series inserts into bucket collections"
```

### Task 4: Add `$_internalUnpackBucket`

**Files:**
- Create: `src/mongo/db/pipeline/document_source_internal_unpack_bucket.h`
- Create: `src/mongo/db/pipeline/document_source_internal_unpack_bucket.cpp`
- Create: `src/mongo/db/pipeline/document_source_internal_unpack_bucket_test.cpp`
- Modify: `src/mongo/db/pipeline/SConscript`
- Test: `tests/jstests/eloq_basic/timeseries/unpack_bucket.js`

- [ ] **Step 1: Add the failing jstest**

Create `tests/jstests/eloq_basic/timeseries/unpack_bucket.js`:

```javascript
(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_unpack_bucket");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.system.buckets.metrics.insert({
        control: {
            version: 1,
            min: {t: ISODate("2025-01-01T00:00:01Z")},
            max: {t: ISODate("2025-01-01T00:00:02Z")},
            count: 2
        },
        meta: {host: "a"},
        data: {
            t: {"0": ISODate("2025-01-01T00:00:01Z"), "1": ISODate("2025-01-01T00:00:02Z")},
            v: {"0": 10, "1": 11}
        }
    }));

    const out = testDB.system.buckets.metrics.aggregate([
        {$_internalUnpackBucket: {timeField: "t", metaField: "tags"}}
    ]).toArray();

    assert.eq([
        {t: ISODate("2025-01-01T00:00:01Z"), tags: {host: "a"}, v: 10},
        {t: ISODate("2025-01-01T00:00:02Z"), tags: {host: "a"}, v: 11}
    ], out);
})();
```

- [ ] **Step 2: Run the failing test**

Run:

```bash
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/unpack_bucket.js
```

Expected: FAIL with invalid pipeline operator.

- [ ] **Step 3: Implement the stage**

Create a `DocumentSource` modeled after existing single-source stages such as `document_source_project.cpp`:
- Register `$_internalUnpackBucket`.
- Parse object spec with required `timeField` and optional `metaField`.
- On each input bucket, materialize measurement documents in `control.count` index order.
- Copy `meta` to the configured `metaField` path.
- Read every `data.<field>.<index>` value into each output measurement.
- Skip malformed buckets with a user-visible parse error only when the stage is directly executing that malformed document.

Core state:

```cpp
std::deque<Document> _pending;
std::string _timeField;
std::string _metaField;
```

Core `getNext()` behavior:

```cpp
if (!_pending.empty()) {
    auto out = _pending.front();
    _pending.pop_front();
    return out;
}

auto input = pSource->getNext();
if (!input.isAdvanced()) {
    return input;
}

unpackBucket(input.releaseDocument(), &_pending);
```

- [ ] **Step 4: Add C++ unit coverage**

Add `document_source_internal_unpack_bucket_test.cpp` with tests for:
- Two measurements unpacked in order.
- Meta field omitted when no `metaField` option is given.
- Sparse data field missing an index does not invent a value.

- [ ] **Step 5: Build and run unpack tests**

Run:

```bash
python scripts/buildscripts/scons.py build/opt/mongo/db/pipeline/document_source_internal_unpack_bucket_test -j16
python scripts/buildscripts/scons.py install-core --dbg=on -j16
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/unpack_bucket.js
```

Expected: exit 0.

- [ ] **Step 6: Commit**

```bash
git add src/mongo/db/pipeline/document_source_internal_unpack_bucket.h src/mongo/db/pipeline/document_source_internal_unpack_bucket.cpp src/mongo/db/pipeline/document_source_internal_unpack_bucket_test.cpp src/mongo/db/pipeline/SConscript tests/jstests/eloq_basic/timeseries/unpack_bucket.js
git commit -m "Add internal bucket unpack stage"
```

### Task 5: Translate Aggregate And Find Reads

**Files:**
- Create: `src/mongo/db/timeseries/query_translator.h`
- Create: `src/mongo/db/timeseries/query_translator.cpp`
- Modify: `src/mongo/db/commands/pipeline_command.cpp` or `src/mongo/db/commands/run_aggregate.cpp`
- Modify: `src/mongo/db/commands/find_cmd.cpp`
- Modify: `src/mongo/db/SConscript`
- Test: `tests/jstests/eloq_basic/timeseries/find_and_aggregate.js`

- [ ] **Step 1: Add the failing jstest**

Create `tests/jstests/eloq_basic/timeseries/find_and_aggregate.js`:

```javascript
(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_find_aggregate");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {
        timeseries: {timeField: "t", metaField: "tags", granularity: "minutes"}
    }));

    assert.commandWorked(testDB.metrics.insertMany([
        {t: ISODate("2025-01-01T00:00:01Z"), tags: {host: "a"}, v: 1},
        {t: ISODate("2025-01-01T00:00:05Z"), tags: {host: "a"}, v: 2},
        {t: ISODate("2025-01-01T00:01:01Z"), tags: {host: "b"}, v: 3}
    ]));

    assert.eq([2], testDB.metrics.find({t: {$gte: ISODate("2025-01-01T00:00:05Z")}})
                                .sort({v: 1})
                                .map(doc => doc.v));

    assert.eq([1, 2], testDB.metrics.find({"tags.host": "a"}).sort({v: 1}).map(doc => doc.v));

    const grouped = testDB.metrics.aggregate([
        {$match: {"tags.host": "a"}},
        {$group: {_id: "$tags.host", total: {$sum: "$v"}}}
    ]).toArray();
    assert.eq([{_id: "a", total: 3}], grouped);
})();
```

- [ ] **Step 2: Run the failing test**

Run:

```bash
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/find_and_aggregate.js
```

Expected: FAIL because logical reads still target the empty logical collection.

- [ ] **Step 3: Implement translator helpers**

Create `query_translator.h/cpp` with:

```cpp
std::vector<BSONObj> makeBucketPipeline(const CollectionOptions& options,
                                        const BSONObj& originalMatch,
                                        const std::vector<BSONObj>& userPipeline);

BSONObj makeFindAsAggregateCommand(const NamespaceString& logicalNss,
                                   const CollectionOptions& options,
                                   const BSONObj& findCmd);
```

Translation shape:

```javascript
[
  {$match: {"control.max.<timeField>": {$gte: lowerBound},
            "control.min.<timeField>": {$lte: upperBound},
            "meta.<subpath>": <meta predicate>}},
  {$_internalUnpackBucket: {timeField: "<timeField>", metaField: "<metaField>"}},
  {$match: <original measurement predicate>},
  ...userPipeline
]
```

Only build coarse bucket predicates for simple time comparisons and `metaField` equality in this task. Always keep the original `$match` after unpacking for correctness.

- [ ] **Step 4: Route aggregate commands**

In the aggregate command path, after parsing `AggregationRequest` and acquiring enough catalog state to inspect the namespace, if the target collection has `CollectionOptions::timeseries`:
- Call `timeseries::ensureBucketCollection()` or verify the buckets namespace exists before translation, so non-atomic create recovery works on first read as well as first write.
- Change the aggregation namespace to `system.buckets.<name>`.
- Prefix the pipeline with the coarse `$match` and `$_internalUnpackBucket`.
- Preserve the user's original pipeline after unpack.
- Run `runAggregate()` against the buckets namespace.

- [ ] **Step 5: Route find commands**

In `find_cmd.cpp`, after parsing the find request and before normal executor planning, detect a time-series collection. Build an aggregate command with:
- `aggregate: "system.buckets.<name>"`
- `pipeline: <translated pipeline>`
- `cursor: <original cursor options>`

Preserve simple `sort`, `skip`, `limit`, and projection by using `QueryRequest::asAggregationCommand()` as the starting point, then translate that aggregation. As with aggregate, ensure the buckets collection exists before dispatching to `runAggregate()`.

- [ ] **Step 6: Build and run read tests**

Run:

```bash
python scripts/buildscripts/scons.py install-core --dbg=on -j16
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/find_and_aggregate.js
```

Expected: exit 0.

- [ ] **Step 7: Commit**

```bash
git add src/mongo/db/timeseries/query_translator.h src/mongo/db/timeseries/query_translator.cpp src/mongo/db/commands/pipeline_command.cpp src/mongo/db/commands/run_aggregate.cpp src/mongo/db/commands/find_cmd.cpp src/mongo/db/SConscript tests/jstests/eloq_basic/timeseries/find_and_aggregate.js
git commit -m "Translate time-series reads through bucket unpacking"
```

### Task 6: Add Bucket-Level TTL

**Files:**
- Modify: `src/mongo/db/ttl.cpp`
- Modify: `src/mongo/db/timeseries/timeseries_namespace.h`
- Modify: `src/mongo/db/timeseries/timeseries_namespace.cpp`
- Test: `tests/jstests/eloq_basic/timeseries/ttl.js`

- [ ] **Step 1: Confirm the active TTL implementation**

Run:

```bash
sed -n '77,285p' src/mongo/db/ttl.cpp
```

Expected: confirm that `TTLMonitor::doTTLPass()` currently gathers TTL indexes and that `doTTLForIndex()` deletes through an index descriptor. This verifies that collection-level time-series TTL needs a separate scan path.

- [ ] **Step 2: Add the failing jstest**

Create `tests/jstests/eloq_basic/timeseries/ttl.js`:

```javascript
(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_ttl");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {
        timeseries: {timeField: "t"},
        expireAfterSeconds: 1
    }));

    assert.commandWorked(testDB.metrics.insert({t: ISODate("2000-01-01T00:00:00Z"), v: 1}));

    assert.soon(function() {
        return testDB.system.buckets.metrics.count({}) === 0;
    }, "expired time-series bucket was not deleted", 120000);
})();
```

- [ ] **Step 3: Collect time-series TTL namespaces**

In `TTLMonitor::doTTLPass()`, while iterating all collections, read each collection's catalog options:

```cpp
auto options = coll->getCatalogEntry()->getCollectionOptions(&opCtx);
if (options.timeseries && options.expireAfterSeconds) {
    timeseriesTTLCollections.push_back(
        TimeseriesTTLCollection{collectionNSS, *options.timeseries, *options.expireAfterSeconds});
}
```

Keep this list separate from `ttlIndexes`. The existing `ttlIndexes` path is index-based and should remain unchanged for normal TTL indexes.

- [ ] **Step 4: Implement bucket expiration**

Add a helper in `src/mongo/db/ttl.cpp`:

```cpp
void doTTLForTimeseriesCollection(OperationContext* opCtx,
                                  const NamespaceString& logicalNss,
                                  const timeseries::TimeseriesOptions& options,
                                  long long expireAfterSeconds,
                                  int limit);
```

Behavior:
- Resolve `timeseries::makeBucketNamespace(logicalNss)`.
- Acquire the buckets collection with `AutoGetCollection(opCtx, bucketNss, MODE_IX)`.
- Delete buckets where `control.max.<timeField> < Date_t::now() - expireAfterSeconds`.
- Do not unpack buckets or delete individual measurements in this MVP.
- Do not require or create a TTL index on `control.max.<timeField>` in this task; keeping bucket internals out of `listIndexes` is preferred for the MVP.

Query shape:

```javascript
{"control.max.<timeField>": {$lt: expirationDate}}
```

The implementation can use a collection scan delete plan for the first slice. If an index is later added on the buckets collection, that is a performance follow-up and should be documented separately.

- [ ] **Step 5: Build and run TTL test**

Run:

```bash
python scripts/buildscripts/scons.py install-core --dbg=on -j16
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/ttl.js
```

Expected: exit 0.

- [ ] **Step 6: Commit**

```bash
git add tests/jstests/eloq_basic/timeseries/ttl.js
git add src/mongo/db/ttl.cpp src/mongo/db/timeseries/timeseries_namespace.h src/mongo/db/timeseries/timeseries_namespace.cpp
git commit -m "Expire time-series buckets by max time"
```

### Task 7: Hardening And Compatibility Tests

**Files:**
- Modify: `docs/analysis/final/tasks/time-series-collections.md`
- Create: `tests/jstests/eloq_basic/timeseries/error_cases.js`
- Create: `tests/jstests/eloq_basic/timeseries/basic_matrix.js`

- [ ] **Step 1: Add error-case tests**

Create `error_cases.js`:

```javascript
(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_error_cases");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {timeseries: {timeField: "t"}}));

    assert.commandFailed(testDB.metrics.insert({v: 1}));
    assert.commandFailed(testDB.metrics.insert({t: "2025-01-01T00:00:00Z", v: 1}));
    assert.commandFailed(testDB.system.buckets.metrics.insert({control: {}, data: {}}));
})();
```

- [ ] **Step 2: Add basic matrix test**

Create `basic_matrix.js`:

```javascript
(function() {
    "use strict";

    const granularities = ["seconds", "minutes", "hours"];
    granularities.forEach(granularity => {
        const testDB = db.getSiblingDB("eloq_ts_matrix_" + granularity);
        assert.commandWorked(testDB.dropDatabase());
        assert.commandWorked(testDB.createCollection("metrics", {
            timeseries: {timeField: "t", metaField: "tags", granularity: granularity}
        }));
        assert.commandWorked(testDB.metrics.insertMany([
            {t: ISODate("2025-01-01T00:00:00Z"), tags: {site: granularity}, v: 1},
            {t: ISODate("2025-01-01T00:00:01Z"), tags: {site: granularity}, v: 2}
        ]));
        assert.eq(2, testDB.metrics.find({}).itcount());
        assert.eq(2, testDB.metrics.find({"tags.site": granularity}).itcount());
    });
})();
```

- [ ] **Step 3: Update the task doc**

In `docs/analysis/final/tasks/time-series-collections.md`, add an implementation note recording the MVP deviations:
- Buckets are ordinary collections until clustered collection support is added.
- Bucket packing is conservative and optimized later.
- The first read path translates to aggregation rather than native find executor unpacking.
- TTL is bucket-level only.
- Explain output may expose `system.buckets.<collection>` until a later compatibility pass rewrites explain results back to the logical time-series namespace.

- [ ] **Step 4: Run the complete focused test set**

Run:

```bash
python scripts/buildscripts/scons.py install-core --dbg=on -j16
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/
```

Expected: exit 0.

- [ ] **Step 5: Commit**

```bash
git add docs/analysis/final/tasks/time-series-collections.md tests/jstests/eloq_basic/timeseries/error_cases.js tests/jstests/eloq_basic/timeseries/basic_matrix.js
git commit -m "Document time-series MVP limitations"
```

### Task 8: Final Verification And Push

**Files:**
- All time-series implementation files from Tasks 1-7.

- [ ] **Step 1: Run formatting and diff checks**

Run:

```bash
git diff --check
```

Expected: no output and exit 0.

- [ ] **Step 2: Run focused build**

Run:

```bash
python scripts/buildscripts/scons.py install-core --dbg=on -j16
```

Expected: exit 0.

- [ ] **Step 3: Run focused test suite**

Run:

```bash
python scripts/buildscripts/resmoke.py --suite=eloq_basic tests/jstests/eloq_basic/timeseries/
```

Expected: exit 0.

- [ ] **Step 4: Review status and push**

Run:

```bash
git status --short
git log --oneline --decorate -8
git push
```

Expected: clean worktree after commits, then successful push to the tracked branch.

## Residual Risks

- Insert routing touches the write path and must be reviewed carefully for transaction and write result accounting.
- Query translation must preserve command options such as batch size, limit, sort, projection, collation, and explain. The first implementation should support the common subset and fail clearly for unsupported combinations.
- The bucket catalog is in-memory. Restart correctness is preserved because existing buckets remain queryable, but efficient bucket reopening is deferred.
- Bucket updates may need storage-side help if full-document replacement creates unacceptable write amplification. This is a performance issue for the MVP unless it causes correctness failures.
- TTL implementation depends on the active TTL monitor shape in this branch and must be verified before editing.
