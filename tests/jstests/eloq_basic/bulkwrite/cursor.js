const coll = db.bulk_write_cursor;
coll.drop();

const ops = [];
for (let i = 0; i < 5; ++i) {
    ops.push({insert: 0, document: {_id: i}});
}

let res = assert.commandWorked(db.adminCommand({
    bulkWrite: 1,
    ops: ops,
    nsInfo: [{ns: coll.getFullName()}],
    ordered: true,
    cursor: {batchSize: 2},
}));

assert.eq(5, res.nInserted, tojson(res));
assert.eq(0, res.nErrors, tojson(res));
assert.neq(0, res.cursor.id, tojson(res));
assert.eq([0, 1], res.cursor.firstBatch.map(entry => entry.idx), tojson(res));

res = assert.commandWorked(db.getSiblingDB("admin").runCommand({
    getMore: res.cursor.id,
    collection: "$cmd.bulkWrite",
    batchSize: 2,
}));
assert.neq(0, res.cursor.id, tojson(res));
assert.eq([2, 3], res.cursor.nextBatch.map(entry => entry.idx), tojson(res));

res = assert.commandWorked(db.getSiblingDB("admin").runCommand({
    getMore: res.cursor.id,
    collection: "$cmd.bulkWrite",
    batchSize: 2,
}));
assert.eq(0, res.cursor.id, tojson(res));
assert.eq([4], res.cursor.nextBatch.map(entry => entry.idx), tojson(res));

assert.eq(5, coll.count(), tojson(coll.find().toArray()));
