/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/query_translator.h"

#include "mongo/db/catalog/collection_options.h"
#include "mongo/db/json.h"
#include "mongo/unittest/unittest.h"

namespace mongo {
namespace timeseries {
namespace {

CollectionOptions makeOptions() {
    CollectionOptions options;
    timeseries::TimeseriesOptions tsOptions;
    tsOptions.timeField = "t";
    tsOptions.metaField = "tags";
    tsOptions.granularity = "minutes";
    options.timeseries = tsOptions;
    return options;
}

TEST(TimeSeriesQueryTranslator, AddsBucketMatchForSimpleTimeAndMetaPredicates) {
    const auto start = Date_t::fromMillisSinceEpoch(1735689600000LL);
    const auto end = Date_t::fromMillisSinceEpoch(1735776000000LL);

    const auto pipeline =
        makeBucketPipeline(makeOptions(),
                           {BSON("$match" << BSON("t" << BSON("$gte" << start << "$lt" << end)
                                                      << "tags.host"
                                                      << "a"
                                                      << "v" << 1)),
                            fromjson("{$sort: {v: 1}}")});

    ASSERT_EQUALS(4U, pipeline.size());
    ASSERT_BSONOBJ_EQ(
        BSON("$match" << BSON("control.max.t" << BSON("$gte" << start) << "control.min.t"
                                              << BSON("$lt" << end) << "meta.host"
                                              << "a")),
        pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(BSON("$match" << BSON("t" << BSON("$gte" << start << "$lt" << end)
                                           << "tags.host"
                                           << "a"
                                           << "v" << 1)),
                      pipeline[2]);
    ASSERT_BSONOBJ_EQ(fromjson("{$sort: {v: 1}}"), pipeline[3]);
}

TEST(TimeSeriesQueryTranslator, KeepsUnpackFirstWhenNoBucketPredicateCanBeBuilt) {
    const auto pipeline = makeBucketPipeline(makeOptions(), {fromjson("{$match: {v: 1}}")});

    ASSERT_EQUALS(2U, pipeline.size());
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[0].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$match: {v: 1}}"), pipeline[1]);
}

}  // namespace
}  // namespace timeseries
}  // namespace mongo
