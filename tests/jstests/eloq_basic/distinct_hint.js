const coll = db.distinct_hint;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, category: "a", active: true},
    {_id: 2, category: "b", active: true},
    {_id: 3, category: "a", active: false},
]));
assert.commandWorked(coll.createIndex({active: 1, category: 1}, {name: "active_category"}));

function sorted(values) {
    return values.sort();
}

let res = assert.commandWorked(db.runCommand({
    distinct: coll.getName(),
    key: "category",
    query: {active: true},
    hint: "active_category",
}));
assert.eq(["a", "b"], sorted(res.values), tojson(res));

res = assert.commandWorked(db.runCommand({
    distinct: coll.getName(),
    key: "category",
    query: {active: true},
    hint: {active: 1, category: 1},
}));
assert.eq(["a", "b"], sorted(res.values), tojson(res));

assert.commandFailedWithCode(db.runCommand({
    distinct: coll.getName(),
    key: "category",
    query: {active: true},
    hint: "missing_index",
}), ErrorCodes.BadValue);

assert.commandFailedWithCode(db.runCommand({
    distinct: coll.getName(),
    key: "category",
    query: {active: true},
    hint: 1,
}), ErrorCodes.FailedToParse);
