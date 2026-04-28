const coll = db.pipeline_update;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, qty: 5, bonus: 2, tag: "low"},
    {_id: 2, qty: 10, bonus: 4, tag: "mid"},
]));

let res = assert.commandWorked(db.runCommand({
    update: coll.getName(),
    updates: [{
        q: {_id: 1},
        u: [
            {$set: {total: {$add: ["$qty", "$bonus"]}}},
            {$unset: "bonus"},
        ],
        multi: false,
    }],
}));
assert.eq(1, res.n, tojson(res));
assert.eq({_id: 1, qty: 5, tag: "low", total: 7}, coll.findOne({_id: 1}));

res = assert.commandWorked(db.runCommand({
    update: coll.getName(),
    updates: [{
        q: {$expr: {$gte: ["$qty", "$$lim"]}},
        u: [{$set: {selected: "$$mark"}}],
        multi: true,
    }],
    let: {lim: 10, mark: true},
}));
assert.eq(1, res.n, tojson(res));
assert.eq(true, coll.findOne({_id: 2}).selected);

res = assert.commandWorked(db.runCommand({
    findAndModify: coll.getName(),
    query: {_id: 2},
    update: [{$set: {famTotal: {$add: ["$qty", "$bonus"]}}}],
    new: true,
}));
assert.eq(14, res.value.famTotal, tojson(res));

assert.commandFailed(db.runCommand({
    update: coll.getName(),
    updates: [{
        q: {_id: 1},
        u: [{$group: {_id: "$tag"}}],
        multi: false,
    }],
}));

assert.commandFailed(db.runCommand({
    update: coll.getName(),
    updates: [{
        q: {_id: 1},
        u: [{$set: {_id: 99}}],
        multi: false,
    }],
}));
assert.eq(1, coll.findOne({_id: 1})._id);
