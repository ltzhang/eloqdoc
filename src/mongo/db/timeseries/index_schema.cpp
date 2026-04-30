/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/index_schema.h"

#include "mongo/db/index/index_descriptor.h"
#include "mongo/util/assert_util.h"

namespace mongo {
namespace timeseries {
namespace {

std::string translateKeyPathToBucketSchema(StringData path, const timeseries::TimeseriesOptions& tsOptions) {
    if (tsOptions.hasMetaField()) {
        const StringData metaField(tsOptions.metaField);
        if (path == metaField) {
            return "meta";
        }
        if (path.startsWith(metaField.toString() + ".")) {
            return "meta." + path.substr(metaField.size() + 1).toString();
        }
    }

    return "data." + path.toString();
}

std::string translateKeyPathFromBucketSchema(StringData path,
                                             const timeseries::TimeseriesOptions& tsOptions) {
    if (path == "meta"_sd) {
        invariant(tsOptions.hasMetaField());
        return tsOptions.metaField;
    }
    if (path.startsWith("meta."_sd)) {
        invariant(tsOptions.hasMetaField());
        return tsOptions.metaField + "." + path.substr(5).toString();
    }
    if (path.startsWith("data."_sd)) {
        return path.substr(5).toString();
    }

    return path.toString();
}

bool isUnsupportedGeospatialIndexType(const BSONElement& keyValue) {
    if (keyValue.type() != mongo::String) {
        return false;
    }

    const auto indexType = keyValue.valueStringData();
    return indexType == "2d"_sd || indexType == "2dsphere"_sd ||
        indexType == "geoHaystack"_sd || indexType == "2dsphere_bucket"_sd;
}

BSONObj translateKeyPatternToBucketSchema(const BSONObj& keyPattern,
                                          const timeseries::TimeseriesOptions& tsOptions) {
    BSONObjBuilder builder;
    for (auto&& elem : keyPattern) {
        uassert(ErrorCodes::CannotCreateIndex,
                str::stream() << "time-series geospatial indexes are not supported: " << elem,
                !isUnsupportedGeospatialIndexType(elem));
        builder.appendAs(elem, translateKeyPathToBucketSchema(elem.fieldNameStringData(), tsOptions));
    }
    return builder.obj();
}

BSONObj translateKeyPatternFromBucketSchema(const BSONObj& keyPattern,
                                            const timeseries::TimeseriesOptions& tsOptions) {
    BSONObjBuilder builder;
    for (auto&& elem : keyPattern) {
        builder.appendAs(elem,
                         translateKeyPathFromBucketSchema(elem.fieldNameStringData(), tsOptions));
    }
    return builder.obj();
}

BSONObj translateIndexSpec(const NamespaceString& nss,
                           const CollectionOptions& options,
                           const BSONObj& spec,
                           bool toBucketSchema) {
    invariant(options.timeseries);

    BSONObjBuilder builder;
    for (auto&& elem : spec) {
        const auto fieldName = elem.fieldNameStringData();
        if (fieldName == IndexDescriptor::kKeyPatternFieldName) {
            if (toBucketSchema) {
                builder.append(IndexDescriptor::kKeyPatternFieldName,
                               translateKeyPatternToBucketSchema(elem.Obj(), *options.timeseries));
            } else {
                builder.append(IndexDescriptor::kKeyPatternFieldName,
                               translateKeyPatternFromBucketSchema(elem.Obj(), *options.timeseries));
            }
        } else if (fieldName == IndexDescriptor::kNamespaceFieldName) {
            builder.append(IndexDescriptor::kNamespaceFieldName, nss.ns());
        } else if (toBucketSchema &&
                   fieldName == IndexDescriptor::kPartialFilterExprFieldName) {
            uasserted(ErrorCodes::CannotCreateIndex,
                      "time-series partial indexes are not supported");
        } else {
            builder.append(elem);
        }
    }

    return builder.obj();
}

}  // namespace

BSONObj translateIndexSpecToBucketSchema(const NamespaceString& bucketNss,
                                         const CollectionOptions& options,
                                         const BSONObj& userSpec) {
    return translateIndexSpec(bucketNss, options, userSpec, true);
}

BSONObj translateIndexSpecFromBucketSchema(const NamespaceString& logicalNss,
                                           const CollectionOptions& options,
                                           const BSONObj& bucketSpec) {
    return translateIndexSpec(logicalNss, options, bucketSpec, false);
}

}  // namespace timeseries
}  // namespace mongo
