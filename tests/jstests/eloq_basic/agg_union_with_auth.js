const admin = db.getSiblingDB("admin");
const testDB = db.getSiblingDB("test");
const source = testDB.agg_union_with_auth_source;
const other = testDB.agg_union_with_auth_other;

if (!admin.auth("unionWithRoot", "pwd")) {
    const createRoot = admin.runCommand({createUser: "unionWithRoot", pwd: "pwd", roles: ["root"]});
    if (!createRoot.ok) {
        print("Skipping $unionWith auth test because no root credentials are available: " +
              tojson(createRoot));
        quit();
    }
    assert(admin.auth("unionWithRoot", "pwd"));
}

const cmdLine = assert.commandWorked(admin.runCommand({getCmdLineOpts: 1}));
if (!cmdLine.parsed.security || cmdLine.parsed.security.authorization !== "enabled") {
    print("Skipping $unionWith auth test because authorization is not enabled");
    quit();
}

source.drop();
other.drop();
assert.commandWorked(source.insert({_id: 1, src: "source"}));
assert.commandWorked(other.insert({_id: 2, src: "other"}));

testDB.dropUser("sourceOnly");
testDB.dropUser("sourceAndOther");
testDB.dropRole("unionWithSourceOnly");
testDB.dropRole("unionWithSourceAndOther");

assert.commandWorked(testDB.runCommand({
    createRole: "unionWithSourceOnly",
    privileges: [{resource: {db: "test", collection: source.getName()}, actions: ["find"]}],
    roles: [],
}));
assert.commandWorked(testDB.runCommand({
    createRole: "unionWithSourceAndOther",
    privileges: [
        {resource: {db: "test", collection: source.getName()}, actions: ["find"]},
        {resource: {db: "test", collection: other.getName()}, actions: ["find"]},
    ],
    roles: [],
}));

assert.commandWorked(testDB.runCommand(
    {createUser: "sourceOnly", pwd: "pwd", roles: ["unionWithSourceOnly"]}));
assert.commandWorked(testDB.runCommand(
    {createUser: "sourceAndOther", pwd: "pwd", roles: ["unionWithSourceAndOther"]}));

admin.logout();

let limitedConn = new Mongo(db.getMongo().host);
let limitedDB = limitedConn.getDB("test");
assert(limitedDB.auth("sourceOnly", "pwd"));
assert.commandFailedWithCode(limitedDB.runCommand({
    aggregate: source.getName(),
    pipeline: [{$unionWith: other.getName()}],
    cursor: {},
}), ErrorCodes.Unauthorized);

let allowedConn = new Mongo(db.getMongo().host);
let allowedDB = allowedConn.getDB("test");
assert(allowedDB.auth("sourceAndOther", "pwd"));
const allowed = assert.commandWorked(allowedDB.runCommand({
    aggregate: source.getName(),
    pipeline: [{$unionWith: other.getName()}, {$sort: {_id: 1}}],
    cursor: {},
}));
assert.eq([{_id: 1, src: "source"}, {_id: 2, src: "other"}], allowed.cursor.firstBatch);
