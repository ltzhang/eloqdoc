const coll = db.analyze_basic;
const stats = db.getCollection("system.statistics.analyze_basic");

coll.drop();
assert.writeOK(stats.remove({}));

assert.writeOK(coll.insert([
    {_id: 1, category: "a", score: 10, ignored: {nested: true}},
    {_id: 2, category: "b", score: 20},
    {_id: 3, category: "a", score: 30},
    {_id: 4, category: "c", score: 40},
]));

let res = assert.commandWorked(db.runCommand({analyze: coll.getName(), key: "score"}));
assert.eq(coll.getName(), res.collection, tojson(res));
assert.eq(4, res.sampledDocuments, tojson(res));
assert.eq(["score"], res.fields, tojson(res));

let docs = stats.find().sort({field: 1}).toArray();
assert.eq(1, docs.length, tojson(docs));
assert.eq("score", docs[0].field, tojson(docs));
assert.eq(4, docs[0].cardinality, tojson(docs));
assert.eq(10, docs[0].min, tojson(docs));
assert.eq(40, docs[0].max, tojson(docs));
assert.gt(docs[0].histogram.length, 0, tojson(docs));

res = assert.commandWorked(db.runCommand({analyze: coll.getName()}));
assert.eq(4, res.sampledDocuments, tojson(res));
assert.eq(["category", "score"], res.fields.sort(), tojson(res));

docs = stats.find().sort({field: 1}).toArray();
assert.eq(2, docs.length, tojson(docs));
assert.eq(["category", "score"], docs.map(doc => doc.field), tojson(docs));

assert.commandWorked(db.runCommand({analyze: coll.getName(), key: "score", sampleSize: 2}));
docs = stats.find().sort({field: 1}).toArray();
assert.eq(1, docs.length, tojson(docs));
assert.eq(2, docs[0].sampledDocuments, tojson(docs));

res = assert.commandWorked(db.runCommand({analyze: coll.getName(), key: "score", sampleRate: 0.5}));
assert.eq(2, res.sampledDocuments, tojson(res));
docs = stats.find().sort({field: 1}).toArray();
assert.eq(1, docs.length, tojson(docs));
assert.eq(2, docs[0].sampledDocuments, tojson(docs));
