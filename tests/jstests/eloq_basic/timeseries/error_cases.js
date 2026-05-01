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
    assert.commandFailed(testDB.getCollection("system.buckets.metrics").createIndex({v: 1}));
    assert.commandFailed(testDB.runCommand({dropIndexes: "system.buckets.metrics", index: "*"}));
    assert.commandFailed(testDB.runCommand({
        collMod: "system.buckets.metrics",
        index: {name: "_id_", hidden: true}
    }));

    assert.commandWorked(testDB.metrics.insert({t: ISODate("2025-01-01T00:00:00Z"), v: 1}));
    assert.commandWorked(testDB.metrics.update({v: 1}, {$set: {v: 2}}));
    assert.eq(2, testDB.metrics.findOne().v);
    assert.commandWorked(testDB.metrics.remove({v: 2}));
    assert.eq(0, testDB.metrics.find({}).itcount());
})();
