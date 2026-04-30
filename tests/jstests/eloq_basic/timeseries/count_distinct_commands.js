(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_count_distinct_commands");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {
        timeseries: {timeField: "t", metaField: "tags", granularity: "minutes"}
    }));

    assert.commandWorked(testDB.metrics.insertMany([
        {t: ISODate("2025-01-01T00:00:01Z"), tags: {host: "a", site: "north"}, v: 1},
        {t: ISODate("2025-01-01T00:00:05Z"), tags: {host: "a", site: "north"}, v: 2},
        {t: ISODate("2025-01-01T00:01:01Z"), tags: {host: "b", site: "south"}, v: 3}
    ]));

    assert.eq(3, testDB.metrics.count({}));
    assert.eq(2, testDB.metrics.count({"tags.site": "north"}));
    assert.eq(1, testDB.metrics.count({v: {$gte: 3}}));

    assert.eq(["a", "b"], testDB.metrics.distinct("tags.host").sort());
    assert.eq([1, 2], testDB.metrics.distinct("v", {"tags.site": "north"}).sort());
})();
