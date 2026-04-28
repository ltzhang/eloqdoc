/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/timeseries_options.h"

#include "mongo/base/error_codes.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {
namespace timeseries {

Status parseTimeseriesOptions(const BSONElement& elem, TimeseriesOptions* out) {
    if (elem.type() != mongo::Object) {
        return {ErrorCodes::TypeMismatch, "'timeseries' has to be a document."};
    }

    TimeseriesOptions parsed;
    BSONForEach(field, elem.Obj()) {
        auto name = field.fieldNameStringData();
        if (name == "timeField") {
            if (field.type() != mongo::String) {
                return {ErrorCodes::TypeMismatch, "'timeseries.timeField' has to be a string."};
            }
            parsed.timeField = field.String();
        } else if (name == "metaField") {
            if (field.type() != mongo::String) {
                return {ErrorCodes::TypeMismatch, "'timeseries.metaField' has to be a string."};
            }
            parsed.metaField = field.String();
        } else if (name == "granularity") {
            if (field.type() != mongo::String) {
                return {ErrorCodes::TypeMismatch, "'timeseries.granularity' has to be a string."};
            }
            parsed.granularity = field.String();
        } else if (name == "bucketMaxSpanSeconds" || name == "bucketRoundingSeconds") {
            if (!field.isNumber()) {
                return {ErrorCodes::TypeMismatch,
                        str::stream() << "'timeseries." << name << "' has to be numeric."};
            }
        } else {
            return {ErrorCodes::InvalidOptions,
                    str::stream() << "'timeseries." << name << "' is not supported."};
        }
    }

    if (parsed.timeField.empty()) {
        return {ErrorCodes::InvalidOptions, "'timeseries.timeField' is required."};
    }

    if (parsed.granularity != "seconds" && parsed.granularity != "minutes" &&
        parsed.granularity != "hours") {
        return {ErrorCodes::InvalidOptions,
                "'timeseries.granularity' must be 'seconds', 'minutes', or 'hours'."};
    }

    *out = parsed;
    return Status::OK();
}

void appendTimeseriesOptions(BSONObjBuilder* builder, const TimeseriesOptions& options) {
    BSONObjBuilder ts(builder->subobjStart("timeseries"));
    ts.append("timeField", options.timeField);
    if (!options.metaField.empty()) {
        ts.append("metaField", options.metaField);
    }
    ts.append("granularity", options.granularity);
    ts.doneFast();
}

}  // namespace timeseries
}  // namespace mongo
