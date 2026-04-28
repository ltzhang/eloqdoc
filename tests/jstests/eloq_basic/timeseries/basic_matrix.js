(function() {
    "use strict";

    const granularities = ["seconds", "minutes", "hours"];
    granularities.forEach(granularity => {
        const testDB = db.getSiblingDB("eloq_ts_matrix_" + granularity);
        assert.commandWorked(testDB.dropDatabase());
        assert.commandWorked(testDB.createCollection("metrics", {
            timeseries: {timeField: "t", metaField: "tags", granularity: granularity}
        }));
        assert.commandWorked(testDB.metrics.insertMany([
            {t: ISODate("2025-01-01T00:00:00Z"), tags: {site: granularity}, v: 1},
            {t: ISODate("2025-01-01T00:00:01Z"), tags: {site: granularity}, v: 2}
        ]));
        assert.eq(2, testDB.metrics.find({}).itcount());
        assert.eq(2, testDB.metrics.find({"tags.site": granularity}).itcount());
    });
})();
