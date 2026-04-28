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

let sortLimit = assert.commandWorked(
    db.adminCommand({getParameter: 1, internalQueryExecMaxBlockingSortBytes: 1}));
const oldSortLimit = sortLimit.internalQueryExecMaxBlockingSortBytes;

try {
    assert.commandWorked(
        db.adminCommand({setParameter: 1, internalQueryExecMaxBlockingSortBytes: 1024 * 1024}));

    coll.drop();
    let large = "";
    for (let i = 0; i < 32 * 1024; ++i) {
        large += "x";
    }
    let bulk = coll.initializeUnorderedBulkOp();
    for (let i = 0; i < 100; ++i) {
        bulk.insert({_id: i, x: 100 - i, payload: large});
    }
    assert.commandWorked(bulk.execute());

    assert.throws(function() {
        coll.find({}).sort({payload: 1, x: 1}).itcount();
    });

    const sorted = coll.find({}, {_id: 1, x: 1}).sort({payload: 1, x: 1}).allowDiskUse().toArray();
    assert.eq(100, sorted.length, tojson(sorted));
    assert.eq({_id: 99, x: 1}, sorted[0], tojson(sorted));
    assert.eq({_id: 0, x: 100}, sorted[99], tojson(sorted));

    const cursor = coll.find({}, {_id: 1, x: 1})
                       .sort({payload: 1, x: 1})
                       .allowDiskUse()
                       .batchSize(11);
    for (let expectedX = 1; expectedX <= 100; ++expectedX) {
        assert(cursor.hasNext(), "expected x " + expectedX);
        const doc = cursor.next();
        assert.eq(100 - expectedX, doc._id, tojson(doc));
        assert.eq(expectedX, doc.x, tojson(doc));
    }
    assert(!cursor.hasNext());
} finally {
    assert.commandWorked(db.adminCommand(
        {setParameter: 1, internalQueryExecMaxBlockingSortBytes: oldSortLimit}));
}
