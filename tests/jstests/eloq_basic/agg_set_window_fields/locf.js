const coll = db.agg_set_window_fields_locf;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, sensor: "a", t: 1, value: null},
    {_id: 2, sensor: "a", t: 2, value: 10},
    {_id: 3, sensor: "a", t: 3},
    {_id: 4, sensor: "a", t: 4, value: 20},
    {_id: 5, sensor: "b", t: 1},
    {_id: 6, sensor: "b", t: 2, value: 5},
]));

const result = coll.aggregate([
    {
        $setWindowFields: {
            partitionBy: "$sensor",
            sortBy: {t: 1},
            output: {
                filled: {$locf: "$value"},
            },
        },
    },
    {$project: {_id: 0, sensor: 1, t: 1, filled: 1}},
]).toArray();

assert.eq([
    {sensor: "a", t: 1, filled: null},
    {sensor: "a", t: 2, filled: 10},
    {sensor: "a", t: 3, filled: 10},
    {sensor: "a", t: 4, filled: 20},
    {sensor: "b", t: 1, filled: null},
    {sensor: "b", t: 2, filled: 5},
], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {t: 1},
            output: {bad: {$locf: "$value", window: {documents: ["unbounded", "current"]}}},
        },
    }],
    cursor: {},
}));
