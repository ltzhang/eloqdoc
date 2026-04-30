/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/bucket_catalog.h"
#include "mongo/db/timeseries/bucket_mutation.h"

#include "mongo/db/json.h"
#include "mongo/unittest/unittest.h"

namespace mongo {
namespace timeseries {
namespace {

TimeseriesOptions makeOptions() {
    TimeseriesOptions options;
    options.timeField = "t";
    options.metaField = "tags";
    options.granularity = "minutes";
    return options;
}

BSONObj makeBucket() {
    OID oid("000000000000000000000001");
    const auto t0 = Date_t::fromMillisSinceEpoch(1767225600000LL);
    const auto t1 = Date_t::fromMillisSinceEpoch(1767225660000LL);
    const auto t2 = Date_t::fromMillisSinceEpoch(1767225720000LL);

    return BSON("_id" << oid << "control"
                      << BSON("version" << 1 << "min" << BSON("t" << t0 << "temp" << 10)
                                        << "max" << BSON("t" << t2 << "temp" << 30)
                                        << "count" << 3)
                      << "meta" << BSON("host" << "a")
                      << "data" << BSON("t" << BSON("0" << t0 << "1" << t1 << "2" << t2)
                                            << "temp" << BSON("0" << 10 << "1" << 20 << "2"
                                                                    << 30)));
}

TEST(TimeSeriesBucketMutation, UnpacksMeasurementsWithMetaField) {
    auto measurements = unittest::assertGet(unpackBucketMeasurements(makeOptions(), makeBucket()));

    ASSERT_EQUALS(3U, measurements.size());
    ASSERT_BSONOBJ_EQ(BSON("t" << Date_t::fromMillisSinceEpoch(1767225600000LL) << "temp" << 10
                               << "tags" << BSON("host" << "a")),
                      measurements[0]);
    ASSERT_BSONOBJ_EQ(BSON("t" << Date_t::fromMillisSinceEpoch(1767225720000LL) << "temp" << 30
                               << "tags" << BSON("host" << "a")),
                      measurements[2]);
}

TEST(TimeSeriesBucketMutation, DeletesMatchingMeasurementsAndRebuildsControl) {
    auto result = unittest::assertGet(deleteMatchingMeasurementsFromBucket(
        makeOptions(), makeBucket(), [](const BSONObj& measurement) {
            return measurement["temp"].numberInt() >= 20;
        }, true));

    ASSERT_EQUALS(2, result.deleted);
    ASSERT_EQUALS(1U, result.survivors.size());
    ASSERT_TRUE(result.replacementBucket);
    ASSERT_BSONOBJ_EQ(BSON("version" << 1 << "min"
                                      << BSON("t" << Date_t::fromMillisSinceEpoch(1767225600000LL)
                                                  << "temp" << 10)
                                      << "max"
                                      << BSON("t" << Date_t::fromMillisSinceEpoch(1767225600000LL)
                                                  << "temp" << 10)
                                      << "count" << 1LL),
                      result.replacementBucket->getObjectField("control"));
    ASSERT_BSONOBJ_EQ(BSON("t" << BSON("0" << Date_t::fromMillisSinceEpoch(1767225600000LL))
                               << "temp" << BSON("0" << 10)),
                      result.replacementBucket->getObjectField("data"));
}

TEST(TimeSeriesBucketMutation, SingleDeleteOnlyRemovesFirstMatchingMeasurement) {
    auto result = unittest::assertGet(deleteMatchingMeasurementsFromBucket(
        makeOptions(), makeBucket(), [](const BSONObj& measurement) {
            return measurement["temp"].numberInt() >= 20;
        }, false));

    ASSERT_EQUALS(1, result.deleted);
    ASSERT_EQUALS(2U, result.survivors.size());
    ASSERT_TRUE(result.replacementBucket);
    ASSERT_EQUALS(2, result.replacementBucket->getObjectField("control").getIntField("count"));
}

TEST(TimeSeriesBucketMutation, UpdatesMatchingMeasurementsAndRebuildsControl) {
    auto result = unittest::assertGet(updateMatchingMeasurementsInBucket(
        makeOptions(),
        makeBucket(),
        [](const BSONObj& measurement) { return measurement["temp"].numberInt() >= 20; },
        [](const BSONObj& measurement) -> StatusWith<BSONObj> {
            BSONObjBuilder builder;
            BSONForEach(elem, measurement) {
                if (elem.fieldNameStringData() == "temp") {
                    builder.append("temp", elem.numberInt() + 5);
                } else {
                    builder.append(elem);
                }
            }
            return builder.obj();
        },
        true));

    ASSERT_EQUALS(2, result.matched);
    ASSERT_EQUALS(2, result.modified);
    ASSERT_EQUALS(3U, result.measurements.size());
    ASSERT_TRUE(result.replacementBucket);
    ASSERT_BSONOBJ_EQ(BSON("version" << 1 << "min"
                                      << BSON("t" << Date_t::fromMillisSinceEpoch(1767225600000LL)
                                                  << "temp" << 10)
                                      << "max"
                                      << BSON("t" << Date_t::fromMillisSinceEpoch(1767225720000LL)
                                                  << "temp" << 35)
                                      << "count" << 3LL),
                      result.replacementBucket->getObjectField("control"));
    ASSERT_BSONOBJ_EQ(BSON("t" << BSON("0" << Date_t::fromMillisSinceEpoch(1767225600000LL) << "1"
                                            << Date_t::fromMillisSinceEpoch(1767225660000LL) << "2"
                                            << Date_t::fromMillisSinceEpoch(1767225720000LL))
                               << "temp" << BSON("0" << 10 << "1" << 25 << "2" << 35)),
                      result.replacementBucket->getObjectField("data"));
}

TEST(TimeSeriesBucketCatalog, CloseBucketByIdInvalidatesAnyKeyForNamespace) {
    const NamespaceString ns("test.metrics");
    const OID id("0000000000000000000000aa");
    BucketKey keyA{ns, BSON("meta" << BSON("host" << "a")), 1000};
    BucketKey keyB{ns, BSON("meta" << BSON("host" << "b")), 2000};
    BucketKey otherNs{NamespaceString("test.other"), BSON("meta" << BSON("host" << "a")), 1000};

    BucketCatalog::get().upsertBucket(keyA, BucketHandle{id, 3, 1000, 1000});
    BucketCatalog::get().upsertBucket(keyB, BucketHandle{id, 2, 2000, 2000});
    BucketCatalog::get().upsertBucket(otherNs, BucketHandle{id, 1, 1000, 1000});

    BucketCatalog::get().closeBucketById(ns, id);

    ASSERT_FALSE(BucketCatalog::get().findOpenBucket(keyA, 1000, 60000, 1000));
    ASSERT_FALSE(BucketCatalog::get().findOpenBucket(keyB, 2000, 60000, 1000));
    ASSERT_TRUE(BucketCatalog::get().findOpenBucket(otherNs, 1000, 60000, 1000));
    BucketCatalog::get().closeBucketById(otherNs.logicalNss, id);
}

}  // namespace
}  // namespace timeseries
}  // namespace mongo
