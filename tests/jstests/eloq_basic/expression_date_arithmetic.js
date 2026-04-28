const coll = db.expression_date_arithmetic;
coll.drop();

assert.commandWorked(coll.insert([
    {
        _id: 1,
        jan31: ISODate("2024-01-31T10:15:30.123Z"),
        leapDay: ISODate("2024-02-29T12:00:00.000Z"),
        start: ISODate("2024-01-01T00:00:00.000Z"),
        end: ISODate("2024-01-03T12:30:00.000Z"),
        dstStart: ISODate("2026-03-08T06:30:00.000Z"),
        toTrunc: ISODate("2024-01-03T12:34:56.789Z"),
    },
    {_id: 2, jan31: null, start: null, end: ISODate("2024-01-01T00:00:00.000Z")},
]));

let docs = coll.aggregate([
    {$sort: {_id: 1}},
    {
        $project: {
            _id: 1,
            addMonth: {$dateAdd: {startDate: "$jan31", unit: "month", amount: 1}},
            subtractYear: {$dateSubtract: {startDate: "$leapDay", unit: "year", amount: 1}},
            addDstDay: {
                $dateAdd: {
                    startDate: "$dstStart",
                    unit: "day",
                    amount: 1,
                    timezone: "America/New_York",
                },
            },
            diffHours: {$dateDiff: {startDate: "$start", endDate: "$end", unit: "hour"}},
            diffDays: {$dateDiff: {startDate: "$start", endDate: "$end", unit: "day"}},
            truncHour: {$dateTrunc: {date: "$toTrunc", unit: "hour"}},
            truncFiveMinutes: {$dateTrunc: {date: "$toTrunc", unit: "minute", binSize: 5}},
            truncWeekMonday: {
                $dateTrunc: {date: "$toTrunc", unit: "week", startOfWeek: "monday"},
            },
        },
    },
]).toArray();

assert.eq(ISODate("2024-02-29T10:15:30.123Z"), docs[0].addMonth);
assert.eq(ISODate("2023-02-28T12:00:00.000Z"), docs[0].subtractYear);
assert.eq(ISODate("2026-03-09T05:30:00.000Z"), docs[0].addDstDay);
assert.eq(60, docs[0].diffHours);
assert.eq(2, docs[0].diffDays);
assert.eq(ISODate("2024-01-03T12:00:00.000Z"), docs[0].truncHour);
assert.eq(ISODate("2024-01-03T12:30:00.000Z"), docs[0].truncFiveMinutes);
assert.eq(ISODate("2024-01-01T00:00:00.000Z"), docs[0].truncWeekMonday);

assert.eq(null, docs[1].addMonth);
assert.eq(null, docs[1].diffDays);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$dateAdd: {startDate: "$jan31", unit: "fortnight", amount: 1}}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$dateTrunc: {date: "$jan31", unit: "hour", binSize: 0}}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$dateAdd: {startDate: "$jan31", amount: 1}}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$dateDiff: {startDate: "$start", endDate: "$end"}}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$dateTrunc: {date: "$jan31"}}}}],
    cursor: {},
}));

function getSerializedOut(expr) {
    const serialized = db.runCommand({
        aggregate: coll.getName(),
        pipeline: [{$project: {_id: 0, out: expr}}],
        cursor: {},
        explain: true,
    });
    assert.commandWorked(serialized);
    assert(serialized.stages, tojson(serialized));
    for (let i = 0; i < serialized.stages.length; ++i) {
        if (serialized.stages[i].$project) {
            return serialized.stages[i].$project.out;
        }
    }
    throw new Error("missing serialized $project stage: " + tojson(serialized));
}

// Serialize round-trip: the serialized form must include required fields and be parseable.
// This catches missing required fields (e.g. unit, timezone, startOfWeek) in serialize().
function assertSerializedFields(expr, opName, expectedFields) {
    const out = getSerializedOut(expr);
    assert(out[opName], tojson(out));
    expectedFields.forEach((field) => assert.neq(undefined, out[opName][field], tojson(out)));
    assert.commandWorked(db.runCommand({
        aggregate: coll.getName(),
        pipeline: [{$project: {_id: 0, out: out}}],
        cursor: {},
    }));
}

assertSerializedFields({$dateAdd: {startDate: ISODate("2024-01-01"), unit: "month", amount: 1}},
                       "$dateAdd",
                       ["startDate", "unit", "amount"]);
assertSerializedFields({
    $dateAdd: {startDate: ISODate("2024-01-01"), unit: "day", amount: 1, timezone: "America/New_York"},
}, "$dateAdd", ["startDate", "unit", "amount", "timezone"]);
assertSerializedFields({$dateSubtract: {startDate: ISODate("2024-06-01"), unit: "week", amount: 2}},
                       "$dateSubtract",
                       ["startDate", "unit", "amount"]);
assertSerializedFields({
    $dateDiff: {startDate: ISODate("2024-01-01"), endDate: ISODate("2024-06-01"), unit: "day"},
}, "$dateDiff", ["startDate", "endDate", "unit"]);
assertSerializedFields({$dateTrunc: {date: ISODate("2024-01-15T12:34:56Z"), unit: "hour"}},
                       "$dateTrunc",
                       ["date", "unit", "binSize"]);
assertSerializedFields({
    $dateTrunc: {date: ISODate("2024-01-15T12:34:56Z"), unit: "week", startOfWeek: "monday"},
}, "$dateTrunc", ["date", "unit", "binSize", "startOfWeek"]);
