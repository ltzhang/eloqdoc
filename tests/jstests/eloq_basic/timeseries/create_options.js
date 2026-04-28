(function() {
    "use strict";

    const dbName = "eloq_ts_create_options";
    const testDB = db.getSiblingDB(dbName);
    assert.commandWorked(testDB.dropDatabase());

    assert.commandWorked(testDB.createCollection("metrics", {
        timeseries: {timeField: "t", metaField: "tags", granularity: "minutes"},
        expireAfterSeconds: 3600
    }));

    const infos = testDB.getCollectionInfos({name: "metrics"});
    assert.eq(1, infos.length, tojson(infos));
    assert.eq("t", infos[0].options.timeseries.timeField, tojson(infos[0]));
    assert.eq("tags", infos[0].options.timeseries.metaField, tojson(infos[0]));
    assert.eq("minutes", infos[0].options.timeseries.granularity, tojson(infos[0]));
    assert.eq(3600, infos[0].options.expireAfterSeconds, tojson(infos[0]));

    assert.commandFailedWithCode(
        testDB.createCollection("badMissingTimeField", {timeseries: {metaField: "tags"}}),
        ErrorCodes.InvalidOptions);

    assert.commandFailedWithCode(
        testDB.createCollection("badTimeFieldType", {timeseries: {timeField: 5}}),
        ErrorCodes.TypeMismatch);

    assert.commandFailedWithCode(
        testDB.createCollection("badGranularity",
                                {timeseries: {timeField: "t", granularity: "weeks"}}),
        ErrorCodes.InvalidOptions);
})();
