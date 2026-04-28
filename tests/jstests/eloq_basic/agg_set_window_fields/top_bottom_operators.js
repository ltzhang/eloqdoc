const coll = db.agg_set_window_fields_top_bottom_operators;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, grp: "a", t: 1, score: 5, label: "a1"},
    {_id: 2, grp: "a", t: 2, score: 9, label: "a2"},
    {_id: 3, grp: "a", t: 3, score: 7, label: "a3"},
    {_id: 4, grp: "b", t: 1, score: 3, label: "b1"},
    {_id: 5, grp: "b", t: 2, score: 4, label: "b2"},
]));

const result = coll.aggregate([
    {
        $setWindowFields: {
            partitionBy: "$grp",
            sortBy: {t: 1},
            output: {
                topOne: {$top: {sortBy: {score: -1}, output: "$label"}, window: {documents: ["unbounded", "current"]}},
                topTwo: {$topN: {sortBy: {score: -1}, output: "$label", n: 2}, window: {documents: ["unbounded", "current"]}},
                bottomOne: {$bottom: {sortBy: {score: -1}, output: "$label"}, window: {documents: ["unbounded", "current"]}},
                bottomTwo: {$bottomN: {sortBy: {score: -1}, output: "$label", n: 2}, window: {documents: ["unbounded", "current"]}},
            },
        },
    },
    {$project: {_id: 0, grp: 1, t: 1, topOne: 1, topTwo: 1, bottomOne: 1, bottomTwo: 1}},
]).toArray();

const expected = [
    {grp: "a", t: 1, topOne: "a1", topTwo: ["a1"], bottomOne: "a1", bottomTwo: ["a1"]},
    {grp: "a", t: 2, topOne: "a2", topTwo: ["a2", "a1"], bottomOne: "a1", bottomTwo: ["a1", "a2"]},
    {grp: "a", t: 3, topOne: "a2", topTwo: ["a2", "a3"], bottomOne: "a1", bottomTwo: ["a1", "a3"]},
    {grp: "b", t: 1, topOne: "b1", topTwo: ["b1"], bottomOne: "b1", bottomTwo: ["b1"]},
    {grp: "b", t: 2, topOne: "b2", topTwo: ["b2", "b1"], bottomOne: "b1", bottomTwo: ["b1", "b2"]},
];

assert.eq(expected, result, tojson(result));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {t: 1},
            output: {bad: {$topN: {sortBy: {score: -1}, output: "$label", n: 0}, window: {documents: ["unbounded", "current"]}}},
        },
    }],
    cursor: {},
}));
