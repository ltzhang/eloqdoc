const coll = db.let_in_commands;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, qty: 5, tag: "low"},
    {_id: 2, qty: 10, tag: "mid"},
    {_id: 3, qty: 15, tag: "high"},
]));

let res = assert.commandWorked(db.runCommand({
    find: coll.getName(),
    filter: {$expr: {$gt: ["$qty", "$$threshold"]}},
    let: {threshold: 9},
}));
assert.eq([2, 3], res.cursor.firstBatch.map(doc => doc._id), tojson(res));

res = assert.commandWorked(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$match: {$expr: {$eq: ["$tag", "$$wanted"]}}}],
    cursor: {},
    let: {wanted: "mid"},
    runtimeConstants: {localNow: new Date()},
}));
assert.eq([2], res.cursor.firstBatch.map(doc => doc._id), tojson(res));

res = assert.commandWorked(db.runCommand({
    update: coll.getName(),
    updates: [{
        q: {$expr: {$gte: ["$qty", "$$lim"]}},
        u: {$set: {selected: true}},
        multi: true,
    }],
    let: {lim: 10},
    runtimeConstants: {},
}));
assert.eq(2, res.n, tojson(res));
assert.eq([2, 3], coll.find({selected: true}).sort({_id: 1}).map(doc => doc._id));

res = assert.commandWorked(db.runCommand({
    findAndModify: coll.getName(),
    query: {$expr: {$eq: ["$tag", "$$wanted"]}},
    update: {$set: {fam: true}},
    new: true,
    let: {wanted: "low"},
    runtimeConstants: {},
}));
assert.eq(1, res.value._id, tojson(res));
assert.eq(true, res.value.fam, tojson(res));

res = assert.commandWorked(db.runCommand({
    delete: coll.getName(),
    deletes: [{
        q: {$expr: {$lt: ["$qty", "$$max"]}},
        limit: 0,
    }],
    let: {max: 11},
    runtimeConstants: {},
}));
assert.eq(2, res.n, tojson(res));
assert.eq([3], coll.find().map(doc => doc._id));

assert.commandFailedWithCode(db.runCommand({
    find: coll.getName(),
    filter: {$expr: {$eq: ["$qty", "$$missing"]}},
}), 17276);
