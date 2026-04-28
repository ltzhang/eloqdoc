/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/insert_router.h"

#include "mongo/base/error_codes.h"
#include "mongo/db/timeseries/bucket_catalog.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {
namespace timeseries {
namespace {

long long granularityMillis(const std::string& granularity) {
    if (granularity == "hours") {
        return 60LL * 60LL * 1000LL;
    }
    if (granularity == "minutes") {
        return 60LL * 1000LL;
    }
    return 1000LL;
}

BSONObj wrapMetaForKey(const BSONElement& meta) {
    if (meta.eoo()) {
        return BSONObj();
    }

    BSONObjBuilder builder;
    builder.appendAs(meta, "meta");
    return builder.obj();
}

StatusWith<BSONObj> makeBucketDocument(const NamespaceString& logicalNss,
                                       const CollectionOptions& options,
                                       const InsertStatement& measurement) {
    invariant(options.timeseries);

    const auto& tsOptions = *options.timeseries;
    auto timeElem = measurement.doc.getField(tsOptions.timeField);
    if (timeElem.eoo()) {
        return {ErrorCodes::BadValue,
                str::stream() << "time-series measurement is missing time field '"
                              << tsOptions.timeField << "'"};
    }
    if (timeElem.type() != mongo::Date) {
        return {ErrorCodes::TypeMismatch,
                str::stream() << "time-series measurement time field '" << tsOptions.timeField
                              << "' must be a Date"};
    }

    BSONElement metaElem;
    if (tsOptions.hasMetaField()) {
        metaElem = measurement.doc.getField(tsOptions.metaField);
    }

    const auto time = timeElem.Date();
    const long long span = granularityMillis(tsOptions.granularity);
    const long long roundedMillis = (time.toMillisSinceEpoch() / span) * span;

    BucketKey key{logicalNss, wrapMetaForKey(metaElem), roundedMillis};
    auto bucket = BucketCatalog::get().getOrCreateBucket(key);

    BSONObjBuilder builder;
    builder.appendOID("_id", &bucket.id);

    {
        BSONObjBuilder control(builder.subobjStart("control"));
        control.append("version", 1);
        {
            BSONObjBuilder min(control.subobjStart("min"));
            min.appendAs(timeElem, tsOptions.timeField);
            min.doneFast();
        }
        {
            BSONObjBuilder max(control.subobjStart("max"));
            max.appendAs(timeElem, tsOptions.timeField);
            max.doneFast();
        }
        control.append("count", 1);
        control.doneFast();
    }

    if (!metaElem.eoo()) {
        builder.appendAs(metaElem, "meta");
    }

    {
        BSONObjBuilder data(builder.subobjStart("data"));
        BSONForEach(elem, measurement.doc) {
            if (elem.fieldNameStringData() == "_id") {
                continue;
            }
            BSONObjBuilder column(data.subobjStart(elem.fieldName()));
            column.appendAs(elem, "0");
            column.doneFast();
        }
        data.doneFast();
    }

    BucketCatalog::get().recordInsert(key);
    return builder.obj();
}

}  // namespace

Status routeInsert(const NamespaceString& logicalNss,
                   const CollectionOptions& options,
                   const std::vector<InsertStatement>& input,
                   std::vector<InsertStatement>* bucketStatements) {
    bucketStatements->clear();
    bucketStatements->reserve(input.size());

    for (const auto& measurement : input) {
        auto bucket = makeBucketDocument(logicalNss, options, measurement);
        if (!bucket.isOK()) {
            return bucket.getStatus();
        }
        bucketStatements->emplace_back(measurement.stmtId, bucket.getValue());
    }

    return Status::OK();
}

}  // namespace timeseries
}  // namespace mongo
