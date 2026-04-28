const coll = db.accumulator_n_value;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, group: "a", score: 5, label: "first"},
    {_id: 2, group: "a", score: 1, label: "second"},
    {_id: 3, group: "a", score: 9, label: "third"},
    {_id: 4, group: "a", score: 3, label: "fourth"},
    {_id: 5, group: "b", score: 8, label: "only"},
]));

let result = coll.aggregate([
    {$sort: {_id: 1}},
    {
        $group: {
            _id: "$group",
            firstTwo: {$firstN: {input: "$label", n: 2}},
            lastTwo: {$lastN: {input: "$label", n: 2}},
            minThree: {$minN: {input: "$score", n: 3}},
            maxThree: {$maxN: {input: "$score", n: 3}},
        }
    },
    {$sort: {_id: 1}},
]).toArray();

assert.eq([
    {
        _id: "a",
        firstTwo: ["first", "second"],
        lastTwo: ["third", "fourth"],
        minThree: [1, 3, 5],
        maxThree: [9, 5, 3],
    },
    {
        _id: "b",
        firstTwo: ["only"],
        lastTwo: ["only"],
        minThree: [8],
        maxThree: [8],
    },
], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$group: {_id: null, bad: {$firstN: {input: "$score", n: 0}}}}],
    cursor: {},
}));

result = coll.aggregate([
    {$sort: {_id: 1}},
    {
        $group: {
            _id: "$group",
            topScore: {$top: {sortBy: {score: -1}, output: "$label"}},
            bottomScore: {$bottom: {sortBy: {score: -1}, output: "$label"}},
            topTwo: {$topN: {sortBy: {score: -1}, output: "$label", n: 2}},
            bottomTwo: {$bottomN: {sortBy: {score: -1}, output: "$label", n: 2}},
        }
    },
    {$sort: {_id: 1}},
]).toArray();

assert.eq([
    {
        _id: "a",
        topScore: "third",
        bottomScore: "second",
        topTwo: ["third", "first"],
        bottomTwo: ["second", "fourth"],
    },
    {
        _id: "b",
        topScore: "only",
        bottomScore: "only",
        topTwo: ["only"],
        bottomTwo: ["only"],
    },
], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $group: {
            _id: null,
            bad: {$topN: {sortBy: {score: -1}, output: "$label", n: 0}},
        }
    }],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$group: {_id: null, bad: {$minN: {input: "$score", n: "two"}}}}],
    cursor: {},
}));
