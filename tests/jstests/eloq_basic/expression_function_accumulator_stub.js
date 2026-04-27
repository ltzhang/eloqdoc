const coll = db.expression_function_accumulator_stub;
coll.drop();

assert.commandWorked(coll.insert([{_id: 1, value: 2}, {_id: 2, value: 3}]));

let functionResult = db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $project: {
            computed: {
                $function: {
                    body: function(x) {
                        return x + 1;
                    },
                    args: ["$value"],
                    lang: "js",
                },
            },
        },
    }],
    cursor: {},
});
assert.commandFailed(functionResult);
assert.neq(-1, functionResult.errmsg.indexOf("server-side scripting disabled"), tojson(functionResult));

let accumulatorResult = db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $group: {
            _id: null,
            total: {
                $accumulator: {
                    init: function() {
                        return 0;
                    },
                    accumulate: function(state, value) {
                        return state + value;
                    },
                    accumulateArgs: ["$value"],
                    merge: function(left, right) {
                        return left + right;
                    },
                    finalize: function(state) {
                        return state;
                    },
                    lang: "js",
                },
            },
        },
    }],
    cursor: {},
});
assert.commandFailed(accumulatorResult);
assert.neq(-1,
           accumulatorResult.errmsg.indexOf("server-side scripting disabled"),
           tojson(accumulatorResult));
