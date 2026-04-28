(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_create_buckets");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {timeseries: {timeField: "t"}}));

    const logical = testDB.getCollectionInfos({name: "metrics"});
    const buckets = testDB.getCollectionInfos({name: "system.buckets.metrics"});
    assert.eq(1, logical.length, tojson(logical));
    assert.eq(1, buckets.length, tojson(buckets));

    assert.commandFailed(
        testDB.createCollection("system.buckets.user", {timeseries: {timeField: "t"}}));
})();
