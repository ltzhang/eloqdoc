const collA = db.bulk_write_base_a;
const collB = db.bulk_write_base_b;
collA.drop();
collB.drop();

let res = assert.commandWorked(db.adminCommand({
    bulkWrite: 1,
    ops: [
        {insert: 0, document: {_id: 1, qty: 5}},
        {insert: 1, document: {_id: 10, old: true}},
        {update: 0, filter: {_id: 1}, updateMods: {$set: {qty: 7}}},
        {delete: 1, filter: {old: true}, multi: false},
    ],
    nsInfo: [
        {ns: collA.getFullName()},
        {ns: collB.getFullName()},
    ],
    ordered: true,
    cursor: {},
}));

assert.eq(2, res.nInserted, tojson(res));
assert.eq(1, res.nMatched, tojson(res));
assert.eq(1, res.nModified, tojson(res));
assert.eq(0, res.nUpserted, tojson(res));
assert.eq(1, res.nDeleted, tojson(res));
assert.eq(0, res.nErrors, tojson(res));
assert.eq(0, res.cursor.id, tojson(res));
assert.eq([0, 1, 2, 3], res.cursor.firstBatch.map(entry => entry.idx), tojson(res));

assert.eq({_id: 1, qty: 7}, collA.findOne({_id: 1}));
assert.eq(0, collB.count(), tojson(collB.find().toArray()));

res = assert.commandWorked(db.adminCommand({
    bulkWrite: 1,
    ops: [
        {insert: 0, document: {_id: 2}},
        {insert: 0, document: {_id: 2}},
        {insert: 0, document: {_id: 3}},
    ],
    nsInfo: [{ns: collA.getFullName()}],
    ordered: true,
    cursor: {},
}));

assert.eq(1, res.nInserted, tojson(res));
assert.eq(1, res.nErrors, tojson(res));
assert.eq([0, 1], res.cursor.firstBatch.map(entry => entry.idx), tojson(res));
assert.eq(11000, res.cursor.firstBatch[1].code, tojson(res));
assert.eq(null, collA.findOne({_id: 3}));

res = assert.commandWorked(db.adminCommand({
    bulkWrite: 1,
    ops: [
        {insert: 0, document: {_id: 4}},
        {insert: 0, document: {_id: 4}},
        {insert: 0, document: {_id: 5}},
    ],
    nsInfo: [{ns: collA.getFullName()}],
    ordered: false,
    cursor: {},
}));

assert.eq(2, res.nInserted, tojson(res));
assert.eq(1, res.nErrors, tojson(res));
assert.eq([0, 1, 2], res.cursor.firstBatch.map(entry => entry.idx), tojson(res));
assert.eq(11000, res.cursor.firstBatch[1].code, tojson(res));
assert.neq(null, collA.findOne({_id: 5}));

res = assert.commandWorked(db.adminCommand({
    bulkWrite: 1,
    ops: [
        {update: 0, filter: {$expr: {$gte: ["$qty", "$$lim"]}}, updateMods: [{$set: {bucket: "$$bucket"}}]},
        {delete: 0, filter: {$expr: {$eq: ["$bucket", "$$bucket"]}}, multi: false},
    ],
    nsInfo: [{ns: collA.getFullName()}],
    let: {lim: 7, bucket: "matched"},
    ordered: true,
    cursor: {},
}));

assert.eq(1, res.nMatched, tojson(res));
assert.eq(1, res.nModified, tojson(res));
assert.eq(1, res.nDeleted, tojson(res));
assert.eq(null, collA.findOne({_id: 1}));

res = assert.commandWorked(db.adminCommand({
    bulkWrite: 1,
    ops: [
        {insert: 0, document: {_id: 6}},
        {insert: 0, document: {_id: 6}},
        {insert: 0, document: {_id: 7}},
    ],
    nsInfo: [{ns: collA.getFullName()}],
    ordered: false,
    errorsOnly: true,
    cursor: {},
}));

assert.eq(2, res.nInserted, tojson(res));
assert.eq(1, res.nErrors, tojson(res));
assert.eq([1], res.cursor.firstBatch.map(entry => entry.idx), tojson(res));
assert.eq(11000, res.cursor.firstBatch[0].code, tojson(res));
