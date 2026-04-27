const coll = db.expression_regex_replace;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, text: "abc 123 abc", pattern: "abc", replacement: "X"},
    {_id: 2, text: "ABC 456", pattern: "[a-z]+", replacement: "word"},
    {_id: 3, text: "no digits here", pattern: "\\d+", replacement: "#"},
    {_id: 4, text: null, pattern: "abc", replacement: "X"},
]));

let docs = coll.aggregate([
    {$sort: {_id: 1}},
    {
        $project: {
            _id: 1,
            matched: {$regexMatch: {input: "$text", regex: "$pattern"}},
            insensitive: {$regexMatch: {input: "$text", regex: "$pattern", options: "i"}},
            first: {$regexFind: {input: "$text", regex: "$pattern", options: "i"}},
            allDigits: {$regexFindAll: {input: "$text", regex: "\\d"}},
            replaceOne: {$replaceOne: {input: "$text", find: "$pattern", replacement: "$replacement"}},
            replaceAll: {$replaceAll: {input: "$text", find: "$pattern", replacement: "$replacement"}},
        },
    },
]).toArray();

assert.eq(true, docs[0].matched);
assert.eq(true, docs[0].insensitive);
assert.eq({match: "abc", idx: 0, captures: []}, docs[0].first);
assert.eq([{match: "1", idx: 4, captures: []},
           {match: "2", idx: 5, captures: []},
           {match: "3", idx: 6, captures: []}], docs[0].allDigits);
assert.eq("X 123 abc", docs[0].replaceOne);
assert.eq("X 123 X", docs[0].replaceAll);

assert.eq(false, docs[1].matched);
assert.eq(true, docs[1].insensitive);
assert.eq({match: "ABC", idx: 0, captures: []}, docs[1].first);
assert.eq([{match: "4", idx: 4, captures: []},
           {match: "5", idx: 5, captures: []},
           {match: "6", idx: 6, captures: []}], docs[1].allDigits);

assert.eq(false, docs[2].matched);
assert.eq(null, docs[2].first);
assert.eq([], docs[2].allDigits);
assert.eq("no digits here", docs[2].replaceOne);
assert.eq("no digits here", docs[2].replaceAll);

assert.eq(null, docs[3].matched);
assert.eq(null, docs[3].first);
assert.eq(null, docs[3].allDigits);
assert.eq(null, docs[3].replaceOne);
assert.eq(null, docs[3].replaceAll);

let captured = coll.aggregate([
    {$match: {_id: 1}},
    {$project: {m: {$regexFind: {input: "$text", regex: "(abc) (\\d+)"}}}},
]).toArray()[0].m;
assert.eq("abc 123", captured.match);
assert.eq(0, captured.idx);
assert.eq(["abc", "123"], captured.captures);

assert.eq(1, coll.aggregate([
    {$match: {$expr: {$regexMatch: {input: "$text", regex: "456"}}}},
]).itcount());

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$regexMatch: {input: 123, regex: "123"}}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$regexFind: {input: "$text", regex: "["}}}}],
    cursor: {},
}));

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{$project: {bad: {$replaceOne: {input: "$text", find: "$pattern"}}}}],
    cursor: {},
}));
