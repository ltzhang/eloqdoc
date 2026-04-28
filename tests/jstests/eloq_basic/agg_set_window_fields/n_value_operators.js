const coll = db.agg_set_window_fields_n_value_operators;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, grp: "a", t: 1, score: 5, label: "a1"},
    {_id: 2, grp: "a", t: 2, score: 1, label: "a2"},
    {_id: 3, grp: "a", t: 3, score: 3, label: "a3"},
    {_id: 4, grp: "b", t: 1, score: 8, label: "b1"},
    {_id: 5, grp: "b", t: 2, score: 4, label: "b2"},
]));

const result = coll.aggregate([
    {
        $setWindowFields: {
            partitionBy: "$grp",
            sortBy: {t: 1},
            output: {
                firstTwo: {$firstN: {input: "$label", n: 2}, window: {documents: ["unbounded", "current"]}},
                lastTwo: {$lastN: {input: "$label", n: 2}, window: {documents: ["unbounded", "current"]}},
                minTwo: {$minN: {input: "$score", n: 2}, window: {documents: ["unbounded", "current"]}},
                maxTwo: {$maxN: {input: "$score", n: 2}, window: {documents: ["unbounded", "current"]}},
            },
        },
    },
    {$project: {_id: 0, grp: 1, t: 1, firstTwo: 1, lastTwo: 1, minTwo: 1, maxTwo: 1}},
]).toArray();

const expected = [
    {grp: "a", t: 1, firstTwo: ["a1"], lastTwo: ["a1"], minTwo: [5], maxTwo: [5]},
    {grp: "a", t: 2, firstTwo: ["a1", "a2"], lastTwo: ["a1", "a2"], minTwo: [1, 5], maxTwo: [5, 1]},
    {grp: "a", t: 3, firstTwo: ["a1", "a2"], lastTwo: ["a2", "a3"], minTwo: [1, 3], maxTwo: [5, 3]},
    {grp: "b", t: 1, firstTwo: ["b1"], lastTwo: ["b1"], minTwo: [8], maxTwo: [8]},
    {grp: "b", t: 2, firstTwo: ["b1", "b2"], lastTwo: ["b1", "b2"], minTwo: [4, 8], maxTwo: [8, 4]},
];

assert.eq(expected, result, tojson(result));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {t: 1},
            output: {bad: {$firstN: {input: "$label", n: 0}, window: {documents: ["unbounded", "current"]}}},
        },
    }],
    cursor: {},
}));
