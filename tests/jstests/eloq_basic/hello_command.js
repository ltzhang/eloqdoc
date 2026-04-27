const hello = assert.commandWorked(db.adminCommand({hello: 1}));
assert.eq(true, hello.isWritablePrimary, tojson(hello));
assert.eq(true, hello.helloOk, tojson(hello));

const strictHello =
    assert.commandWorked(db.adminCommand({hello: 1, apiVersion: "1", apiStrict: true}));
assert.eq(true, strictHello.isWritablePrimary, tojson(strictHello));
assert.eq(true, strictHello.helloOk, tojson(strictHello));

const isMaster = assert.commandWorked(db.adminCommand({isMaster: 1, helloOk: true}));
assert.eq(true, isMaster.helloOk, tojson(isMaster));
assert.eq(isMaster.ismaster, isMaster.isWritablePrimary, tojson(isMaster));
