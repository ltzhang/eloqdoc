const coll = db.expression_round_trunc;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, value: 2.5, place: 0},
    {_id: 2, value: 3.14159, place: 2},
    {_id: 3, value: 335.14159, place: -1},
    {_id: 4, value: NumberDecimal("2.5"), place: 0},
    {_id: 5, value: null, place: 1},
]));

let docs = coll.aggregate([
    {$sort: {_id: 1}},
    {
        $project: {
            _id: 1,
            rounded: {$round: ["$value", "$place"]},
            truncated: {$trunc: ["$value", "$place"]},
            roundedDefault: {$round: "$value"},
            truncatedDefault: {$trunc: "$value"},
        },
    },
]).toArray();

assert.eq(2.0, docs[0].rounded);
assert.eq(2.0, docs[0].truncated);
assert.eq(2.0, docs[0].roundedDefault);
assert.eq(2.0, docs[0].truncatedDefault);

assert.eq(3.14, docs[1].rounded);
assert.eq(3.14, docs[1].truncated);

assert.eq(340.0, docs[2].rounded);
assert.eq(330.0, docs[2].truncated);

assert.eq(NumberDecimal("2"), docs[3].rounded);
assert.eq(NumberDecimal("2"), docs[3].truncated);

assert.eq(null, docs[4].rounded);
assert.eq(null, docs[4].truncated);
assert.eq(null, docs[4].roundedDefault);
assert.eq(null, docs[4].truncatedDefault);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$round: "string"}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$trunc: [1, 101]}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$round: [1, 1.5]}}}],
    cursor: {},
}));
