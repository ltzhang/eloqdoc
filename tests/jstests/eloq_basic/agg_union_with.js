const source = db.agg_union_with_source;
const other = db.agg_union_with_other;
const third = db.agg_union_with_third;
const empty = db.agg_union_with_empty;

source.drop();
other.drop();
third.drop();
empty.drop();

assert.writeOK(source.insert([{_id: 1, src: "source"}, {_id: 2, src: "source"}]));
assert.writeOK(other.insert([
    {_id: 10, src: "other", keep: true},
    {_id: 11, src: "other", keep: false},
]));
assert.writeOK(third.insert([{_id: 20, src: "third"}]));

let result = source.aggregate([
    {$project: {_id: 1, src: 1}},
    {$unionWith: other.getName()},
    {$project: {_id: 1, src: 1}},
    {$sort: {_id: 1}},
]).toArray();
assert.eq([
    {_id: 1, src: "source"},
    {_id: 2, src: "source"},
    {_id: 10, src: "other"},
    {_id: 11, src: "other"},
], result);

result = source.aggregate([
    {$unionWith: {coll: other.getName(), pipeline: [{$match: {keep: true}}, {$project: {_id: 1}}]}},
    {$project: {_id: 1}},
    {$sort: {_id: 1}},
]).toArray();
assert.eq([{_id: 1}, {_id: 2}, {_id: 10}], result);

result = empty.aggregate([{$unionWith: other.getName()}, {$sort: {_id: 1}}]).toArray();
assert.eq([
    {_id: 10, src: "other", keep: true},
    {_id: 11, src: "other", keep: false},
], result);

result = source.aggregate([{$unionWith: empty.getName()}, {$sort: {_id: 1}}]).toArray();
assert.eq([{_id: 1, src: "source"}, {_id: 2, src: "source"}], result);

result = source.aggregate([
    {$unionWith: {coll: other.getName(), pipeline: [{$unionWith: third.getName()}]}},
    {$project: {_id: 1, src: 1}},
    {$sort: {_id: 1}},
]).toArray();
assert.eq([
    {_id: 1, src: "source"},
    {_id: 2, src: "source"},
    {_id: 10, src: "other"},
    {_id: 11, src: "other"},
    {_id: 20, src: "third"},
], result);
