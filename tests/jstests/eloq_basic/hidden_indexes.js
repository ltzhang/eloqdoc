const coll = db.hidden_indexes;
coll.drop();

function planHasStage(plan, stage) {
    if (!plan || typeof plan !== "object") {
        return false;
    }
    if (plan.stage === stage) {
        return true;
    }
    for (let key in plan) {
        if (planHasStage(plan[key], stage)) {
            return true;
        }
    }
    return false;
}

function winningPlanFor(query) {
    return coll.find(query).explain("queryPlanner").queryPlanner.winningPlan;
}

function indexSpec(name) {
    return coll.getIndexes().filter(spec => spec.name === name)[0];
}

assert.commandWorked(coll.insert([
    {_id: 1, a: 1},
    {_id: 2, a: 2},
    {_id: 3, a: 3},
]));

assert.commandWorked(coll.createIndex({a: 1}, {name: "a_1_hidden", hidden: true}));
assert.eq(true, indexSpec("a_1_hidden").hidden, tojson(coll.getIndexes()));
assert(!planHasStage(winningPlanFor({a: 2}), "IXSCAN"), tojson(winningPlanFor({a: 2})));
assert.commandWorked(coll.insert({_id: 4, a: 2}));

assert.commandWorked(coll.unhideIndex("a_1_hidden"));
assert.eq(false, indexSpec("a_1_hidden").hidden, tojson(coll.getIndexes()));
assert(planHasStage(winningPlanFor({a: 2}), "IXSCAN"), tojson(winningPlanFor({a: 2})));
assert.eq([2, 4], coll.find({a: 2}).sort({_id: 1}).map(doc => doc._id));

assert.commandWorked(coll.hideIndex({a: 1}));
assert.eq(true, indexSpec("a_1_hidden").hidden, tojson(coll.getIndexes()));
assert(!planHasStage(winningPlanFor({a: 2}), "IXSCAN"), tojson(winningPlanFor({a: 2})));
