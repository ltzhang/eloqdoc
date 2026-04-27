const coll = db.accumulator_count;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, category: "a", qty: 2},
    {_id: 2, category: "a", qty: 3},
    {_id: 3, category: "b", qty: 5},
]));

const grouped = coll.aggregate([
    {$group: {_id: "$category", n: {$count: {}}, total: {$sum: "$qty"}}},
    {$sort: {_id: 1}},
]).toArray();

assert.eq([{_id: "a", n: NumberLong(2), total: 5}, {_id: "b", n: NumberLong(1), total: 5}],
          grouped);

assert.commandFailedWithCode(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$group: {_id: null, n: {$count: 1}}}],
    cursor: {},
}), ErrorCodes.FailedToParse);

assert.commandFailedWithCode(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$group: {_id: null, n: {$count: {unexpected: true}}}}],
    cursor: {},
}), ErrorCodes.FailedToParse);
