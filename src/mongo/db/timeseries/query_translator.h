/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include <vector>

#include "mongo/db/catalog/collection_options.h"
#include "mongo/db/jsobj.h"
#include "mongo/db/namespace_string.h"
#include "mongo/db/pipeline/aggregation_request.h"

namespace mongo {
namespace timeseries {

std::vector<BSONObj> makeBucketPipeline(const CollectionOptions& options,
                                        const std::vector<BSONObj>& userPipeline);
AggregationRequest makeBucketAggregationRequest(const NamespaceString& bucketNss,
                                                const CollectionOptions& options,
                                                const AggregationRequest& request);

}  // namespace timeseries
}  // namespace mongo
