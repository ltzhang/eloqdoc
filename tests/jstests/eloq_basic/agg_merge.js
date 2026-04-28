const source = db.agg_merge_source;
const target = db.agg_merge_target;
const otherDb = db.getSiblingDB("agg_merge_other_db");
const crossTarget = otherDb.agg_merge_cross_target;

source.drop();
target.drop();
crossTarget.drop();

assert.commandWorked(source.insert([
    {_id: 1, item: "apple", qty: 5},
    {_id: 2, item: "banana", qty: 7},
]));

source.aggregate([{$merge: target.getName()}]).toArray();
assert.eq([
    {_id: 1, item: "apple", qty: 5},
    {_id: 2, item: "banana", qty: 7},
], target.find().sort({_id: 1}).toArray());

assert.commandWorked(source.update({_id: 1}, {$set: {qty: 8, color: "red"}}));
source.aggregate([{$match: {_id: 1}}, {$merge: {into: target.getName(), whenMatched: "merge"}}])
    .toArray();
assert.eq({_id: 1, item: "apple", qty: 8, color: "red"}, target.findOne({_id: 1}));

assert.commandWorked(source.update({_id: 1}, {$unset: {color: 1}, $set: {qty: 9}}));
source.aggregate([{$match: {_id: 1}}, {$merge: {into: target.getName(), whenMatched: "replace"}}])
    .toArray();
assert.eq({_id: 1, item: "apple", qty: 9}, target.findOne({_id: 1}));

assert.commandWorked(source.update({_id: 1}, {$set: {qty: 11, extra: true}}));
source.aggregate([
    {$match: {_id: 1}},
    {$merge: {into: target.getName(), whenMatched: "keepExisting"}},
]).toArray();
assert.eq({_id: 1, item: "apple", qty: 9}, target.findOne({_id: 1}));

assert.commandWorked(source.insert({_id: 3, item: "cherry", qty: 3}));
source.aggregate([
    {$match: {_id: 3}},
    {$merge: {into: target.getName(), whenNotMatched: "discard"}},
]).toArray();
assert.eq(0, target.count({_id: 3}));

assert.commandFailedWithCode(db.runCommand({
    aggregate: source.getName(),
    pipeline: [
        {$match: {_id: 1}},
        {$merge: {into: target.getName(), whenMatched: "fail"}},
    ],
    cursor: {},
}), ErrorCodes.DuplicateKey);

assert.commandFailedWithCode(db.runCommand({
    aggregate: source.getName(),
    pipeline: [
        {$match: {_id: 3}},
        {$merge: {into: target.getName(), whenNotMatched: "fail"}},
    ],
    cursor: {},
}), ErrorCodes.NoMatchingDocument);

assert.commandWorked(target.update({_id: 2}, {$set: {qty: 10, seen: 1}}));
assert.commandWorked(source.update({_id: 2}, {$set: {qty: 4, item: "banana-new"}}));
source.aggregate([
    {$match: {_id: 2}},
    {
        $merge: {
            into: target.getName(),
            whenMatched: [
                {
                    $set: {
                        qty: {$add: ["$qty", "$$new.qty"]},
                        item: "$$new.item",
                        seen: {$add: ["$seen", 1]},
                    },
                },
            ],
        },
    },
]).toArray();
assert.eq({_id: 2, item: "banana-new", qty: 14, seen: 2}, target.findOne({_id: 2}));

assert.commandWorked(target.update({_id: 2}, {$set: {qty: 20, seen: 2}}));
assert.commandWorked(source.update({_id: 2}, {$set: {qty: 6, item: "banana-let"}}));
source.aggregate([
    {$match: {_id: 2}},
    {
        $merge: {
            into: target.getName(),
            let: {incomingQty: "$qty", incomingItem: "$item"},
            whenMatched: [
                {$set: {qty: {$add: ["$qty", "$$incomingQty"]}, item: "$$incomingItem"}},
            ],
        },
    },
]).toArray();
assert.eq({_id: 2, item: "banana-let", qty: 26, seen: 2}, target.findOne({_id: 2}));

assert.commandWorked(source.insert({_id: 4, sku: {code: "dotted"}, qty: 2}));
assert.commandWorked(target.insert({_id: 40, sku: {code: "dotted"}, qty: 1, existing: true}));
source.aggregate([
    {$match: {_id: 4}},
    {$merge: {into: target.getName(), on: "sku.code", whenMatched: "merge"}},
]).toArray();
assert.eq({_id: 40, sku: {code: "dotted"}, qty: 2, existing: true},
          target.findOne({"sku.code": "dotted"}));

source.aggregate([
    {$match: {_id: 3}},
    {$merge: {into: {db: otherDb.getName(), coll: crossTarget.getName()}}},
]).toArray();
assert.eq([{_id: 3, item: "cherry", qty: 3}], crossTarget.find().toArray());
