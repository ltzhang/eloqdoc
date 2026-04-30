/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/insert_router_internal.h"

#include <string>

#include "mongo/unittest/unittest.h"

namespace mongo {
namespace timeseries {
namespace {

TimeseriesOptions makeOptions() {
    TimeseriesOptions options;
    options.timeField = "t";
    options.granularity = "seconds";
    return options;
}

BSONObj makeBucketWithPayload(const std::string& payload) {
    OID oid("000000000000000000000001");
    const auto t0 = Date_t::fromMillisSinceEpoch(1767225600000LL);

    return BSON("_id" << oid << "control"
                      << BSON("version" << 1 << "min" << BSON("t" << t0 << "payload" << payload)
                                        << "max" << BSON("t" << t0 << "payload" << payload)
                                        << "count" << 1LL)
                      << "data" << BSON("t" << BSON("0" << t0) << "payload"
                                            << BSON("0" << payload)));
}

TEST(TimeSeriesInsertRouter, RejectsReusableBucketWhenAppendWouldExceedBucketSizeLimit) {
    std::string payload(insert_router_detail::kMaxBucketDocumentSize / 4, 'x');
    auto bucket = makeBucketWithPayload(payload);
    const auto t1 = Date_t::fromMillisSinceEpoch(1767225601000LL);
    auto measurement = BSON("t" << t1 << "payload" << payload);

    ASSERT_FALSE(insert_router_detail::bucketAcceptsMeasurement(
        makeOptions(), bucket, measurement, t1.toMillisSinceEpoch(), 60LL * 60LL * 1000LL));
}

TEST(TimeSeriesInsertRouter, AcceptsReusableBucketBelowBucketSizeLimit) {
    auto bucket = makeBucketWithPayload("small");
    const auto t1 = Date_t::fromMillisSinceEpoch(1767225601000LL);
    auto measurement = BSON("t" << t1 << "payload" << "small");

    ASSERT_TRUE(insert_router_detail::bucketAcceptsMeasurement(
        makeOptions(), bucket, measurement, t1.toMillisSinceEpoch(), 60LL * 60LL * 1000LL));
}

}  // namespace
}  // namespace timeseries
}  // namespace mongo
