(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_update_measurements");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {
        timeseries: {timeField: "t", metaField: "tags", granularity: "minutes"}
    }));

    const metrics = testDB.metrics;
    assert.commandWorked(metrics.insertMany([
        {t: ISODate("2026-01-01T00:00:00Z"), tags: {host: "a"}, temp: 10},
        {t: ISODate("2026-01-01T00:01:00Z"), tags: {host: "a"}, temp: 20},
        {t: ISODate("2026-01-01T00:02:00Z"), tags: {host: "a"}, temp: 30}
    ]));

    assert.eq(3, metrics.find().itcount());
    assert.eq(1, testDB.getCollection("system.buckets.metrics").find().itcount());

    assert.eq(2, metrics.updateMany({temp: {$gte: 20}}, {$inc: {temp: 5}}).modifiedCount);
    assert.eq([10, 25, 35], metrics.find({}, {_id: 0, temp: 1}).sort({temp: 1}).toArray().map(
                              doc => doc.temp));

    let bucket = testDB.getCollection("system.buckets.metrics").findOne();
    assert.eq(3, bucket.control.count, tojson(bucket));
    assert.eq(10, bucket.control.min.temp, tojson(bucket));
    assert.eq(35, bucket.control.max.temp, tojson(bucket));

    assert.eq(1, metrics.updateOne({temp: 10}, {$set: {status: "ok"}}).modifiedCount);
    assert.eq("ok", metrics.findOne({temp: 10}).status);

    bucket = testDB.getCollection("system.buckets.metrics").findOne();
    assert.eq("ok", bucket.control.min.status, tojson(bucket));
    assert.eq("ok", bucket.control.max.status, tojson(bucket));

    assert.commandFailed(metrics.updateOne({temp: 10}, {$set: {tags: {host: "b"}}}));
})();
