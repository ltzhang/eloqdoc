const coll = db.wildcard_indexes_base;
coll.drop();

assert.commandWorked(coll.createIndex({"$**": 1}, {name: "all_paths"}));
let spec = coll.getIndexes().filter(index => index.name === "all_paths")[0];
assert.neq(null, spec, tojson(coll.getIndexes()));
assert.eq({"$**": 1}, spec.key, tojson(spec));
assert.commandFailed(coll.createIndex({"$**": 1}, {name: "projected", wildcardProjection: {a: 1}}));

assert.commandWorked(coll.insert({_id: 1, a: 5, nested: {b: "x"}, arr: [1, 2]}));
assert.commandWorked(coll.update({_id: 1}, {$set: {"nested.c": 9, z: true}}));
assert.eq(1, coll.find({z: true}).itcount());

assert.commandWorked(coll.remove({_id: 1}));
assert.eq(0, coll.find({a: 5}).itcount());

let validation = coll.validate({full: true});
assert.commandWorked(validation);
