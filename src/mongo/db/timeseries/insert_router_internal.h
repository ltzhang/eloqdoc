/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include <cstddef>

#include "mongo/db/catalog/collection_options.h"
#include "mongo/db/jsobj.h"

namespace mongo {
namespace timeseries {
namespace insert_router_detail {

constexpr std::size_t kMaxMeasurementsPerBucket = 1000;
constexpr int kMaxBucketDocumentSize = 12 * 1024 * 1024;

BSONObj makeReplacementBucketDocument(const TimeseriesOptions& options,
                                      const BSONObj& oldBucket,
                                      const BSONObj& measurement);

bool bucketAcceptsMeasurement(const TimeseriesOptions& options,
                              const BSONObj& bucketDoc,
                              const BSONObj& measurement,
                              long long measurementTimeMillis,
                              long long maxSpanMillis);

}  // namespace insert_router_detail
}  // namespace timeseries
}  // namespace mongo
