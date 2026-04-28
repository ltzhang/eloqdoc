const coll = db.agg_set_window_fields_exp_moving_avg;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, sensor: "a", t: 1, value: 10},
    {_id: 2, sensor: "a", t: 2, value: 20},
    {_id: 3, sensor: "a", t: 3, value: 30},
    {_id: 4, sensor: "a", t: 4, value: 40},
    {_id: 5, sensor: "b", t: 1, value: 100},
    {_id: 6, sensor: "b", t: 2, value: 50},
]));

const result = coll.aggregate([
    {
        $setWindowFields: {
            partitionBy: "$sensor",
            sortBy: {t: 1},
            output: {
                avgN: {$expMovingAvg: {input: "$value", N: 3}},
                avgAlpha: {$expMovingAvg: {input: "$value", alpha: 0.25}},
            },
        },
    },
    {$project: {_id: 0, sensor: 1, t: 1, avgN: 1, avgAlpha: 1}},
]).toArray();

const expected = [
    {sensor: "a", t: 1, avgN: 10, avgAlpha: 10},
    {sensor: "a", t: 2, avgN: 15, avgAlpha: 12.5},
    {sensor: "a", t: 3, avgN: 22.5, avgAlpha: 16.875},
    {sensor: "a", t: 4, avgN: 31.25, avgAlpha: 22.65625},
    {sensor: "b", t: 1, avgN: 100, avgAlpha: 100},
    {sensor: "b", t: 2, avgN: 75, avgAlpha: 87.5},
];

assert.eq(expected.length, result.length, tojson(result));
for (let i = 0; i < expected.length; ++i) {
    assert.eq(expected[i].sensor, result[i].sensor, tojson(result));
    assert.eq(expected[i].t, result[i].t, tojson(result));
    assert.close(expected[i].avgN, result[i].avgN, tojson(result), 8);
    assert.close(expected[i].avgAlpha, result[i].avgAlpha, tojson(result), 8);
}

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {t: 1},
            output: {bad: {$expMovingAvg: {input: "$value", N: 3, alpha: 0.5}}},
        },
    }],
    cursor: {},
}));
