/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include <string>

#include "mongo/base/status.h"
#include "mongo/db/jsobj.h"

namespace mongo {
namespace timeseries {

struct TimeseriesOptions {
    std::string timeField;
    std::string metaField;
    std::string granularity = "seconds";

    bool hasMetaField() const {
        return !metaField.empty();
    }
};

Status parseTimeseriesOptions(const BSONElement& elem, TimeseriesOptions* out);
void appendTimeseriesOptions(BSONObjBuilder* builder, const TimeseriesOptions& options);

}  // namespace timeseries
}  // namespace mongo
