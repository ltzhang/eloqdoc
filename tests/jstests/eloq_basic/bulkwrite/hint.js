const coll = db.bulk_write_hint;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, active: true, stale: false, hits: 0},
    {_id: 2, active: true, stale: true, hits: 0},
    {_id: 3, active: false, stale: true, hits: 0},
]));
assert.commandWorked(coll.createIndex({active: 1, _id: 1}, {name: "active_id"}));
assert.commandWorked(coll.createIndex({stale: 1}, {name: "stale_idx"}));

let res = assert.commandWorked(db.adminCommand({
    bulkWrite: 1,
    ops: [
        {
            update: 0,
            filter: {_id: 1, active: true},
            updateMods: {$inc: {hits: 1}},
            multi: false,
            hint: "active_id",
        },
        {
            delete: 0,
            filter: {_id: 2, stale: true},
            multi: false,
            hint: {stale: 1},
        },
    ],
    nsInfo: [{ns: coll.getFullName()}],
    ordered: true,
    cursor: {},
}));

assert.eq(1, res.nMatched, tojson(res));
assert.eq(1, res.nModified, tojson(res));
assert.eq(1, res.nDeleted, tojson(res));
assert.eq(0, res.nErrors, tojson(res));
assert.eq({_id: 1, active: true, stale: false, hits: 1}, coll.findOne({_id: 1}));

res = assert.commandWorked(db.adminCommand({
    bulkWrite: 1,
    ops: [
        {
            update: 0,
            filter: {active: true},
            updateMods: {$inc: {hits: 1}},
            multi: true,
            hint: "missing_index",
        },
    ],
    nsInfo: [{ns: coll.getFullName()}],
    ordered: true,
    cursor: {},
}));

assert.eq(0, res.nMatched, tojson(res));
assert.eq(1, res.nErrors, tojson(res));
assert.eq(0, res.cursor.firstBatch[0].idx, tojson(res));
assert.eq(ErrorCodes.BadValue, res.cursor.firstBatch[0].code, tojson(res));
