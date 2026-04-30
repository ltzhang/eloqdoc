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
                                              << "a"
                                              << "control.min.v" << BSON("$lte" << 1)
                                              << "control.max.v" << BSON("$gte" << 1))),
        pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(BSON("$match" << BSON("t" << BSON("$gte" << start << "$lt" << end)
                                           << "tags.host"
                                           << "a"
                                           << "v" << 1)),
                      pipeline[2]);
    ASSERT_BSONOBJ_EQ(fromjson("{$sort: {v: 1}}"), pipeline[3]);
}

TEST(TimeSeriesQueryTranslator, AddsBucketMatchForMeasurementRangePredicates) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {BSON("$match" << BSON("v" << BSON("$gt" << 10 << "$lte" << 20)))});

    ASSERT_EQUALS(3U, pipeline.size());
    ASSERT_BSONOBJ_EQ(BSON("$match" << BSON("control.max.v" << BSON("$gt" << 10)
                                                            << "control.min.v"
                                                            << BSON("$lte" << 20))),
                      pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
}

TEST(TimeSeriesQueryTranslator, AddsBucketMatchesForConsecutiveLeadingMatches) {
    const auto start = Date_t::fromMillisSinceEpoch(1735689600000LL);

    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {BSON("$match" << BSON("t" << BSON("$gte" << start))),
         fromjson("{$match: {v: {$gt: 10}}}"),
         fromjson("{$project: {_id: 0, v: 1}}")});

    ASSERT_EQUALS(6U, pipeline.size());
    ASSERT_BSONOBJ_EQ(BSON("$match" << BSON("control.max.t" << BSON("$gte" << start))),
                      pipeline[0]);
    ASSERT_BSONOBJ_EQ(fromjson("{$match: {'control.max.v': {$gt: 10}}}"), pipeline[1]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[2].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(BSON("$match" << BSON("t" << BSON("$gte" << start))), pipeline[3]);
    ASSERT_BSONOBJ_EQ(fromjson("{$match: {v: {$gt: 10}}}"), pipeline[4]);
    ASSERT_BSONOBJ_EQ(fromjson("{$project: {_id: 0, v: 1}}"), pipeline[5]);
}

TEST(TimeSeriesQueryTranslator, PushesDownLeadingTimeAndMetaSortBeforeUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson("{$sort: {t: 1, 'tags.host': -1}}"), fromjson("{$limit: 5}")});

    ASSERT_EQUALS(5U, pipeline.size());
    ASSERT_BSONOBJ_EQ(fromjson("{$sort: {'control.min.t': 1, 'meta.host': -1}}"), pipeline[0]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[1]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[2].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$sort: {t: 1, 'tags.host': -1}}"), pipeline[3]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[4]);
}

TEST(TimeSeriesQueryTranslator, PushesDownDescendingTimeSortUsingBucketMaxBeforeUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(), {fromjson("{$sort: {t: -1}}"), fromjson("{$limit: 5}")});

    ASSERT_EQUALS(5U, pipeline.size());
    ASSERT_BSONOBJ_EQ(fromjson("{$sort: {'control.max.t': -1}}"), pipeline[0]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[1]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[2].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$sort: {t: -1}}"), pipeline[3]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[4]);
}

TEST(TimeSeriesQueryTranslator, DoesNotPushDownMeasurementSortBeforeUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson("{$sort: {v: 1}}"), fromjson("{$limit: 5}")});

    ASSERT_EQUALS(3U, pipeline.size());
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[0].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$sort: {v: 1}}"), pipeline[1]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[2]);
}

TEST(TimeSeriesQueryTranslator, PushesDownLeadingLimitBeforeUnpack) {
    const auto pipeline =
        makeBucketPipeline(makeOptions(), {fromjson("{$limit: 5}"), fromjson("{$project: {_id: 0}}")});

    ASSERT_EQUALS(4U, pipeline.size());
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[2]);
    ASSERT_BSONOBJ_EQ(fromjson("{$project: {_id: 0}}"), pipeline[3]);
}

TEST(TimeSeriesQueryTranslator, DoesNotPushDownLimitAfterBucketPredicate) {
    const auto start = Date_t::fromMillisSinceEpoch(1735689600000LL);
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {BSON("$match" << BSON("t" << BSON("$gte" << start))), fromjson("{$limit: 5}")});

    ASSERT_EQUALS(4U, pipeline.size());
    ASSERT_BSONOBJ_EQ(BSON("$match" << BSON("control.max.t" << BSON("$gte" << start))),
                      pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(BSON("$match" << BSON("t" << BSON("$gte" << start))), pipeline[2]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[3]);
}

TEST(TimeSeriesQueryTranslator, PushesDownLeadingMetaOnlyProjectBeforeUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson("{$project: {_id: 0, tags: 1}}"), fromjson("{$limit: 5}")});

    ASSERT_EQUALS(4U, pipeline.size());
    ASSERT_BSONOBJ_EQ(fromjson("{$project: {_id: 0, 'control.count': 1, meta: 1}}"), pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$project: {_id: 0, tags: 1}}"), pipeline[2]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[3]);
}

TEST(TimeSeriesQueryTranslator, PushesDownMetaAndMeasurementProjectBeforeUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson("{$project: {_id: 0, tags: 1, v: 1}}"), fromjson("{$limit: 5}")});

    ASSERT_EQUALS(4U, pipeline.size());
    ASSERT_BSONOBJ_EQ(fromjson("{$project: {_id: 0, 'control.count': 1, meta: 1, 'data.v': 1}}"),
                      pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$project: {_id: 0, tags: 1, v: 1}}"), pipeline[2]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[3]);
}

TEST(TimeSeriesQueryTranslator, PushesDownSimpleInclusionProjectBeforeUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson("{$project: {_id: 0, t: 1, tags: 1, v: 1}}"), fromjson("{$limit: 5}")});

    ASSERT_EQUALS(4U, pipeline.size());
    ASSERT_BSONOBJ_EQ(
        fromjson("{$project: {_id: 0, 'control.count': 1, 'data.t': 1, meta: 1, 'data.v': 1}}"),
        pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$project: {_id: 0, t: 1, tags: 1, v: 1}}"), pipeline[2]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[3]);
}

TEST(TimeSeriesQueryTranslator, DoesNotPushDownComputedProjectBeforeUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson("{$project: {_id: 0, doubled: {$add: ['$v', '$v']}}}"),
         fromjson("{$limit: 5}")});

    ASSERT_EQUALS(3U, pipeline.size());
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[0].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$project: {_id: 0, doubled: {$add: ['$v', '$v']}}}"),
                      pipeline[1]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[2]);
}

TEST(TimeSeriesQueryTranslator, PushesDownMetaOnlyAddFieldsBeforeUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson("{$addFields: {tags: {site: 'north'}}}"), fromjson("{$limit: 5}")});

    ASSERT_EQUALS(4U, pipeline.size());
    ASSERT_BSONOBJ_EQ(fromjson("{$addFields: {meta: {site: 'north'}}}"), pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$addFields: {tags: {site: 'north'}}}"), pipeline[2]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[3]);
}

TEST(TimeSeriesQueryTranslator, PushesDownMetaOnlySetBeforeUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(), {fromjson("{$set: {'tags.site': 'north'}}"), fromjson("{$limit: 5}")});

    ASSERT_EQUALS(4U, pipeline.size());
    ASSERT_BSONOBJ_EQ(fromjson("{$set: {'meta.site': 'north'}}"), pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$set: {'tags.site': 'north'}}"), pipeline[2]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[3]);
}

TEST(TimeSeriesQueryTranslator, DoesNotPushDownMeasurementAddFieldsBeforeUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(), {fromjson("{$addFields: {v: 7}}"), fromjson("{$limit: 5}")});

    ASSERT_EQUALS(3U, pipeline.size());
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[0].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$addFields: {v: 7}}"), pipeline[1]);
    ASSERT_BSONOBJ_EQ(fromjson("{$limit: 5}"), pipeline[2]);
}

TEST(TimeSeriesQueryTranslator, RewritesWholeCollectionGroupCountAndTimeBoundsWithoutUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson("{$group: {_id: null, n: {$sum: 1}, minT: {$min: '$t'}, maxT: {$max: '$t'}}}")});

    ASSERT_EQUALS(1U, pipeline.size());
    ASSERT_BSONOBJ_EQ(
        fromjson("{$group: {_id: null, n: {$sum: '$control.count'}, "
                 "minT: {$min: '$control.min.t'}, maxT: {$max: '$control.max.t'}}}"),
        pipeline[0]);
}

TEST(TimeSeriesQueryTranslator, RewritesWholeCollectionGroupMeasurementBoundsWithoutUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson("{$group: {_id: null, minV: {$min: '$v'}, maxV: {$max: '$v'}}}")});

    ASSERT_EQUALS(1U, pipeline.size());
    ASSERT_BSONOBJ_EQ(
        fromjson("{$group: {_id: null, "
                 "minV: {$min: '$control.min.v'}, maxV: {$max: '$control.max.v'}}}"),
        pipeline[0]);
}

TEST(TimeSeriesQueryTranslator, RewritesMetaGroupedCountAndMeasurementBoundsWithoutUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson(
            "{$group: {_id: '$tags.host', n: {$sum: 1}, minV: {$min: '$v'}, maxT: {$max: '$t'}}}")});

    ASSERT_EQUALS(1U, pipeline.size());
    ASSERT_BSONOBJ_EQ(fromjson("{$group: {_id: '$meta.host', n: {$sum: '$control.count'}, "
                               "minV: {$min: '$control.min.v'}, "
                               "maxT: {$max: '$control.max.t'}}}"),
                      pipeline[0]);
}

TEST(TimeSeriesQueryTranslator, DoesNotRewriteMeasurementGroupedTimeSeriesGroupBeforeUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson("{$group: {_id: '$v', n: {$sum: 1}}}")});

    ASSERT_EQUALS(2U, pipeline.size());
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[0].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$group: {_id: '$v', n: {$sum: 1}}}"), pipeline[1]);
}

TEST(TimeSeriesQueryTranslator, RewritesMetaFieldGroupBoundsWithoutUnpack) {
    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {fromjson("{$group: {_id: null, minHost: {$min: '$tags.host'}, "
                  "maxSite: {$max: '$tags.site'}}}")});

    ASSERT_EQUALS(1U, pipeline.size());
    ASSERT_BSONOBJ_EQ(fromjson("{$group: {_id: null, "
                               "minHost: {$min: '$meta.host'}, "
                               "maxSite: {$max: '$meta.site'}}}"),
                      pipeline[0]);
}

TEST(TimeSeriesQueryTranslator, AddsBucketMatchForMeasurementEqualityPredicate) {
    const auto pipeline =
        makeBucketPipeline(makeOptions(), {BSON("$match" << BSON("v" << BSON("$eq" << 7)))});

    ASSERT_EQUALS(3U, pipeline.size());
    ASSERT_BSONOBJ_EQ(BSON("$match" << BSON("control.min.v" << BSON("$lte" << 7)
                                                            << "control.max.v"
                                                            << BSON("$gte" << 7))),
                      pipeline[0]);
}

TEST(TimeSeriesQueryTranslator, KeepsUnpackFirstWhenNoBucketPredicateCanBeBuilt) {
    const auto pipeline =
        makeBucketPipeline(makeOptions(), {fromjson("{$match: {v: {$ne: 1}}}")});

    ASSERT_EQUALS(2U, pipeline.size());
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[0].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$match: {v: {$ne: 1}}}"), pipeline[1]);
}

TEST(TimeSeriesQueryTranslator, AddsBucketMatchForAndAndOrPredicates) {
    const auto start = Date_t::fromMillisSinceEpoch(1735689600000LL);
    const auto end = Date_t::fromMillisSinceEpoch(1735776000000LL);

    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {BSON("$match" << BSON("$and" << BSON_ARRAY(BSON("t" << BSON("$gte" << start))
                                                    << BSON("v" << 1))
                                  << "$or"
                                  << BSON_ARRAY(BSON("tags.host" << "a")
                                                << BSON("t" << BSON("$lt" << end)))) )});

    ASSERT_EQUALS(3U, pipeline.size());
    ASSERT_BSONOBJ_EQ(
        BSON("$match" << BSON("$and" << BSON_ARRAY(BSON("control.max.t" << BSON("$gte" << start))
                                                   << BSON("control.min.v" << BSON("$lte" << 1)
                                                                           << "control.max.v"
                                                                           << BSON("$gte" << 1)))
                             << "$or" << BSON_ARRAY(BSON("meta.host" << "a")
                                                    << BSON("control.min.t" << BSON("$lt" << end))))),
        pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
}

TEST(TimeSeriesQueryTranslator, DoesNotAddBucketOrWhenAnyBranchCannotBeTranslated) {
    const auto start = Date_t::fromMillisSinceEpoch(1735689600000LL);

    const auto pipeline = makeBucketPipeline(
        makeOptions(),
        {BSON("$match" << BSON("$or" << BSON_ARRAY(BSON("t" << BSON("$gte" << start))
                                                   << BSON("v" << BSON("$ne" << 1)))))});

    ASSERT_EQUALS(2U, pipeline.size());
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[0].firstElementFieldName());
}

TEST(TimeSeriesQueryTranslator, AddsBucketMatchForTimeInPredicate) {
    const auto first = Date_t::fromMillisSinceEpoch(1735689600000LL);
    const auto second = Date_t::fromMillisSinceEpoch(1735776000000LL);

    const auto pipeline =
        makeBucketPipeline(makeOptions(),
                           {BSON("$match" << BSON("t" << BSON("$in" << BSON_ARRAY(first
                                                                                   << second))))});

    ASSERT_EQUALS(3U, pipeline.size());
    ASSERT_BSONOBJ_EQ(
        BSON("$match" << BSON("$or" << BSON_ARRAY(BSON("control.min.t" << BSON("$lte" << first)
                                                     << "control.max.t" << BSON("$gte" << first))
                                                 << BSON("control.min.t" << BSON("$lte" << second)
                                                     << "control.max.t" << BSON("$gte" << second))))),
        pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
}

TEST(TimeSeriesQueryTranslator, AddsBucketMatchForMeasurementInPredicate) {
    const auto pipeline =
        makeBucketPipeline(makeOptions(), {fromjson("{$match: {v: {$in: [7, 11]}}}")});

    ASSERT_EQUALS(3U, pipeline.size());
    ASSERT_BSONOBJ_EQ(
        fromjson("{$match: {$or: ["
                 "{'control.min.v': {$lte: 7}, 'control.max.v': {$gte: 7}}, "
                 "{'control.min.v': {$lte: 11}, 'control.max.v': {$gte: 11}}]}}"),
        pipeline[0]);
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[1].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$match: {v: {$in: [7, 11]}}}"), pipeline[2]);
}

TEST(TimeSeriesQueryTranslator, DoesNotAddBucketMatchForMixedTypeMeasurementInPredicate) {
    const auto pipeline =
        makeBucketPipeline(makeOptions(), {fromjson("{$match: {v: {$in: [7, 'hot']}}}")});

    ASSERT_EQUALS(2U, pipeline.size());
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"), pipeline[0].firstElementFieldName());
    ASSERT_BSONOBJ_EQ(fromjson("{$match: {v: {$in: [7, 'hot']}}}"), pipeline[1]);
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
    ASSERT_EQUALS(std::string("$match"), bucketRequest.getPipeline()[0].firstElementFieldName());
    ASSERT_EQUALS(std::string("$_internalUnpackBucket"),
                  bucketRequest.getPipeline()[1].firstElementFieldName());
}

}  // namespace
}  // namespace timeseries
}  // namespace mongo
