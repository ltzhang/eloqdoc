(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_find_aggregate");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {
        timeseries: {timeField: "t", metaField: "tags", granularity: "minutes"}
    }));

    assert.commandWorked(testDB.metrics.insertMany([
        {t: ISODate("2025-01-01T00:00:01Z"), tags: {host: "a"}, v: 1},
        {t: ISODate("2025-01-01T00:00:05Z"), tags: {host: "a"}, v: 2},
        {t: ISODate("2025-01-01T00:01:01Z"), tags: {host: "b"}, v: 3}
    ]));

    assert.eq([2, 3],
              testDB.metrics.find({t: {$gte: ISODate("2025-01-01T00:00:05Z")}})
                  .sort({v: 1})
                  .map(doc => doc.v));

    assert.eq([1, 2], testDB.metrics.find({"tags.host": "a"}).sort({v: 1}).map(doc => doc.v));

    const grouped = testDB.metrics.aggregate([
        {$match: {"tags.host": "a"}},
        {$group: {_id: "$tags.host", total: {$sum: "$v"}}}
    ]).toArray();
    assert.eq([{_id: "a", total: 3}], grouped);
})();
