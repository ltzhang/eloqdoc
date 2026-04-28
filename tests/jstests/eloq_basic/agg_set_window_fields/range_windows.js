const coll = db.agg_set_window_fields_range_windows;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, sensor: "a", t: 1, value: 10},
    {_id: 2, sensor: "a", t: 2, value: 20},
    {_id: 3, sensor: "a", t: 4, value: 40},
    {_id: 4, sensor: "b", t: 2, value: 5},
    {_id: 5, sensor: "b", t: 5, value: 15},
]));

const result = coll.aggregate([
    {
        $setWindowFields: {
            partitionBy: "$sensor",
            sortBy: {t: 1},
            output: {
                recentSum: {$sum: "$value", window: {range: [-2, 0]}},
                nearAvg: {$avg: "$value", window: {range: [-1, 1]}},
                seenCount: {$count: {}, window: {range: ["unbounded", "current"]}},
            },
        },
    },
    {$project: {_id: 0, sensor: 1, t: 1, value: 1, recentSum: 1, nearAvg: 1, seenCount: 1}},
]).toArray();

assert.eq([
    {sensor: "a", t: 1, value: 10, recentSum: 10, nearAvg: 15, seenCount: 1},
    {sensor: "a", t: 2, value: 20, recentSum: 30, nearAvg: 15, seenCount: 2},
    {sensor: "a", t: 4, value: 40, recentSum: 60, nearAvg: 40, seenCount: 3},
    {sensor: "b", t: 2, value: 5, recentSum: 5, nearAvg: 5, seenCount: 1},
    {sensor: "b", t: 5, value: 15, recentSum: 15, nearAvg: 15, seenCount: 2},
], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            partitionBy: "$sensor",
            sortBy: {t: 1, _id: 1},
            output: {bad: {$sum: "$value", window: {range: [-1, 0]}}},
        },
    }],
    cursor: {},
}));
