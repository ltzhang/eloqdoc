/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/query_translator.h"

namespace mongo {
namespace timeseries {

std::vector<BSONObj> makeBucketPipeline(const CollectionOptions& options,
                                        const std::vector<BSONObj>& userPipeline) {
    invariant(options.timeseries);

    BSONObjBuilder unpackSpec;
    unpackSpec.append("timeField", options.timeseries->timeField);
    if (options.timeseries->hasMetaField()) {
        unpackSpec.append("metaField", options.timeseries->metaField);
    }

    std::vector<BSONObj> translated;
    translated.reserve(userPipeline.size() + 1);
    translated.push_back(BSON("$_internalUnpackBucket" << unpackSpec.obj()));
    for (const auto& stage : userPipeline) {
        translated.push_back(stage.getOwned());
    }
    return translated;
}

}  // namespace timeseries
}  // namespace mongo
