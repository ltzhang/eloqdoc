const coll = db.commit_quorum_stub;
coll.drop();

function createIndexWithCommitQuorum(indexName, commitQuorum) {
    return db.runCommand({
        createIndexes: coll.getName(),
        indexes: [{key: {[indexName]: 1}, name: indexName + "_1"}],
        commitQuorum: commitQuorum,
    });
}

assert.commandWorked(createIndexWithCommitQuorum("cq_zero", 0));
assert.commandWorked(createIndexWithCommitQuorum("cq_one", 1));
assert.commandWorked(createIndexWithCommitQuorum("cq_majority", "majority"));
assert.commandWorked(createIndexWithCommitQuorum("cq_voting_members", "votingMembers"));
assert.commandWorked(createIndexWithCommitQuorum("cq_tag", "eastCoast"));

assert.commandFailedWithCode(createIndexWithCommitQuorum("cq_bool", true), ErrorCodes.TypeMismatch);

assert.neq(null, coll.getIndexes().filter((idx) => idx.name === "cq_zero_1")[0], tojson(coll.getIndexes()));
assert.neq(null, coll.getIndexes().filter((idx) => idx.name === "cq_one_1")[0], tojson(coll.getIndexes()));
