const coll = db.agg_set_window_fields_percentile_median;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, grp: "a", t: 1, value: 1},
    {_id: 2, grp: "a", t: 2, value: 3},
    {_id: 3, grp: "a", t: 3, value: 5},
    {_id: 4, grp: "a", t: 4, value: "skip"},
    {_id: 5, grp: "b", t: 1, value: 10},
    {_id: 6, grp: "b", t: 2, value: 20},
]));

const result = coll.aggregate([
    {
        $setWindowFields: {
            partitionBy: "$grp",
            sortBy: {t: 1},
            output: {
                pct: {
                    $percentile: {input: "$value", p: [0, 0.5, 1], method: "approximate"},
                    window: {documents: ["unbounded", "current"]},
                },
                med: {
                    $median: {input: "$value", method: "approximate"},
                    window: {documents: ["unbounded", "current"]},
                },
            },
        },
    },
    {$project: {_id: 0, grp: 1, t: 1, pct: 1, med: 1}},
]).toArray();

const expected = [
    {grp: "a", t: 1, pct: [1, 1, 1], med: 1},
    {grp: "a", t: 2, pct: [1, 2, 3], med: 2},
    {grp: "a", t: 3, pct: [1, 3, 5], med: 3},
    {grp: "a", t: 4, pct: [1, 3, 5], med: 3},
    {grp: "b", t: 1, pct: [10, 10, 10], med: 10},
    {grp: "b", t: 2, pct: [10, 15, 20], med: 15},
];

assert.eq(expected.length, result.length, tojson(result));
for (let i = 0; i < expected.length; ++i) {
    assert.eq(expected[i].grp, result[i].grp, tojson(result));
    assert.eq(expected[i].t, result[i].t, tojson(result));
    assert.eq(expected[i].pct.length, result[i].pct.length, tojson(result));
    for (let j = 0; j < expected[i].pct.length; ++j) {
        assert.close(expected[i].pct[j], result[i].pct[j], tojson(result), 8);
    }
    assert.close(expected[i].med, result[i].med, tojson(result), 8);
}

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {t: 1},
            output: {
                bad: {
                    $percentile: {input: "$value", p: [1.1], method: "approximate"},
                    window: {documents: ["unbounded", "current"]},
                },
            },
        },
    }],
    cursor: {},
}));
