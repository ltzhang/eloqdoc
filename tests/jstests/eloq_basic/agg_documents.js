const coll = db.agg_documents;
coll.drop();

assert.commandWorked(coll.insert([{_id: "ignored"}]));

let res = db.runCommand({
    aggregate: 1,
    pipeline: [
        {$documents: [{_id: 1, a: 2}, {_id: 2, a: 5}]},
        {$match: {a: {$gt: 2}}},
        {$project: {_id: 0, doubled: {$multiply: ["$a", 2]}}},
    ],
    cursor: {},
});
assert.commandWorked(res);
let docs = res.cursor.firstBatch;
assert.eq([{doubled: 10}], docs);

docs = coll.aggregate([
    {$documents: [{source: "literal"}, {source: "also literal"}]},
    {$sort: {source: 1}},
]).toArray();
assert.eq([{source: "also literal"}, {source: "literal"}], docs);

assert.eq([{_id: "ignored"}], coll.find().toArray());

assert.commandFailed(db.runCommand({
    aggregate: 1,
    pipeline: [
        {$match: {}},
        {$documents: [{a: 1}]},
    ],
    cursor: {},
}));

assert.commandFailedWithCode(db.runCommand({
    aggregate: 1,
    pipeline: [{$documents: {a: 1}}],
    cursor: {},
}), ErrorCodes.TypeMismatch);
