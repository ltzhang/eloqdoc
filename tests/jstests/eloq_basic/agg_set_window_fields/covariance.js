const coll = db.agg_set_window_fields_covariance;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, grp: "a", t: 1, x: 1, y: 2},
    {_id: 2, grp: "a", t: 2, x: 2, y: 4},
    {_id: 3, grp: "a", t: 3, x: 3, y: 6},
    {_id: 4, grp: "b", t: 1, x: 10, y: 1},
    {_id: 5, grp: "b", t: 2, x: 20, y: 3},
]));

const result = coll.aggregate([
    {
        $setWindowFields: {
            partitionBy: "$grp",
            sortBy: {t: 1},
            output: {
                pop: {$covariancePop: ["$x", "$y"], window: {documents: ["unbounded", "current"]}},
                samp: {$covarianceSamp: ["$x", "$y"], window: {documents: ["unbounded", "current"]}},
            },
        },
    },
    {$project: {_id: 0, grp: 1, t: 1, pop: 1, samp: 1}},
]).toArray();

const expected = [
    {grp: "a", t: 1, pop: 0, samp: null},
    {grp: "a", t: 2, pop: 0.5, samp: 1},
    {grp: "a", t: 3, pop: 4 / 3, samp: 2},
    {grp: "b", t: 1, pop: 0, samp: null},
    {grp: "b", t: 2, pop: 5, samp: 10},
];

assert.eq(expected.length, result.length, tojson(result));
for (let i = 0; i < expected.length; ++i) {
    assert.eq(expected[i].grp, result[i].grp, tojson(result));
    assert.eq(expected[i].t, result[i].t, tojson(result));
    assert.close(expected[i].pop, result[i].pop, tojson(result), 8);
    if (expected[i].samp === null) {
        assert.eq(null, result[i].samp, tojson(result));
    } else {
        assert.close(expected[i].samp, result[i].samp, tojson(result), 8);
    }
}

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {t: 1},
            output: {bad: {$covariancePop: ["$x"]}},
        },
    }],
    cursor: {},
}));
