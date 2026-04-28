const coll = db.agg_fill_date_linear;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, sensor: "a", ts: 1, observedAt: new Date(1000)},
    {_id: 2, sensor: "a", ts: 2, observedAt: null},
    {_id: 3, sensor: "a", ts: 3, observedAt: new Date(7000)},
    {_id: 4, sensor: "b", ts: 1, observedAt: null},
    {_id: 5, sensor: "b", ts: 2, observedAt: new Date(9000)},
    {_id: 6, sensor: "c", ts: 1, observedAt: new Date(1000), mixed: new Date(1000)},
    {_id: 7, sensor: "c", ts: 2, observedAt: null, mixed: null},
    {_id: 8, sensor: "c", ts: 3, observedAt: new Date(3000), mixed: 3000},
]));

const result = coll.aggregate([
    {$sort: {sensor: 1, ts: 1}},
    {
        $fill: {
            partitionByFields: ["sensor"],
            sortBy: {ts: 1},
            output: {
                observedAt: {method: "linear"},
            },
        },
    },
    {$project: {_id: 1, sensor: 1, ts: 1, observedAt: 1}},
]).toArray();

assert.eq([
    {_id: 1, sensor: "a", ts: 1, observedAt: new Date(1000)},
    {_id: 2, sensor: "a", ts: 2, observedAt: new Date(4000)},
    {_id: 3, sensor: "a", ts: 3, observedAt: new Date(7000)},
    {_id: 4, sensor: "b", ts: 1, observedAt: null},
    {_id: 5, sensor: "b", ts: 2, observedAt: new Date(9000)},
    {_id: 6, sensor: "c", ts: 1, observedAt: new Date(1000)},
    {_id: 7, sensor: "c", ts: 2, observedAt: new Date(2000)},
    {_id: 8, sensor: "c", ts: 3, observedAt: new Date(3000)},
], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [
        {$sort: {sensor: 1, ts: 1}},
        {
            $fill: {
                partitionByFields: ["sensor"],
                sortBy: {ts: 1},
                output: {
                    mixed: {method: "linear"},
                },
            },
        },
    ],
    cursor: {},
}));
