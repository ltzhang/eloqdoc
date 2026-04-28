const coll = db.agg_set_window_fields_position_operators;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, store: "a", score: 10},
    {_id: 2, store: "a", score: 20},
    {_id: 3, store: "a", score: 20},
    {_id: 4, store: "a", score: 30},
    {_id: 5, store: "b", score: 5},
    {_id: 6, store: "b", score: 5},
]));

let result = coll.aggregate([
    {
        $setWindowFields: {
            partitionBy: "$store",
            sortBy: {score: 1},
            output: {
                docNum: {$documentNumber: {}},
                rank: {$rank: {}},
                denseRank: {$denseRank: {}},
                previousScore: {$shift: {output: "$score", by: -1, default: null}},
                nextScore: {$shift: {output: "$score", by: 1, default: "none"}},
            },
        },
    },
    {$project: {_id: 0, store: 1, score: 1, docNum: 1, rank: 1, denseRank: 1, previousScore: 1, nextScore: 1}},
]).toArray();

assert.eq([
    {store: "a", score: 10, docNum: 1, rank: 1, denseRank: 1, previousScore: null, nextScore: 20},
    {store: "a", score: 20, docNum: 2, rank: 2, denseRank: 2, previousScore: 10, nextScore: 20},
    {store: "a", score: 20, docNum: 3, rank: 2, denseRank: 2, previousScore: 20, nextScore: 30},
    {store: "a", score: 30, docNum: 4, rank: 4, denseRank: 3, previousScore: 20, nextScore: "none"},
    {store: "b", score: 5, docNum: 1, rank: 1, denseRank: 1, previousScore: null, nextScore: 5},
    {store: "b", score: 5, docNum: 2, rank: 1, denseRank: 1, previousScore: 5, nextScore: "none"},
], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {score: 1},
            output: {bad: {$shift: {output: "$score", by: 0}}},
        },
    }],
    cursor: {},
}));
