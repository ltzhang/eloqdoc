(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_collmod_granularity");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {
        timeseries: {timeField: "t", metaField: "tags", granularity: "seconds"}
    }));

    assert.commandWorked(testDB.runCommand({collMod: "metrics", timeseries: {granularity: "minutes"}}));
    let infos = testDB.getCollectionInfos({name: "metrics"});
    assert.eq(1, infos.length, tojson(infos));
    assert.eq("minutes", infos[0].options.timeseries.granularity, tojson(infos[0]));

    assert.commandWorked(testDB.runCommand({collMod: "metrics", timeseries: {granularity: "hours"}}));
    infos = testDB.getCollectionInfos({name: "metrics"});
    assert.eq("hours", infos[0].options.timeseries.granularity, tojson(infos[0]));

    assert.commandFailedWithCode(
        testDB.runCommand({collMod: "metrics", timeseries: {granularity: "seconds"}}),
        ErrorCodes.InvalidOptions);
    assert.commandFailedWithCode(
        testDB.runCommand({collMod: "metrics", timeseries: {granularity: "weeks"}}),
        ErrorCodes.InvalidOptions);
    assert.commandFailedWithCode(
        testDB.runCommand({collMod: "metrics", timeseries: {granularity: "hours", timeField: "ts"}}),
        ErrorCodes.InvalidOptions);
    assert.commandFailedWithCode(
        testDB.runCommand({collMod: "metrics", timeseries: {granularity: "hours", metaField: "m"}}),
        ErrorCodes.InvalidOptions);
    assert.commandFailedWithCode(
        testDB.runCommand({collMod: "metrics", timeseries: {granularity: "hours", bucketMaxSpanSeconds: 3600}}),
        ErrorCodes.InvalidOptions);

    assert.commandWorked(testDB.createCollection("ordinary"));
    assert.commandFailedWithCode(
        testDB.runCommand({collMod: "ordinary", timeseries: {granularity: "minutes"}}),
        ErrorCodes.InvalidOptions);

    assert.commandWorked(testDB.createCollection("customBuckets", {
        timeseries: {
            timeField: "t",
            bucketMaxSpanSeconds: 3600,
            bucketRoundingSeconds: 3600
        }
    }));
    assert.commandFailedWithCode(
        testDB.runCommand({collMod: "customBuckets", timeseries: {granularity: "minutes"}}),
        ErrorCodes.InvalidOptions);
})();
