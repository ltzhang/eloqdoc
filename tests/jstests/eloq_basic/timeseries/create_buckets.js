(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_create_buckets");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {timeseries: {timeField: "t"}}));

    const logical = testDB.getCollectionInfos({name: "metrics"});
    assert.eq(1, logical.length, tojson(logical));
    assert.eq(0,
              testDB.getCollectionInfos({name: "system.buckets.metrics"}).length,
              "bucket collections are hidden from listCollections");
    assert.commandWorked(testDB.metrics.insert({t: ISODate("2025-01-01T00:00:00Z"), v: 1}));
    assert.eq(1, testDB.getCollection("system.buckets.metrics").find().itcount());

    assert.commandFailed(
        testDB.createCollection("system.buckets.user", {timeseries: {timeField: "t"}}));
})();
