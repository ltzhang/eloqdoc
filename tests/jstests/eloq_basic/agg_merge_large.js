const source = db.agg_merge_large_source;
const target = db.agg_merge_large_target;
const nDocs = 10 * 1000;
const batchSize = 1000;

source.drop();
target.drop();

for (let batchStart = 0; batchStart < nDocs; batchStart += batchSize) {
    const bulk = source.initializeUnorderedBulkOp();
    for (let i = batchStart; i < batchStart + batchSize; ++i) {
        bulk.insert({_id: i, group: i % 10, value: i});
    }
    assert.commandWorked(bulk.execute());
}

source.aggregate([{$merge: target.getName()}]).toArray();

assert.eq(nDocs, target.count());
assert.eq({_id: 0, group: 0, value: 0}, target.findOne({_id: 0}));
assert.eq({_id: nDocs - 1, group: 9, value: nDocs - 1}, target.findOne({_id: nDocs - 1}));
