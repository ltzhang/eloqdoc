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
        return testDB.getCollection("system.buckets.metrics").count({}) === 0;
    }, "expired time-series bucket was not deleted", 120000);
})();
