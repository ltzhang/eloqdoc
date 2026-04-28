const coll = db.find_allow_disk_use;
coll.drop();

assert.commandWorked(coll.insert([{_id: 1, x: 2}, {_id: 2, x: 1}]));

let res = assert.commandWorked(db.runCommand({
    find: coll.getName(),
    filter: {},
    sort: {x: 1},
    allowDiskUse: true,
}));
assert.eq([{_id: 2, x: 1}, {_id: 1, x: 2}], res.cursor.firstBatch, tojson(res));

assert.commandFailedWithCode(db.runCommand({
    find: coll.getName(),
    filter: {},
    sort: {x: 1},
    allowDiskUse: "true",
}), ErrorCodes.FailedToParse);

assert.eq([{_id: 2, x: 1}, {_id: 1, x: 2}], coll.find().sort({x: 1}).allowDiskUse().toArray());
