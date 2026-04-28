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

TEST(TimeSeriesQueryTranslator, BucketAggregationRequestPreservesOptions) {
    AggregationRequest request(NamespaceString("db.metrics"), {fromjson("{$match: {v: 1}}")});
    const auto collation = BSON("locale" << "simple");
    const auto hint = BSON("v" << 1);
    const auto letVariables = BSON("threshold" << 5);
    const auto runtimeConstants = BSON("localNow" << Date_t::fromMillisSinceEpoch(1));
    const auto readConcern = BSON("level" << "local");
    const auto readPref = BSON("mode" << "primary");

    request.setBatchSize(7);
    request.setCollation(collation);
    request.setHint(hint);
    request.setLet(letVariables);
    request.setRuntimeConstants(runtimeConstants);
    request.setComment("ts-read");
    request.setExplain(ExplainOptions::Verbosity::kQueryPlanner);
    request.setAllowDiskUse(true);
    request.setFromMongos(true);
    request.setNeedsMerge(true);
    request.setBypassDocumentValidation(true);
    request.setMaxTimeMS(123);
    request.setReadConcern(readConcern);
    request.setUnwrappedReadPref(readPref);

    const auto bucketRequest = makeBucketAggregationRequest(
        NamespaceString("db.system.buckets.metrics"), makeOptions(), request);

    ASSERT_EQUALS(NamespaceString("db.system.buckets.metrics"),
                  bucketRequest.getNamespaceString());
    ASSERT_EQUALS(7, bucketRequest.getBatchSize());
    ASSERT_BSONOBJ_EQ(collation, bucketRequest.getCollation());
    ASSERT_BSONOBJ_EQ(hint, bucketRequest.getHint());
    ASSERT_BSONOBJ_EQ(letVariables, bucketRequest.getLet());
    ASSERT_BSONOBJ_EQ(runtimeConstants, bucketRequest.getRuntimeConstants());
    ASSERT_EQUALS(std::string("ts-read"), bucketRequest.getComment());
    ASSERT_EQUALS(static_cast<int>(ExplainOptions::Verbosity::kQueryPlanner),
                  static_cast<int>(*bucketRequest.getExplain()));
    ASSERT_TRUE(bucketRequest.shouldAllowDiskUse());
    ASSERT_TRUE(bucketRequest.isFromMongos());
    ASSERT_TRUE(bucketRequest.needsMerge());
    ASSERT_TRUE(bucketRequest.shouldBypassDocumentValidation());
    ASSERT_EQUALS(123U, bucketRequest.getMaxTimeMS());
    ASSERT_BSONOBJ_EQ(readConcern, bucketRequest.getReadConcern());
    ASSERT_BSONOBJ_EQ(readPref, bucketRequest.getUnwrappedReadPref());
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"),
                  bucketRequest.getPipeline()[0].firstElementFieldName());
}

}  // namespace
}  // namespace timeseries
}  // namespace mongo
