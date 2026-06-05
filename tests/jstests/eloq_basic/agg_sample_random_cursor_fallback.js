const coll = db.eloq_sample_random_cursor_fallback;
coll.drop();

const bulk = coll.initializeUnorderedBulkOp();
for (let i = 0; i < 200; ++i) {
    bulk.insert({_id: i, value: i});
}
assert.commandWorked(bulk.execute());

// With 200 records and sample size 5, the aggregation planner attempts the optimized random cursor
// path. Eloq should report unsupported random cursors by returning null and letting $sample use its
// generic random-sort fallback.
const docs = coll.aggregate([{$sample: {size: 5}}]).toArray();
assert.eq(5, docs.length, tojson(docs));
assert.eq(5, Array.unique(docs.map(doc => doc._id)).length, tojson(docs));
