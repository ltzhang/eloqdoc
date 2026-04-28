const coll = db.accumulator_percentile;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, group: "a", value: 1},
    {_id: 2, group: "a", value: 2},
    {_id: 3, group: "a", value: 3},
    {_id: 4, group: "a", value: 4},
    {_id: 5, group: "a", value: "ignored"},
    {_id: 6, group: "b", value: null},
]));

let result = coll.aggregate([
    {
        $group: {
            _id: "$group",
            p50p75: {$percentile: {input: "$value", p: [0.5, 0.75], method: "approximate"}},
            median: {$median: {input: "$value", method: "approximate"}},
        }
    },
    {$sort: {_id: 1}},
]).toArray();

assert.eq([
    {_id: "a", p50p75: [2.5, 3.25], median: 2.5},
    {_id: "b", p50p75: [null, null], median: null},
], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [
        {
            $group: {
                _id: null,
                bad: {$percentile: {input: "$value", p: [1.2], method: "approximate"}},
            }
        },
    ],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [
        {$group: {_id: null, bad: {$median: {input: "$value", method: "exact"}}}},
    ],
    cursor: {},
}));
