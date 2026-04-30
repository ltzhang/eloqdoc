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
    assert.eq(2, bucketDocs.length, tojson(bucketDocs));

    const hostABucket = bucketDocs.find(bucket => bucket.meta && bucket.meta.host === "a");
    assert(hostABucket, tojson(bucketDocs));
    assert.eq(2, hostABucket.control.count, tojson(hostABucket));
    assert.eq(ISODate("2025-01-01T00:00:01Z"), hostABucket.control.min.t, tojson(hostABucket));
    assert.eq(ISODate("2025-01-01T00:00:05Z"), hostABucket.control.max.t, tojson(hostABucket));
    assert.eq(1, hostABucket.control.min.v, tojson(hostABucket));
    assert.eq(2, hostABucket.control.max.v, tojson(hostABucket));
    assert(!hostABucket.control.min.hasOwnProperty("tags"), tojson(hostABucket));
    assert(!hostABucket.control.max.hasOwnProperty("tags"), tojson(hostABucket));
    assert.eq(ISODate("2025-01-01T00:00:01Z"), hostABucket.data.t["0"], tojson(hostABucket));
    assert.eq(ISODate("2025-01-01T00:00:05Z"), hostABucket.data.t["1"], tojson(hostABucket));
    assert.eq(1, hostABucket.data.v["0"], tojson(hostABucket));
    assert.eq(2, hostABucket.data.v["1"], tojson(hostABucket));
    assert(!hostABucket.data.hasOwnProperty("tags"), tojson(hostABucket));
})();
