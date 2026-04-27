const coll = db.stable_api_permissive;
coll.drop();

assert.commandWorked(coll.insert({_id: 1, x: 1}));

let res = assert.commandWorked(db.runCommand({find: coll.getName(), apiVersion: "1"}));
assert.eq(1, res.cursor.firstBatch.length, tojson(res));

res = assert.commandWorked(db.runCommand({
    find: coll.getName(),
    apiVersion: "1",
    apiStrict: true,
    apiDeprecationErrors: true,
}));
assert.eq(1, res.cursor.firstBatch.length, tojson(res));

assert.commandFailedWithCode(db.runCommand({find: coll.getName(), apiVersion: "2"}), 322);
assert.commandFailedWithCode(db.runCommand({find: coll.getName(), apiVersion: 1}), 322);
