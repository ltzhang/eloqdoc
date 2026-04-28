const coll = db.agg_set_window_fields_basic_documents;
coll.drop();

assert.commandWorked(coll.insert([
    {_id: 1, store: "b", day: 2, qty: 8, price: 3},
    {_id: 2, store: "a", day: 1, qty: 5, price: 10},
    {_id: 3, store: "a", day: 3, qty: 7, price: 20},
    {_id: 4, store: "a", day: 2, qty: 4, price: 30},
    {_id: 5, store: "b", day: 1, qty: 6, price: 4},
]));

let result = coll.aggregate([
    {
        $setWindowFields: {
            partitionBy: "$store",
            sortBy: {day: 1},
            output: {
                runningQty: {$sum: "$qty", window: {documents: ["unbounded", "current"]}},
                trailingQty: {$sum: "$qty", window: {documents: [-1, "current"]}},
                avgPrice: {$avg: "$price", window: {documents: [-1, 1]}},
                rowCount: {$count: {}, window: {documents: ["unbounded", "current"]}},
                firstQty: {$first: "$qty", window: {documents: ["unbounded", "current"]}},
                lastQty: {$last: "$qty", window: {documents: [-1, "current"]}},
            },
        },
    },
    {$project: {_id: 0, store: 1, day: 1, qty: 1, runningQty: 1, trailingQty: 1, avgPrice: 1, rowCount: 1, firstQty: 1, lastQty: 1}},
]).toArray();

assert.eq([
    {store: "a", day: 1, qty: 5, runningQty: 5, trailingQty: 5, avgPrice: 20, rowCount: 1, firstQty: 5, lastQty: 5},
    {store: "a", day: 2, qty: 4, runningQty: 9, trailingQty: 9, avgPrice: 20, rowCount: 2, firstQty: 5, lastQty: 4},
    {store: "a", day: 3, qty: 7, runningQty: 16, trailingQty: 11, avgPrice: 25, rowCount: 3, firstQty: 5, lastQty: 7},
    {store: "b", day: 1, qty: 6, runningQty: 6, trailingQty: 6, avgPrice: 3.5, rowCount: 1, firstQty: 6, lastQty: 6},
    {store: "b", day: 2, qty: 8, runningQty: 14, trailingQty: 14, avgPrice: 3.5, rowCount: 2, firstQty: 6, lastQty: 8},
], result);

result = coll.aggregate([
    {
        $setWindowFields: {
            sortBy: {day: 1},
            output: {
                minQty: {$min: "$qty", window: {documents: ["unbounded", "current"]}},
                maxQty: {$max: "$qty", window: {documents: ["unbounded", "current"]}},
            },
        },
    },
    {$project: {_id: 0, day: 1, qty: 1, minQty: 1, maxQty: 1}},
]).toArray();

assert.eq([
    {day: 1, qty: 5, minQty: 5, maxQty: 5},
    {day: 1, qty: 6, minQty: 5, maxQty: 6},
    {day: 2, qty: 8, minQty: 5, maxQty: 8},
    {day: 2, qty: 4, minQty: 4, maxQty: 8},
    {day: 3, qty: 7, minQty: 4, maxQty: 8},
], result);

result = coll.aggregate([
    {
        $setWindowFields: {
            sortBy: {_id: 1},
            output: {
                emptySum: {$sum: "$qty", window: {documents: [1, -1]}},
                emptyAvg: {$avg: "$qty", window: {documents: [1, -1]}},
                emptyCount: {$count: {}, window: {documents: [1, -1]}},
            },
        },
    },
    {$match: {_id: 1}},
    {$project: {_id: 0, emptySum: 1, emptyAvg: 1, emptyCount: 1}},
]).toArray();

assert.eq([{emptySum: null, emptyAvg: null, emptyCount: 0}], result);

assert.commandFailed(db.runCommand({
    aggregate: coll.getName(),
    pipeline: [{
        $setWindowFields: {
            sortBy: {day: 1},
            output: {bad: {$sum: "$qty", window: {range: [-1, 0], unit: "day"}}},
        },
    }],
    cursor: {},
}));
