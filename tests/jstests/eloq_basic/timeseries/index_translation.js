(function() {
    "use strict";

    const testDB = db.getSiblingDB("eloq_ts_index_translation");
    assert.commandWorked(testDB.dropDatabase());
    assert.commandWorked(testDB.createCollection("metrics", {
        timeseries: {timeField: "t", metaField: "tags", granularity: "minutes"}
    }));

    const metrics = testDB.metrics;
    assert.commandWorked(metrics.createIndex({"tags.host": 1, t: -1, temp: 1}, {
        name: "host_time_temp"
    }));

    const bucketIndexes = testDB.getCollection("system.buckets.metrics").getIndexes();
    const bucketIndex = bucketIndexes.find((idx) => idx.name === "host_time_temp");
    assert.neq(undefined, bucketIndex, tojson(bucketIndexes));
    assert.eq({"meta.host": 1, "data.t": -1, "data.temp": 1}, bucketIndex.key, tojson(bucketIndex));
    assert.eq("eloq_ts_index_translation.system.buckets.metrics", bucketIndex.ns, tojson(bucketIndex));

    const logicalIndexes = metrics.getIndexes();
    const logicalIndex = logicalIndexes.find((idx) => idx.name === "host_time_temp");
    assert.neq(undefined, logicalIndex, tojson(logicalIndexes));
    assert.eq({"tags.host": 1, t: -1, temp: 1}, logicalIndex.key, tojson(logicalIndex));
    assert.eq("eloq_ts_index_translation.metrics", logicalIndex.ns, tojson(logicalIndex));
})();
