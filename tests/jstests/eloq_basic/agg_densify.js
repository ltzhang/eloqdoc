const coll = db.agg_densify;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, x: 1, kept: "a"},
    {_id: 2, x: 3, kept: "b"},
    {_id: 3, x: 5, kept: "c"},
]));

let result = coll.aggregate([
    {$sort: {x: 1}},
    {$densify: {field: "x", range: {step: 1, bounds: [1, 5]}}},
]).toArray();

assert.eq([
    {_id: 1, x: 1, kept: "a"},
    {x: 2},
    {_id: 2, x: 3, kept: "b"},
    {x: 4},
    {_id: 3, x: 5, kept: "c"},
], result);

coll.drop();
result = coll.aggregate([
    {$densify: {field: "x", range: {step: 2, bounds: [0, 4]}}},
]).toArray();
assert.eq([{x: 0}, {x: 2}, {x: 4}], result);

coll.drop();
assert.commandWorked(coll.insert([
    {_id: 1, sensor: "a", x: 1},
    {_id: 2, sensor: "a", x: 3},
    {_id: 3, sensor: "b", x: 2},
    {_id: 4, sensor: "b", x: 4},
]));

result = coll.aggregate([
    {$sort: {sensor: 1, x: 1}},
    {$densify: {field: "x", partitionByFields: ["sensor"], range: {step: 1, bounds: [1, 4]}}},
]).toArray();

assert.eq([
    {_id: 1, sensor: "a", x: 1},
    {sensor: "a", x: 2},
    {_id: 2, sensor: "a", x: 3},
    {sensor: "a", x: 4},
    {sensor: "b", x: 1},
    {_id: 3, sensor: "b", x: 2},
    {sensor: "b", x: 3},
    {_id: 4, sensor: "b", x: 4},
], result);

result = coll.aggregate([
    {$sort: {sensor: 1, x: 1}},
    {$densify: {field: "x", partitionByFields: ["sensor"], range: {step: 1, bounds: "partition"}}},
]).toArray();

assert.eq([
    {_id: 1, sensor: "a", x: 1},
    {sensor: "a", x: 2},
    {_id: 2, sensor: "a", x: 3},
    {_id: 3, sensor: "b", x: 2},
    {sensor: "b", x: 3},
    {_id: 4, sensor: "b", x: 4},
], result);

result = coll.aggregate([
    {$sort: {sensor: 1, x: 1}},
    {$densify: {field: "x", partitionByFields: ["sensor"], range: {step: 1, bounds: "full"}}},
]).toArray();

assert.eq([
    {_id: 1, sensor: "a", x: 1},
    {sensor: "a", x: 2},
    {_id: 2, sensor: "a", x: 3},
    {sensor: "a", x: 4},
    {sensor: "b", x: 1},
    {_id: 3, sensor: "b", x: 2},
    {sensor: "b", x: 3},
    {_id: 4, sensor: "b", x: 4},
], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [
        {$documents: [{x: 3}, {x: 1}]},
        {$densify: {field: "x", range: {step: 1, bounds: [1, 3]}}},
    ],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [
        {$densify: {field: "x", range: {step: 0, bounds: [1, 3]}}},
    ],
    cursor: {},
}));
