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
