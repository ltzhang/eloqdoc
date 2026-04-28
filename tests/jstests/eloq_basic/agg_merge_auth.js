const admin = db.getSiblingDB("admin");
const testDB = db.getSiblingDB("test");
const source = testDB.agg_merge_auth_source;
const target = testDB.agg_merge_auth_target;

if (!admin.auth("mergeRoot", "pwd")) {
    const createRoot = admin.runCommand({createUser: "mergeRoot", pwd: "pwd", roles: ["root"]});
    if (!createRoot.ok) {
        print("Skipping $merge auth test because no root credentials are available: " +
              tojson(createRoot));
        quit();
    }
    assert(admin.auth("mergeRoot", "pwd"));
}

const cmdLine = assert.commandWorked(admin.runCommand({getCmdLineOpts: 1}));
if (!cmdLine.parsed.security || cmdLine.parsed.security.authorization !== "enabled") {
    print("Skipping $merge auth test because authorization is not enabled");
    quit();
}

source.drop();
target.drop();
assert.commandWorked(source.insert({_id: 1, item: "apple", qty: 5}));

testDB.dropUser("mergeSourceOnly");
testDB.dropUser("mergeSourceAndTarget");
testDB.dropRole("mergeSourceOnly");
testDB.dropRole("mergeSourceAndTarget");

assert.commandWorked(testDB.runCommand({
    createRole: "mergeSourceOnly",
    privileges: [{resource: {db: "test", collection: source.getName()}, actions: ["find"]}],
    roles: [],
}));
assert.commandWorked(testDB.runCommand({
    createRole: "mergeSourceAndTarget",
    privileges: [
        {resource: {db: "test", collection: source.getName()}, actions: ["find"]},
        {
            resource: {db: "test", collection: target.getName()},
            actions: ["find", "insert", "update"],
        },
    ],
    roles: [],
}));

assert.commandWorked(
    testDB.runCommand({createUser: "mergeSourceOnly", pwd: "pwd", roles: ["mergeSourceOnly"]}));
assert.commandWorked(testDB.runCommand(
    {createUser: "mergeSourceAndTarget", pwd: "pwd", roles: ["mergeSourceAndTarget"]}));

admin.logout();

let limitedConn = new Mongo(db.getMongo().host);
let limitedDB = limitedConn.getDB("test");
assert(limitedDB.auth("mergeSourceOnly", "pwd"));
assert.commandFailedWithCode(limitedDB.runCommand({
    aggregate: source.getName(),
    pipeline: [{$merge: target.getName()}],
    cursor: {},
}), ErrorCodes.Unauthorized);

let allowedConn = new Mongo(db.getMongo().host);
let allowedDB = allowedConn.getDB("test");
assert(allowedDB.auth("mergeSourceAndTarget", "pwd"));
assert.commandWorked(allowedDB.runCommand({
    aggregate: source.getName(),
    pipeline: [{$merge: target.getName()}],
    cursor: {},
}));
assert.eq([{_id: 1, item: "apple", qty: 5}], allowedDB.agg_merge_auth_target.find().toArray());
