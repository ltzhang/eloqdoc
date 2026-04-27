const coll = db.expression_misc;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, str: "abc", bin: BinData(0, "1234"), doc: {a: 1, b: "xy"}, hashInput: 4},
    {_id: 2, str: null, doc: null, hashInput: NumberLong(4)},
    {_id: 3, hashInput: 4.9},
]));

let docs = coll.aggregate([
    {$sort: {_id: 1}},
    {
        $project: {
            _id: 1,
            rand: {$rand: {}},
            strSize: {$binarySize: "$str"},
            binSize: {$binarySize: "$bin"},
            docSize: {$bsonSize: "$doc"},
            hash: {$toHashedIndexKey: "$hashInput"},
            nullHash: {$toHashedIndexKey: "$missing"},
        },
    },
]).toArray();

assert.eq(3, docs.length);
docs.forEach(function(doc) {
    assert.gte(doc.rand, 0);
    assert.lt(doc.rand, 1);
});

assert.eq(3, docs[0].strSize);
assert.eq(3, docs[0].binSize);
assert.eq(Object.bsonsize({a: 1, b: "xy"}), docs[0].docSize);
assert.eq(null, docs[1].strSize);
assert.eq(null, docs[1].docSize);

assert.eq(docs[0].hash, docs[1].hash);
assert.eq(docs[0].hash, docs[2].hash);
assert.eq({$numberLong: docs[0].nullHash.toString()}, {$numberLong: docs[1].nullHash.toString()});

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$binarySize: 42}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$bsonSize: "$str"}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$rand: 1}}}],
    cursor: {},
}));
