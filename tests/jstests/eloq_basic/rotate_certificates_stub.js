let result = db.adminCommand({rotateCertificates: 1, message: "eloq basic compatibility test"});
assert.commandWorked(result);
assert.eq(true, result.noop);
assert.neq(-1, result.note.indexOf("TLS certificate rotation is not implemented"), tojson(result));
