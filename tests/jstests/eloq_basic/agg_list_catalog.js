function byName(name) {
    return function(doc) {
        return doc.name === name;
    };
}

const coll = db.agg_list_catalog;
coll.drop();

assert.commandWorked(db.createCollection(coll.getName(), {
    changeStreamPreAndPostImages: {enabled: true},
}));
assert.commandWorked(coll.createIndex({x: 1}, {name: "x_1"}));

let result = db.runCommand({aggregate: 1, pipeline: [{$listCatalog: {}}], cursor: {}});
assert.commandWorked(result);

let entry = result.cursor.firstBatch.filter(byName(coll.getName()))[0];
assert.neq(null, entry, tojson(result));
assert.eq(db.getName(), entry.db, tojson(entry));
assert.eq(coll.getName(), entry.name, tojson(entry));
assert.eq("collection", entry.type, tojson(entry));
assert.eq(coll.getFullName(), entry.ns, tojson(entry));
assert.eq(true, entry.md.options.changeStreamPreAndPostImages.enabled, tojson(entry));
assert.eq(2, entry.md.indexes.length, tojson(entry));
assert.neq(null, entry.md.indexes.filter(byName("x_1"))[0], tojson(entry));

result = db.runCommand({aggregate: coll.getName(), pipeline: [{$listCatalog: {}}], cursor: {}});
assert.commandWorked(result);
assert.eq(1, result.cursor.firstBatch.length, tojson(result));
entry = result.cursor.firstBatch[0];
assert.eq(coll.getName(), entry.name, tojson(entry));

assert.commandFailed(db.runCommand({
    aggregate: 1,
    pipeline: [{$match: {}}, {$listCatalog: {}}],
    cursor: {},
}));
