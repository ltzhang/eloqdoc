const coll = db.write_hint;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, category: "a", active: true, stale: false, hits: 0},
    {_id: 2, category: "b", active: true, stale: true, hits: 0},
    {_id: 3, category: "c", active: false, stale: true, hits: 0},
]));
assert.commandWorked(coll.createIndex({active: 1, category: 1}, {name: "active_category"}));
assert.commandWorked(coll.createIndex({stale: 1}, {name: "stale_idx"}));

function planHasIndexName(plan, indexName) {
    if (plan == null || typeof plan !== "object") {
        return false;
    }
    if (plan.indexName === indexName) {
        return true;
    }
    for (let key in plan) {
        if (planHasIndexName(plan[key], indexName)) {
            return true;
        }
    }
    return false;
}

let updateExplain = coll.explain("queryPlanner").update(
    {active: true}, {$inc: {hits: 1}}, {multi: true, hint: "active_category"});
assert.eq("UPDATE", updateExplain.queryPlanner.winningPlan.stage, tojson(updateExplain));
assert(planHasIndexName(updateExplain.queryPlanner.winningPlan, "active_category"),
       tojson(updateExplain));

let res = assert.commandWorked(db.runCommand({
    update: coll.getName(),
    updates: [{
        q: {active: true},
        u: {$inc: {hits: 1}},
        multi: true,
        hint: "active_category",
    }],
}));
assert.eq(2, res.n, tojson(res));
assert.eq(1, coll.findOne({_id: 1}).hits);
assert.eq(1, coll.findOne({_id: 2}).hits);
assert.eq(0, coll.findOne({_id: 3}).hits);

let deleteExplain = coll.explain("queryPlanner").remove(
    {stale: true}, {justOne: false, hint: {stale: 1}});
assert.eq("DELETE", deleteExplain.queryPlanner.winningPlan.stage, tojson(deleteExplain));
assert(planHasIndexName(deleteExplain.queryPlanner.winningPlan, "stale_idx"),
       tojson(deleteExplain));

res = assert.commandWorked(db.runCommand({
    delete: coll.getName(),
    deletes: [{
        q: {stale: true},
        limit: 0,
        hint: {stale: 1},
    }],
}));
assert.eq(2, res.n, tojson(res));
assert.eq([{_id: 1, category: "a", active: true, stale: false, hits: 1}],
          coll.find({}, {_id: 1, category: 1, active: 1, stale: 1, hits: 1}).toArray());

assert.commandFailedWithCode(db.runCommand({
    update: coll.getName(),
    updates: [{
        q: {active: true},
        u: {$inc: {hits: 1}},
        multi: true,
        hint: "missing_index",
    }],
}), ErrorCodes.BadValue);

assert.commandFailedWithCode(db.runCommand({
    delete: coll.getName(),
    deletes: [{
        q: {active: true},
        limit: 0,
        hint: 1,
    }],
}), ErrorCodes.FailedToParse);
