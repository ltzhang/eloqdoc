function getCollectionInfo(collName) {
    const result =
        db.runCommand({listCollections: 1, filter: {name: collName}, nameOnly: false});
    assert.commandWorked(result);
    assert.eq(1, result.cursor.firstBatch.length, tojson(result));
    return result.cursor.firstBatch[0];
}

const collName = "pre_post_images_options";
db[collName].drop();

assert.commandWorked(db.createCollection(
    collName, {changeStreamPreAndPostImages: {enabled: true}}));

let info = getCollectionInfo(collName);
assert.eq(true, info.options.changeStreamPreAndPostImages.enabled, tojson(info));

assert.commandWorked(
    db.runCommand({collMod: collName, changeStreamPreAndPostImages: {enabled: false}}));

info = getCollectionInfo(collName);
assert.eq(false, info.options.changeStreamPreAndPostImages.enabled, tojson(info));

const legacyCollName = "record_pre_images_options";
db[legacyCollName].drop();

assert.commandWorked(db.createCollection(legacyCollName, {recordPreImages: true}));

info = getCollectionInfo(legacyCollName);
assert.eq(true, info.options.recordPreImages, tojson(info));

assert.commandFailed(
    db.createCollection("bad_pre_post_images_options",
                        {changeStreamPreAndPostImages: {enabled: "yes"}}));
