const coll = db.prepare_unique;
coll.drop();

assert.commandWorked(coll.createIndex({email: 1}, {name: "email_1"}));

assert.commandWorked(coll.insert([
    {_id: 1, email: "dup@example.com", note: "existing duplicate 1"},
    {_id: 2, email: "dup@example.com", note: "existing duplicate 2"},
    {_id: 3, email: "unique@example.com"},
]));

assert.commandWorked(
    db.runCommand({collMod: coll.getName(), index: {keyPattern: {email: 1}, prepareUnique: true}}));

let spec = coll.getIndexes().filter(index => index.name === "email_1")[0];
assert.neq(null, spec, tojson(coll.getIndexes()));
assert.eq(true, spec.prepareUnique, tojson(spec));
assert.neq(true, spec.unique, tojson(spec));

assert.commandFailedWithCode(coll.insert({_id: 4, email: "dup@example.com"}),
                             ErrorCodes.DuplicateKey);
assert.commandWorked(coll.insert({_id: 5, email: "new@example.com"}));
assert.commandFailedWithCode(coll.update({_id: 5}, {$set: {email: "dup@example.com"}}),
                             ErrorCodes.DuplicateKey);
assert.eq(1, coll.find({email: "new@example.com"}).itcount());
assert.eq(2, coll.find({email: "dup@example.com"}).itcount());

assert.commandWorked(db.runCommand({collMod: coll.getName(), index: {name: "email_1", prepareUnique: false}}));
spec = coll.getIndexes().filter(index => index.name === "email_1")[0];
assert.neq(true, spec.prepareUnique, tojson(spec));
