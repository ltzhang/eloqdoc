const coll = db.agg_set_window_fields_integral_derivative;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, grp: "a", t: 0, y: 0},
    {_id: 2, grp: "a", t: 2, y: 4},
    {_id: 3, grp: "a", t: 5, y: 10},
    {_id: 4, grp: "b", t: 1, y: 2},
    {_id: 5, grp: "b", t: 4, y: 8},
]));

const result = coll.aggregate([
    {
        $setWindowFields: {
            partitionBy: "$grp",
            sortBy: {t: 1},
            output: {
                area: {$integral: {input: "$y"}, window: {documents: ["unbounded", "current"]}},
                rate: {$derivative: {input: "$y"}, window: {documents: ["unbounded", "current"]}},
            },
        },
    },
    {$project: {_id: 0, grp: 1, t: 1, area: 1, rate: 1}},
]).toArray();

const expected = [
    {grp: "a", t: 0, area: 0, rate: null},
    {grp: "a", t: 2, area: 4, rate: 2},
    {grp: "a", t: 5, area: 25, rate: 2},
    {grp: "b", t: 1, area: 0, rate: null},
    {grp: "b", t: 4, area: 15, rate: 2},
];

assert.eq(expected.length, result.length, tojson(result));
for (let i = 0; i < expected.length; ++i) {
    assert.eq(expected[i].grp, result[i].grp, tojson(result));
    assert.eq(expected[i].t, result[i].t, tojson(result));
    assert.close(expected[i].area, result[i].area, tojson(result), 8);
    if (expected[i].rate === null) {
        assert.eq(null, result[i].rate, tojson(result));
    } else {
        assert.close(expected[i].rate, result[i].rate, tojson(result), 8);
    }
}

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {t: 1},
            output: {bad: {$derivative: {input: "$y"}}},
        },
    }],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {t: 1},
            output: {
                bad: {
                    $integral: {input: "$y", unit: "second"},
                    window: {documents: ["unbounded", "current"]},
                },
            },
        },
    }],
    cursor: {},
}));
