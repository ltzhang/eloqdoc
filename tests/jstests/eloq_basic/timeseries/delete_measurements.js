(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_delete_measurements");
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

    assert.eq(2, metrics.deleteMany({temp: {$gte: 20}}).deletedCount);
    assert.eq(1, metrics.find().itcount());
    assert.eq(10, metrics.findOne().temp);

    let buckets = testDB.getCollection("system.buckets.metrics").find().toArray();
    assert.eq(1, buckets.length, tojson(buckets));
    assert.eq(1, buckets[0].control.count, tojson(buckets[0]));
    assert.eq(10, buckets[0].control.min.temp, tojson(buckets[0]));
    assert.eq(10, buckets[0].control.max.temp, tojson(buckets[0]));

    assert.eq(1, metrics.deleteOne({temp: 10}).deletedCount);
    assert.eq(0, metrics.find().itcount());
    assert.eq(0, testDB.getCollection("system.buckets.metrics").find().itcount());
})();
