const coll = db.stage_aliases_50;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, a: 1, b: 2, nested: {x: "one"}},
    {_id: 2, a: 3, b: 4, nested: {x: "two"}},
]));

let docs = coll.aggregate([
    {$set: {sum: {$add: ["$a", "$b"]}}},
    {$project: {_id: 0, sum: 1}},
    {$sort: {sum: 1}},
]).toArray();
assert.eq([{sum: 3}, {sum: 7}], docs);

docs = coll.aggregate([
    {$unset: "b"},
    {$project: {_id: 0, a: 1, b: 1}},
    {$sort: {a: 1}},
]).toArray();
assert.eq([{a: 1}, {a: 3}], docs);

docs = coll.aggregate([
    {$unset: ["a", "b"]},
    {$project: {_id: 0, a: 1, b: 1, nested: 1}},
    {$sort: {"nested.x": 1}},
]).toArray();
assert.eq([{nested: {x: "one"}}, {nested: {x: "two"}}], docs);

docs = coll.aggregate([
    {$replaceWith: "$nested"},
    {$sort: {x: 1}},
]).toArray();
assert.eq([{x: "one"}, {x: "two"}], docs);

assert.commandFailedWithCode(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$unset: {a: 1}}],
    cursor: {},
}), ErrorCodes.TypeMismatch);
