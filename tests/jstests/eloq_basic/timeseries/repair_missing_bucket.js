(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_repair_missing_bucket");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {timeseries: {timeField: "t"}}));

    assert.commandWorked(testDB.getCollection("system.buckets.metrics").drop());
    assert.commandWorked(testDB.metrics.insert({t: ISODate("2025-01-01T00:00:00Z"), v: 1}));
    assert.eq(1, testDB.getCollection("system.buckets.metrics").count({}));
    assert.eq(1, testDB.metrics.find({v: 1}).itcount());

    assert.commandWorked(testDB.getCollection("system.buckets.metrics").drop());
    assert.eq(0, testDB.metrics.find({}).itcount());
    assert.eq(1, testDB.getCollectionInfos({name: "system.buckets.metrics"}).length);
})();
