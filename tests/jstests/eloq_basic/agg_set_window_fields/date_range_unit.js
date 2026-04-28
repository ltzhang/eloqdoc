const coll = db.agg_set_window_fields_date_range_unit;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, sensor: "a", ts: new Date(0), value: 1},
    {_id: 2, sensor: "a", ts: new Date(60000), value: 2},
    {_id: 3, sensor: "a", ts: new Date(180000), value: 3},
    {_id: 4, sensor: "b", ts: new Date(0), value: 10},
    {_id: 5, sensor: "b", ts: new Date(120000), value: 20},
]));

const result = coll.aggregate([
    {
        $setWindowFields: {
            partitionBy: "$sensor",
            sortBy: {ts: 1},
            output: {
                recent: {$sum: "$value", window: {range: [-1, 0], unit: "minute"}},
                nearby: {$count: {}, window: {range: [-2, 1], unit: "minute"}},
            },
        },
    },
    {$project: {_id: 0, sensor: 1, ts: 1, recent: 1, nearby: 1}},
]).toArray();

assert.eq([
    {sensor: "a", ts: new Date(0), recent: 1, nearby: 2},
    {sensor: "a", ts: new Date(60000), recent: 3, nearby: 2},
    {sensor: "a", ts: new Date(180000), recent: 3, nearby: 2},
    {sensor: "b", ts: new Date(0), recent: 10, nearby: 1},
    {sensor: "b", ts: new Date(120000), recent: 20, nearby: 2},
], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {ts: 1},
            output: {bad: {$sum: "$value", window: {range: [-1, 0], unit: "month"}}},
        },
    }],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {value: 1},
            output: {bad: {$sum: "$value", window: {range: [-1, 0], unit: "minute"}}},
        },
    }],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {ts: 1},
            output: {bad: {$sum: "$value", window: {documents: [-1, 0], unit: "minute"}}},
        },
    }],
    cursor: {},
}));
