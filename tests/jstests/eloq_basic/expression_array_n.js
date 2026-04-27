const coll = db.expression_array_n;
coll.drop();

assert.commandWorked(coll.insert([
    {
        _id: 1,
        nums: [5, 1, 3, 2, 4],
        words: ["delta", "alpha", "charlie", "bravo"],
        docs: [{score: 2, name: "b"}, {score: 3, name: "a"}, {score: 1, name: "c"}],
    },
    {_id: 2, nums: [2, "a", null, 1], docs: [{score: 2}, {}, {score: 1}]},
    {_id: 3, nums: null, docs: "not an array"},
]));

let docs = coll.aggregate([
    {$sort: {_id: 1}},
    {
        $project: {
            _id: 1,
            sortedNums: {$sortArray: {input: "$nums", sortBy: 1}},
            sortedNumsDesc: {$sortArray: {input: "$nums", sortBy: -1}},
            sortedDocs: {$sortArray: {input: "$docs", sortBy: {score: -1, name: 1}}},
            firstTwo: {$firstN: {input: "$nums", n: 2}},
            lastTwo: {$lastN: {input: "$nums", n: 2}},
            minThree: {$minN: {input: "$nums", n: 3}},
            maxThree: {$maxN: {input: "$nums", n: 3}},
            tooMany: {$firstN: {input: "$words", n: 10}},
        },
    },
]).toArray();

assert.eq([1, 2, 3, 4, 5], docs[0].sortedNums);
assert.eq([5, 4, 3, 2, 1], docs[0].sortedNumsDesc);
assert.eq([{score: 3, name: "a"}, {score: 2, name: "b"}, {score: 1, name: "c"}],
          docs[0].sortedDocs);
assert.eq([5, 1], docs[0].firstTwo);
assert.eq([2, 4], docs[0].lastTwo);
assert.eq([1, 2, 3], docs[0].minThree);
assert.eq([5, 4, 3], docs[0].maxThree);
assert.eq(["delta", "alpha", "charlie", "bravo"], docs[0].tooMany);

assert.eq([null, 1, 2, "a"], docs[1].sortedNums);
assert.eq([{score: 2}, {score: 1}, {}], docs[1].sortedDocs);
assert.eq([null, 1, 2], docs[1].minThree);
assert.eq(["a", 2, 1], docs[1].maxThree);

assert.eq(null, docs[2].sortedNums);
assert.eq(null, docs[2].sortedDocs);
assert.eq(null, docs[2].firstTwo);
assert.eq(null, docs[2].lastTwo);
assert.eq(null, docs[2].minThree);
assert.eq(null, docs[2].maxThree);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$firstN: {input: "$nums", n: 0}}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$sortArray: {input: "$nums", sortBy: {score: 2}}}}}],
    cursor: {},
}));
