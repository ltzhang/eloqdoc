const coll = db.envelope_field_passthrough;
coll.drop();

assert.commandWorked(coll.insert({_id: 1, x: 1}));

let res = assert.commandWorked(db.runCommand({
    find: coll.getName(),
    comment: "accepted globally",
    collectionUUID: UUID("71f1d1d7-68ca-493e-a7e9-f03c94e2e960"),
    isTimeseriesNamespace: false,
    mirrored: true,
    sampleId: UUID("81f1d1d7-68ca-493e-a7e9-f03c94e2e961"),
}));
assert.eq(1, res.cursor.firstBatch.length, tojson(res));
assert.eq(1, res.cursor.firstBatch[0]._id, tojson(res));
