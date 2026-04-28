const coll = db.agg_densify_date;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, sensor: "a", ts: new Date(0), reading: 10},
    {_id: 2, sensor: "a", ts: new Date(120000), reading: 12},
    {_id: 3, sensor: "b", ts: new Date(60000), reading: 20},
]));

let result = coll.aggregate([
    {$sort: {sensor: 1, ts: 1}},
    {
        $densify: {
            field: "ts",
            partitionByFields: ["sensor"],
            range: {step: 1, unit: "minute", bounds: [new Date(0), new Date(180000)]},
        },
    },
    {$project: {_id: 0, sensor: 1, ts: 1, reading: 1}},
]).toArray();

assert.eq([
    {sensor: "a", ts: new Date(0), reading: 10},
    {sensor: "a", ts: new Date(60000)},
    {sensor: "a", ts: new Date(120000), reading: 12},
    {sensor: "a", ts: new Date(180000)},
    {sensor: "b", ts: new Date(0)},
    {sensor: "b", ts: new Date(60000), reading: 20},
    {sensor: "b", ts: new Date(120000)},
    {sensor: "b", ts: new Date(180000)},
], result);

result = coll.aggregate([
    {$sort: {sensor: 1, ts: 1}},
    {
        $densify: {
            field: "ts",
            partitionByFields: ["sensor"],
            range: {step: 1, unit: "minute", bounds: "partition"},
        },
    },
    {$project: {_id: 0, sensor: 1, ts: 1, reading: 1}},
]).toArray();

assert.eq([
    {sensor: "a", ts: new Date(0), reading: 10},
    {sensor: "a", ts: new Date(60000)},
    {sensor: "a", ts: new Date(120000), reading: 12},
    {sensor: "b", ts: new Date(60000), reading: 20},
], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [
        {$densify: {field: "ts", range: {step: 1, bounds: [new Date(0), new Date(60000)]}}},
    ],
    cursor: {},
}));
