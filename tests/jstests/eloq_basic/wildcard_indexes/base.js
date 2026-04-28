const coll = db.wildcard_indexes_base;
const includeColl = db.wildcard_indexes_include_projection;
const excludeColl = db.wildcard_indexes_exclude_projection;
const subtreeColl = db.wildcard_indexes_subtree;
coll.drop();
includeColl.drop();
excludeColl.drop();
subtreeColl.drop();

function planHasIndexName(plan, indexName) {
    if (!plan || typeof plan !== "object") {
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

function winningPlan(collection, query) {
    return collection.find(query).explain("queryPlanner").queryPlanner.winningPlan;
}

assert.commandWorked(coll.createIndex({"$**": 1}, {name: "all_paths"}));
let spec = coll.getIndexes().filter(index => index.name === "all_paths")[0];
assert.neq(null, spec, tojson(coll.getIndexes()));
assert.eq({"$**": 1}, spec.key, tojson(spec));
assert.commandFailed(
    coll.createIndex({"$**": 1}, {name: "mixed_projection", wildcardProjection: {a: 1, b: 0}}));
assert.commandFailed(
    coll.createIndex({"$**": 1}, {name: "bad_projection", wildcardProjection: "a"}));

assert.commandWorked(coll.insert({_id: 1, a: 5, nested: {b: "x"}, arr: [1, 2]}));
assert.commandWorked(coll.insert({_id: 2, a: 8, nested: {b: "y"}}));
assert.commandWorked(coll.update({_id: 1}, {$set: {"nested.c": 9, z: true}}));
assert.eq(1, coll.find({z: true}).itcount());
assert.eq([1], coll.find({"nested.b": "x"}, {_id: 1}).sort({_id: 1}).toArray().map(doc => doc._id));
assert(planHasIndexName(winningPlan(coll, {"nested.b": "x"}), "all_paths"),
       tojson(winningPlan(coll, {"nested.b": "x"})));
assert.eq([2], coll.find({a: {$gt: 6}}, {_id: 1}).sort({_id: 1}).toArray().map(doc => doc._id));
assert(planHasIndexName(winningPlan(coll, {a: {$gt: 6}}), "all_paths"),
       tojson(winningPlan(coll, {a: {$gt: 6}})));
assert.eq([1], coll.find({z: {$exists: true}}, {_id: 1}).sort({_id: 1}).toArray().map(doc => doc._id));
assert(planHasIndexName(winningPlan(coll, {z: {$exists: true}}), "all_paths"),
       tojson(winningPlan(coll, {z: {$exists: true}})));

assert.commandWorked(coll.remove({_id: 1}));
assert.eq(0, coll.find({a: 5}).itcount());

let validation = coll.validate({full: true});
assert.commandWorked(validation);

assert.commandWorked(
    includeColl.createIndex({"$**": 1}, {name: "projected", wildcardProjection: {a: 1}}));
spec = includeColl.getIndexes().filter(index => index.name === "projected")[0];
assert.neq(null, spec, tojson(includeColl.getIndexes()));
assert.eq({a: 1}, spec.wildcardProjection, tojson(spec));
assert.commandWorked(includeColl.insert({_id: 1, a: {b: 1}, skipped: 2}));
assert.commandWorked(includeColl.update({_id: 1}, {$set: {"a.c": 3, skippedAgain: 4}}));
validation = includeColl.validate({full: true});
assert.commandWorked(validation);

assert.commandWorked(
    excludeColl.createIndex({"$**": 1}, {name: "excluded", wildcardProjection: {secret: 0}}));
spec = excludeColl.getIndexes().filter(index => index.name === "excluded")[0];
assert.neq(null, spec, tojson(excludeColl.getIndexes()));
assert.eq({secret: 0}, spec.wildcardProjection, tojson(spec));
assert.commandWorked(excludeColl.insert({_id: 1, public: 1, secret: {token: "hidden"}}));
assert.commandWorked(excludeColl.update({_id: 1}, {$set: {public2: 2, "secret.rotated": true}}));
validation = excludeColl.validate({full: true});
assert.commandWorked(validation);

assert.commandWorked(subtreeColl.createIndex({"products.$**": 1}, {name: "products_wildcard"}));
spec = subtreeColl.getIndexes().filter(index => index.name === "products_wildcard")[0];
assert.neq(null, spec, tojson(subtreeColl.getIndexes()));
assert.eq({"products.$**": 1}, spec.key, tojson(spec));
assert.commandWorked(subtreeColl.insert({
    _id: 1,
    products: [{sku: "a", qty: 2}, {sku: "b", qty: 3}],
    ignored: {sku: "outside"},
}));
assert.commandWorked(subtreeColl.update({_id: 1}, {$set: {"products.0.qty": 4, outside: 1}}));
validation = subtreeColl.validate({full: true});
assert.commandWorked(validation);
