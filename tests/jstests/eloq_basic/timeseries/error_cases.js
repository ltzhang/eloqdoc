(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_error_cases");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {timeseries: {timeField: "t"}}));

    assert.commandFailed(testDB.metrics.insert({v: 1}));
    assert.commandFailed(testDB.metrics.insert({t: "2025-01-01T00:00:00Z", v: 1}));
    assert.commandFailed(testDB.getCollection("system.buckets.metrics").insert({
        control: {},
        data: {}
    }));
})();
