/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/bucket_mutation.h"

#include <set>

#include "mongo/base/error_codes.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {
namespace timeseries {
namespace {

void appendForControl(BSONObjBuilder* builder, const BSONElement& elem) {
    switch (elem.type()) {
        case BSONType::Object:
        case BSONType::Array:
        case BSONType::BinData:
        case BSONType::Undefined:
        case BSONType::jstOID:
        case BSONType::Code:
        case BSONType::CodeWScope:
        case BSONType::DBRef:
        case BSONType::RegEx:
        case BSONType::Symbol:
            return;
        default:
            builder->appendAs(elem, elem.fieldName());
            return;
    }
}

BSONObj makeControlMinMax(const TimeseriesOptions& options,
                          const std::vector<BSONObj>& measurements,
                          bool isMin) {
    std::set<std::string> fieldNames;
    for (const auto& measurement : measurements) {
        BSONForEach(elem, measurement) {
            const auto fieldName = elem.fieldNameStringData();
            if (fieldName == "_id" || (options.hasMetaField() && fieldName == options.metaField)) {
                continue;
            }
            fieldNames.insert(elem.fieldName());
        }
    }

    BSONObjBuilder builder;
    for (const auto& fieldName : fieldNames) {
        BSONElement selected;
        for (const auto& measurement : measurements) {
            auto candidate = measurement.getField(fieldName);
            if (candidate.eoo()) {
                continue;
            }
            if (selected.eoo() ||
                (isMin && candidate.woCompare(selected, false) < 0) ||
                (!isMin && candidate.woCompare(selected, false) > 0)) {
                selected = candidate;
            }
        }
        if (!selected.eoo()) {
            appendForControl(&builder, selected);
        }
    }
    return builder.obj();
}

}  // namespace

StatusWith<std::vector<BSONObj>> unpackBucketMeasurements(const TimeseriesOptions& options,
                                                          const BSONObj& bucket) {
    auto control = bucket["control"];
    auto data = bucket["data"];
    if (control.type() != BSONType::Object) {
        return {ErrorCodes::FailedToParse, "time-series bucket is missing object control field"};
    }
    if (data.type() != BSONType::Object) {
        return {ErrorCodes::FailedToParse, "time-series bucket is missing object data field"};
    }

    auto countElem = control.Obj()["count"];
    if (!countElem.isNumber()) {
        return {ErrorCodes::FailedToParse, "time-series bucket control.count must be numeric"};
    }

    std::vector<BSONObj> measurements;
    const auto metaElem = bucket["meta"];
    const long long count = countElem.numberLong();
    measurements.reserve(static_cast<std::size_t>(count));
    for (long long i = 0; i < count; ++i) {
        const auto index = std::to_string(i);
        BSONObjBuilder measurement;
        BSONForEach(columnElem, data.Obj()) {
            if (columnElem.type() != BSONType::Object) {
                return {ErrorCodes::FailedToParse,
                        "time-series bucket data columns must be objects"};
            }
            auto value = columnElem.Obj()[index];
            if (!value.eoo()) {
                measurement.appendAs(value, columnElem.fieldName());
            }
        }

        if (options.hasMetaField() && !metaElem.eoo()) {
            measurement.appendAs(metaElem, options.metaField);
        }

        measurements.push_back(measurement.obj());
    }

    return measurements;
}

StatusWith<BSONObj> makeBucketFromMeasurements(const TimeseriesOptions& options,
                                               const BSONObj& oldBucket,
                                               const std::vector<BSONObj>& measurements) {
    if (measurements.empty()) {
        return {ErrorCodes::BadValue, "cannot build an empty time-series bucket"};
    }

    BSONObjBuilder builder;
    builder.appendAs(oldBucket["_id"], "_id");

    {
        BSONObjBuilder control(builder.subobjStart("control"));
        control.append("version", oldBucket.getObjectField("control").getIntField("version"));
        control.append("min", makeControlMinMax(options, measurements, true));
        control.append("max", makeControlMinMax(options, measurements, false));
        control.append("count", static_cast<long long>(measurements.size()));
        control.doneFast();
    }

    if (oldBucket.hasField("meta")) {
        builder.appendAs(oldBucket["meta"], "meta");
    }

    {
        BSONObjBuilder data(builder.subobjStart("data"));
        std::set<std::string> fieldNames;
        for (const auto& measurement : measurements) {
            BSONForEach(elem, measurement) {
                const auto fieldName = elem.fieldNameStringData();
                if (fieldName == "_id" || (options.hasMetaField() && fieldName == options.metaField)) {
                    continue;
                }
                fieldNames.insert(elem.fieldName());
            }
        }

        for (const auto& fieldName : fieldNames) {
            BSONObjBuilder column(data.subobjStart(fieldName));
            for (std::size_t i = 0; i < measurements.size(); ++i) {
                auto elem = measurements[i].getField(fieldName);
                if (!elem.eoo()) {
                    column.appendAs(elem, std::to_string(i));
                }
            }
            column.doneFast();
        }
        data.doneFast();
    }

    return builder.obj();
}

StatusWith<BucketDeleteResult> deleteMatchingMeasurementsFromBucket(
    const TimeseriesOptions& options,
    const BSONObj& bucket,
    const std::function<bool(const BSONObj&)>& shouldDelete,
    bool multi) {
    auto swMeasurements = unpackBucketMeasurements(options, bucket);
    if (!swMeasurements.isOK()) {
        return swMeasurements.getStatus();
    }

    BucketDeleteResult result;
    bool deletedOne = false;
    for (const auto& measurement : swMeasurements.getValue()) {
        if ((!deletedOne || multi) && shouldDelete(measurement)) {
            ++result.deleted;
            deletedOne = true;
            continue;
        }
        result.survivors.push_back(measurement.getOwned());
    }

    if (result.deleted == 0 || result.survivors.empty()) {
        return result;
    }

    auto swReplacement = makeBucketFromMeasurements(options, bucket, result.survivors);
    if (!swReplacement.isOK()) {
        return swReplacement.getStatus();
    }
    result.replacementBucket = swReplacement.getValue();
    return result;
}

}  // namespace timeseries
}  // namespace mongo
