(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_unpack_bucket");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {
        timeseries: {timeField: "t", metaField: "tags", granularity: "minutes"}
    }));
    assert.commandWorked(testDB.metrics.insertMany([
        {t: ISODate("2025-01-01T00:00:01Z"), tags: {host: "a"}, v: 10},
        {t: ISODate("2025-01-01T00:00:02Z"), tags: {host: "a"}, v: 11}
    ]));

    const out = testDB.getCollection("system.buckets.metrics")
                    .aggregate([{$_internalUnpackBucket: {timeField: "t", metaField: "tags"}}])
                    .toArray()
                    .sort((lhs, rhs) => lhs.v - rhs.v);

    assert.docEq([
        {t: ISODate("2025-01-01T00:00:01Z"), tags: {host: "a"}, v: 10},
        {t: ISODate("2025-01-01T00:00:02Z"), tags: {host: "a"}, v: 11}
    ], out);
})();
