const coll = db.agg_plan_cache_stats;
coll.drop();

assert.commandWorked(db.createCollection(coll.getName()));
assert.eq([], coll.aggregate([{$planCacheStats: {}}]).toArray());

let bulk = coll.initializeUnorderedBulkOp();
for (let i = 0; i < 200; ++i) {
    bulk.insert({_id: i, a: i % 10, b: i % 7, c: i});
}
assert.commandWorked(bulk.execute());
assert.commandWorked(coll.createIndex({a: 1}));
assert.commandWorked(coll.createIndex({b: 1}));
assert.commandWorked(coll.createIndex({a: 1, b: 1}));

for (let i = 0; i < 20; ++i) {
    coll.find({a: {$gte: 1}, b: {$gte: 1}}).sort({c: 1}).itcount();
}

let legacyShapes = assert.commandWorked(db.runCommand({
    planCacheListQueryShapes: coll.getName(),
})).shapes;

let stats = coll.aggregate([{$planCacheStats: {}}]).toArray();
assert.eq(legacyShapes.length, stats.length);
assert.gt(stats.length, 0, tojson({legacyShapes: legacyShapes, stats: stats}));

const entry = stats[0];
assert(entry.hasOwnProperty("queryHash"), tojson(entry));
assert(entry.hasOwnProperty("planCacheKey"), tojson(entry));
assert(entry.hasOwnProperty("query"), tojson(entry));
assert(entry.hasOwnProperty("sort"), tojson(entry));
assert(entry.hasOwnProperty("projection"), tojson(entry));
assert(entry.hasOwnProperty("plans"), tojson(entry));
assert(Array.isArray(entry.plans), tojson(entry));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [
        {$match: {}},
        {$planCacheStats: {}},
    ],
    cursor: {},
}));

assert.commandFailedWithCode(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$planCacheStats: {unexpected: true}}],
    cursor: {},
}), ErrorCodes.FailedToParse);

coll.drop();
assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$planCacheStats: {}}],
    cursor: {},
}));
