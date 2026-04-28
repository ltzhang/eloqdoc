const coll = db.prepare_unique;
coll.drop();

assert.commandWorked(coll.createIndex({email: 1}, {name: "email_1"}));

assert.commandWorked(
    db.runCommand({collMod: coll.getName(), index: {keyPattern: {email: 1}, prepareUnique: true}}));

let spec = coll.getIndexes().filter(index => index.name === "email_1")[0];
assert.neq(null, spec, tojson(coll.getIndexes()));
assert.eq(true, spec.prepareUnique, tojson(spec));
assert.neq(true, spec.unique, tojson(spec));

assert.commandWorked(db.runCommand({collMod: coll.getName(), index: {name: "email_1", prepareUnique: false}}));
spec = coll.getIndexes().filter(index => index.name === "email_1")[0];
assert.neq(true, spec.prepareUnique, tojson(spec));
