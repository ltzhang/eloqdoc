const coll = db.wildcard_indexes_base;
const includeColl = db.wildcard_indexes_include_projection;
const excludeColl = db.wildcard_indexes_exclude_projection;
coll.drop();
includeColl.drop();
excludeColl.drop();

assert.commandWorked(coll.createIndex({"$**": 1}, {name: "all_paths"}));
let spec = coll.getIndexes().filter(index => index.name === "all_paths")[0];
assert.neq(null, spec, tojson(coll.getIndexes()));
assert.eq({"$**": 1}, spec.key, tojson(spec));
assert.commandFailed(
    coll.createIndex({"$**": 1}, {name: "mixed_projection", wildcardProjection: {a: 1, b: 0}}));

assert.commandWorked(coll.insert({_id: 1, a: 5, nested: {b: "x"}, arr: [1, 2]}));
assert.commandWorked(coll.update({_id: 1}, {$set: {"nested.c": 9, z: true}}));
assert.eq(1, coll.find({z: true}).itcount());

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
