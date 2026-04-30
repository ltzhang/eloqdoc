/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include <functional>
#include <vector>

#include "mongo/base/status_with.h"
#include "mongo/db/timeseries/timeseries_options.h"
#include "mongo/db/jsobj.h"

namespace mongo {
namespace timeseries {

struct BucketDeleteResult {
    long long deleted = 0;
    std::vector<BSONObj> survivors;
    boost::optional<BSONObj> replacementBucket;
};

struct BucketUpdateResult {
    long long matched = 0;
    long long modified = 0;
    std::vector<BSONObj> measurements;
    boost::optional<BSONObj> replacementBucket;
};

StatusWith<std::vector<BSONObj>> unpackBucketMeasurements(const TimeseriesOptions& options,
                                                          const BSONObj& bucket);

StatusWith<BSONObj> makeBucketFromMeasurements(const TimeseriesOptions& options,
                                               const BSONObj& oldBucket,
                                               const std::vector<BSONObj>& measurements);

StatusWith<BucketDeleteResult> deleteMatchingMeasurementsFromBucket(
    const TimeseriesOptions& options,
    const BSONObj& bucket,
    const std::function<bool(const BSONObj&)>& shouldDelete,
    bool multi);

StatusWith<BucketUpdateResult> updateMatchingMeasurementsInBucket(
    const TimeseriesOptions& options,
    const BSONObj& bucket,
    const std::function<bool(const BSONObj&)>& shouldUpdate,
    const std::function<StatusWith<BSONObj>(const BSONObj&)>& updateMeasurement,
    bool multi);

}  // namespace timeseries
}  // namespace mongo
