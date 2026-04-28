const coll = db.agg_fill;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, ts: 1, temp: 10, status: "ok"},
    {_id: 2, ts: 2, temp: null},
    {_id: 3, ts: 3},
    {_id: 4, ts: 4, temp: 13, status: null},
]));

let result = coll.aggregate([
    {$sort: {ts: 1}},
    {
        $fill: {
            sortBy: {ts: 1},
            output: {
                temp: {method: "locf"},
                status: {value: "unknown"},
            },
        }
    },
]).toArray();

assert.eq([
    {_id: 1, ts: 1, temp: 10, status: "ok"},
    {_id: 2, ts: 2, temp: 10, status: "unknown"},
    {_id: 3, ts: 3, temp: 10, status: "unknown"},
    {_id: 4, ts: 4, temp: 13, status: "unknown"},
], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [
        {$fill: {output: {temp: {method: "linear"}}}},
    ],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [
        {$fill: {partitionByFields: ["sensor"], output: {temp: {method: "locf"}}}},
    ],
    cursor: {},
}));
