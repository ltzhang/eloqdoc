const coll = db.expression_bitwise;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 0, a: NumberInt(0), b: NumberInt(127), c: [NumberInt(0), NumberInt(127)]},
    {_id: 1, a: NumberInt(1), b: NumberInt(2)},
    {_id: 2, a: NumberInt(2), b: NumberInt(3)},
    {_id: 3, a: NumberInt(3), b: NumberInt(5)},
    {_id: 4, a: null, b: NumberInt(1)},
    {_id: 5, a: NumberLong("9223372036854775807"), b: NumberLong("1")},
]));

let docs = coll.aggregate([
    {$sort: {_id: 1}},
    {
        $project: {
            _id: 1,
            and: {$bitAnd: ["$a", "$b"]},
            or: {$bitOr: ["$a", "$b"]},
            xor: {$bitXor: ["$a", "$b"]},
            not: {$bitNot: "$a"},
        },
    },
]).toArray();

assert.eq(0, docs[0].and);
assert.eq(127, docs[0].or);
assert.eq(127, docs[0].xor);
assert.eq(-1, docs[0].not);

assert.eq(0, docs[1].and);
assert.eq(3, docs[1].or);
assert.eq(3, docs[1].xor);
assert.eq(-2, docs[1].not);

assert.eq(2, docs[2].and);
assert.eq(3, docs[2].or);
assert.eq(1, docs[2].xor);
assert.eq(-3, docs[2].not);

assert.eq(1, docs[3].and);
assert.eq(7, docs[3].or);
assert.eq(6, docs[3].xor);
assert.eq(-4, docs[3].not);

assert.eq(null, docs[4].and);
assert.eq(null, docs[4].or);
assert.eq(null, docs[4].xor);
assert.eq(null, docs[4].not);

assert.eq(NumberLong(1), docs[5].and);
assert.eq(NumberLong("9223372036854775807"), docs[5].or);
assert.eq(NumberLong("9223372036854775806"), docs[5].xor);
assert.eq(NumberLong("-9223372036854775808"), docs[5].not);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$bitAnd: ["$a", 12.5]}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$bitOr: ["$a", "$c"]}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$bitXor: ["$a", NumberDecimal("12")]}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$bitNot: ["$a", "$b"]}}}],
    cursor: {},
}));
