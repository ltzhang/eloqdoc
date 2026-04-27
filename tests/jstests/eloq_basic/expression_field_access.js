const coll = db.expression_field_access;
coll.drop();

assert.commandWorked(coll.insert([
    {
        _id: 1,
        normal: "value",
        doc: {"a.b": 42, "$price": 12},
        fieldName: "a.b",
    },
]));

let docs = coll.aggregate([
    {
        $project: {
            _id: 0,
            dotted: {$getField: {field: "a.b", input: "$doc"}},
            dollar: {$getField: {field: {$literal: "$price"}, input: "$doc"}},
            dynamic: {$getField: {field: "$fieldName", input: "$doc"}},
            missingDefault: {$ifNull: [{$getField: "missing"}, "fallback"]},
        },
    },
]).toArray();
assert.eq([{dotted: 42, dollar: 12, dynamic: 42, missingDefault: "fallback"}], docs);

docs = coll.aggregate([
    {
        $project: {
            _id: 0,
            changed: {$setField: {field: "a.b", input: "$doc", value: 100}},
            removed: {$unsetField: {field: {$literal: "$price"}, input: "$doc"}},
            removeSentinel: {$setField: {field: {$literal: "$price"}, input: "$doc", value: "$$REMOVE"}},
        },
    },
]).toArray();
assert.eq([{
    changed: {"a.b": 100, "$price": 12},
    removed: {"a.b": 42},
    removeSentinel: {"a.b": 42},
}], docs);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$getField: {field: 3, input: "$doc"}}}}],
    cursor: {},
}));
