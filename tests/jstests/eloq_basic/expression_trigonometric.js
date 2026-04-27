const coll = db.expression_trigonometric;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, x: 0, y: 1, angle: 180},
    {_id: 2, x: 1, y: 1, angle: 90},
    {_id: 3, x: null, y: 1, angle: null},
    {_id: 4, x: NumberDecimal("0.5"), y: NumberDecimal("1"), angle: NumberDecimal("180")},
]));

function assertApprox(actual, expected, epsilon, message) {
    assert.lte(Math.abs(actual - expected), epsilon, message + ": " + tojson(actual));
}

function decimalToNumber(value) {
    return parseFloat(tojson(value).replace(/^NumberDecimal\("([^"]+)"\)$/, "$1"));
}

let docs = coll.aggregate([
    {$sort: {_id: 1}},
    {
        $project: {
            _id: 1,
            sin: {$sin: "$x"},
            cos: {$cos: "$x"},
            tan: {$tan: "$x"},
            asin: {$asin: "$x"},
            acos: {$acos: "$x"},
            atan: {$atan: "$x"},
            atan2: {$atan2: ["$y", "$x"]},
            sinh: {$sinh: "$x"},
            cosh: {$cosh: "$x"},
            tanh: {$tanh: "$x"},
            asinh: {$asinh: "$x"},
            acosh: {$acosh: "$y"},
            atanh: {$atanh: "$x"},
            radians: {$degreesToRadians: "$angle"},
            degrees: {$radiansToDegrees: {$degreesToRadians: "$angle"}},
        },
    },
]).toArray();

assertApprox(docs[0].sin, 0, 1e-12, "sin(0)");
assertApprox(docs[0].cos, 1, 1e-12, "cos(0)");
assertApprox(docs[0].tan, 0, 1e-12, "tan(0)");
assertApprox(docs[0].asin, 0, 1e-12, "asin(0)");
assertApprox(docs[0].acos, Math.PI / 2, 1e-12, "acos(0)");
assertApprox(docs[0].atan, 0, 1e-12, "atan(0)");
assertApprox(docs[0].atan2, Math.PI / 2, 1e-12, "atan2(1, 0)");
assertApprox(docs[0].sinh, 0, 1e-12, "sinh(0)");
assertApprox(docs[0].cosh, 1, 1e-12, "cosh(0)");
assertApprox(docs[0].tanh, 0, 1e-12, "tanh(0)");
assertApprox(docs[0].asinh, 0, 1e-12, "asinh(0)");
assertApprox(docs[0].acosh, 0, 1e-12, "acosh(1)");
assertApprox(docs[0].atanh, 0, 1e-12, "atanh(0)");
assertApprox(docs[0].radians, Math.PI, 1e-12, "degreesToRadians(180)");
assertApprox(docs[0].degrees, 180, 1e-10, "radiansToDegrees(pi)");

assertApprox(docs[1].sin, Math.sin(1), 1e-12, "sin(1)");
assertApprox(docs[1].cos, Math.cos(1), 1e-12, "cos(1)");
assertApprox(docs[1].tan, Math.tan(1), 1e-12, "tan(1)");
assertApprox(docs[1].atan2, Math.PI / 4, 1e-12, "atan2(1, 1)");
assertApprox(docs[1].degrees, 90, 1e-10, "round trip 90 degrees");

assert.eq(null, docs[2].sin);
assert.eq(null, docs[2].atan2);
assert.eq(null, docs[2].radians);

assert.eq(0, tojson(docs[3].sin).indexOf("NumberDecimal"));
assert.eq(0, tojson(docs[3].atan2).indexOf("NumberDecimal"));
assert.eq(0, tojson(docs[3].radians).indexOf("NumberDecimal"));
assertApprox(decimalToNumber(docs[3].sin), Math.sin(0.5), 1e-12, "decimal sin(0.5)");
assertApprox(decimalToNumber(docs[3].atan2), Math.atan2(1, 0.5), 1e-12, "decimal atan2(1, 0.5)");
assertApprox(decimalToNumber(docs[3].radians), Math.PI, 1e-12, "decimal degreesToRadians(180)");

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$sin: "not numeric"}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$asin: 2}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$atanh: 2}}}],
    cursor: {},
}));
