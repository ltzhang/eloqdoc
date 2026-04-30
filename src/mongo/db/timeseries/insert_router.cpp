/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/insert_router.h"

#include <algorithm>

#include "mongo/base/error_codes.h"
#include "mongo/db/catalog/collection.h"
#include "mongo/db/curop.h"
#include "mongo/db/dbhelpers.h"
#include "mongo/db/op_observer.h"
#include "mongo/db/storage/snapshot.h"
#include "mongo/db/timeseries/bucket_catalog.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {
namespace timeseries {
namespace {

constexpr std::size_t kMaxMeasurementsPerBucket = 1000;

long long defaultBucketRoundingSeconds(const std::string& granularity) {
    if (granularity == "hours") {
        return 24LL * 60LL * 60LL;
    }
    if (granularity == "minutes") {
        return 60LL * 60LL;
    }
    return 60LL;
}

long long defaultBucketMaxSpanSeconds(const std::string& granularity) {
    if (granularity == "hours") {
        return 30LL * 24LL * 60LL * 60LL;
    }
    if (granularity == "minutes") {
        return 24LL * 60LL * 60LL;
    }
    return 60LL * 60LL;
}

long long bucketRoundingMillis(const TimeseriesOptions& options) {
    const auto seconds = options.bucketRoundingSeconds
        ? *options.bucketRoundingSeconds
        : defaultBucketRoundingSeconds(options.granularity);
    return seconds * 1000LL;
}

long long bucketMaxSpanMillis(const TimeseriesOptions& options) {
    const auto seconds = options.bucketMaxSpanSeconds
        ? *options.bucketMaxSpanSeconds
        : defaultBucketMaxSpanSeconds(options.granularity);
    return seconds * 1000LL;
}

BSONObj wrapMetaForKey(const BSONElement& meta) {
    if (meta.eoo()) {
        return BSONObj();
    }

    BSONObjBuilder builder;
    builder.appendAs(meta, "meta");
    return builder.obj();
}

StatusWith<BSONElement> extractTimeElement(const TimeseriesOptions& options,
                                           const BSONObj& measurement) {
    auto timeElem = measurement.getField(options.timeField);
    if (timeElem.eoo()) {
        return {ErrorCodes::BadValue,
                str::stream() << "time-series measurement is missing time field '"
                              << options.timeField << "'"};
    }
    if (timeElem.type() != mongo::Date) {
        return {ErrorCodes::TypeMismatch,
                str::stream() << "time-series measurement time field '" << options.timeField
                              << "' must be a Date"};
    }
    return timeElem;
}

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

int compareForMinMax(const BSONElement& lhs, const BSONElement& rhs) {
    return lhs.woCompare(rhs, false);
}

bool shouldStoreAsBucketColumn(const TimeseriesOptions& options, StringData fieldName) {
    return fieldName != "_id" && (!options.hasMetaField() || fieldName != options.metaField);
}

BSONObj makeNewBucketDocument(const TimeseriesOptions& options,
                              const OID& bucketId,
                              const BSONObj& measurement,
                              const BSONElement& metaElem) {
    BSONObjBuilder builder;
    builder.appendOID("_id", const_cast<OID*>(&bucketId));

    {
        BSONObjBuilder control(builder.subobjStart("control"));
        control.append("version", 1);
        {
            BSONObjBuilder min(control.subobjStart("min"));
            BSONForEach(elem, measurement) {
                if (!shouldStoreAsBucketColumn(options, elem.fieldNameStringData())) {
                    continue;
                }
                appendForControl(&min, elem);
            }
            min.doneFast();
        }
        {
            BSONObjBuilder max(control.subobjStart("max"));
            BSONForEach(elem, measurement) {
                if (!shouldStoreAsBucketColumn(options, elem.fieldNameStringData())) {
                    continue;
                }
                appendForControl(&max, elem);
            }
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
        BSONForEach(elem, measurement) {
            if (!shouldStoreAsBucketColumn(options, elem.fieldNameStringData())) {
                continue;
            }
            BSONObjBuilder column(data.subobjStart(elem.fieldName()));
            column.appendAs(elem, "0");
            column.doneFast();
        }
        data.doneFast();
    }

    return builder.obj();
}

BSONObj makeReplacementBucketDocument(const TimeseriesOptions& options,
                                      const BSONObj& oldBucket,
                                      const BSONObj& measurement) {
    const auto oldControl = oldBucket.getObjectField("control");
    const auto oldMin = oldControl.getObjectField("min");
    const auto oldMax = oldControl.getObjectField("max");
    const auto oldData = oldBucket.getObjectField("data");
    const auto newCount = static_cast<long long>(oldControl.getField("count").numberLong() + 1);
    const auto indexString = std::to_string(newCount - 1);

    BSONObjBuilder builder;
    builder.appendElements(oldBucket["_id"].wrap());

    {
        BSONObjBuilder control(builder.subobjStart("control"));
        control.append("version", oldControl.getIntField("version"));
        {
            BSONObjBuilder min(control.subobjStart("min"));
            BSONForEach(oldElem, oldMin) {
                auto incoming = measurement.getField(oldElem.fieldName());
                if (!incoming.eoo() && compareForMinMax(incoming, oldElem) < 0) {
                    appendForControl(&min, incoming);
                } else {
                    min.appendAs(oldElem, oldElem.fieldName());
                }
            }
            BSONForEach(incoming, measurement) {
                if (!shouldStoreAsBucketColumn(options, incoming.fieldNameStringData()) ||
                    oldMin.hasField(incoming.fieldName())) {
                    continue;
                }
                appendForControl(&min, incoming);
            }
            min.doneFast();
        }
        {
            BSONObjBuilder max(control.subobjStart("max"));
            BSONForEach(oldElem, oldMax) {
                auto incoming = measurement.getField(oldElem.fieldName());
                if (!incoming.eoo() && compareForMinMax(incoming, oldElem) > 0) {
                    appendForControl(&max, incoming);
                } else {
                    max.appendAs(oldElem, oldElem.fieldName());
                }
            }
            BSONForEach(incoming, measurement) {
                if (!shouldStoreAsBucketColumn(options, incoming.fieldNameStringData()) ||
                    oldMax.hasField(incoming.fieldName())) {
                    continue;
                }
                appendForControl(&max, incoming);
            }
            max.doneFast();
        }
        control.append("count", newCount);
        control.doneFast();
    }

    if (oldBucket.hasField("meta")) {
        builder.appendAs(oldBucket["meta"], "meta");
    }

    {
        BSONObjBuilder data(builder.subobjStart("data"));
        BSONForEach(oldColumn, oldData) {
            BSONObjBuilder column(data.subobjStart(oldColumn.fieldName()));
            if (oldColumn.type() == mongo::Object) {
                BSONForEach(existing, oldColumn.Obj()) {
                    column.appendAs(existing, existing.fieldName());
                }
            }
            const auto incoming = measurement.getField(oldColumn.fieldName());
            if (!incoming.eoo()) {
                column.appendAs(incoming, indexString);
            }
            column.doneFast();
        }
        BSONForEach(incoming, measurement) {
            if (!shouldStoreAsBucketColumn(options, incoming.fieldNameStringData()) ||
                oldData.hasField(incoming.fieldName())) {
                continue;
            }
            BSONObjBuilder column(data.subobjStart(incoming.fieldName()));
            column.appendAs(incoming, indexString);
            column.doneFast();
        }
        data.doneFast();
    }

    return builder.obj();
}

bool bucketAcceptsMeasurement(const TimeseriesOptions& options,
                              const BSONObj& bucketDoc,
                              long long measurementTimeMillis,
                              long long maxSpanMillis) {
    const auto control = bucketDoc.getObjectField("control");
    const auto countElem = control.getField("count");
    if (!countElem.isNumber() || countElem.numberLong() >= static_cast<long long>(kMaxMeasurementsPerBucket)) {
        return false;
    }

    const auto minTimeElem = control.getObjectField("min").getField(options.timeField);
    if (minTimeElem.type() != mongo::Date) {
        return false;
    }

    const auto minTimeMillis = minTimeElem.Date().toMillisSinceEpoch();
    return measurementTimeMillis >= minTimeMillis &&
        measurementTimeMillis < minTimeMillis + maxSpanMillis;
}

StatusWith<RecordId> findReusableBucketRecord(OperationContext* opCtx,
                                              Collection* bucketCollection,
                                              const TimeseriesOptions& options,
                                              const BucketKey& key,
                                              long long measurementTimeMillis,
                                              long long maxSpanMillis,
                                              BSONObj* bucketDocOut,
                                              BucketHandle* handleOut) {
    auto cached = BucketCatalog::get().findOpenBucket(
        key, measurementTimeMillis, maxSpanMillis, kMaxMeasurementsPerBucket);
    if (cached) {
        BSONObj cachedDoc;
        if (Helpers::findOne(opCtx, bucketCollection, BSON("_id" << cached->id), cachedDoc)) {
            if (bucketAcceptsMeasurement(options, cachedDoc, measurementTimeMillis, maxSpanMillis)) {
                *bucketDocOut = cachedDoc.getOwned();
                *handleOut = *cached;
                return Helpers::findOne(opCtx, bucketCollection, BSON("_id" << cached->id), false);
            }
        }
        BucketCatalog::get().closeBucket(key, cached->id);
    }

    auto cursor = bucketCollection->getCursor(opCtx);
    while (auto record = cursor->next()) {
        auto bucketDoc = record->data.releaseToBson();
        if (bucketDoc.hasField("meta") != !key.meta.isEmpty()) {
            continue;
        }
        if (!key.meta.isEmpty() &&
            bucketDoc.getField("meta").woCompare(key.meta.getField("meta"), false) != 0) {
            continue;
        }
        if (!bucketAcceptsMeasurement(options, bucketDoc, measurementTimeMillis, maxSpanMillis)) {
            continue;
        }

        const auto count = static_cast<std::size_t>(
            bucketDoc.getObjectField("control").getField("count").numberLong());
        const auto minTimeMillis =
            bucketDoc.getObjectField("control").getObjectField("min").getField(options.timeField)
                .Date()
                .toMillisSinceEpoch();
        *bucketDocOut = bucketDoc.getOwned();
        *handleOut = BucketHandle{bucketDoc["_id"].OID(), count, minTimeMillis, key.roundedMillis};
        BucketCatalog::get().upsertBucket(key, *handleOut);
        return record->id;
    }

    return RecordId();
}

Status insertNewBucket(OperationContext* opCtx,
                       Collection* bucketCollection,
                       const BucketKey& key,
                       const TimeseriesOptions& options,
                       const BSONObj& measurement,
                       const BSONElement& metaElem,
                       long long measurementTimeMillis) {
    BucketHandle handle{OID::gen(), 1U, measurementTimeMillis, key.roundedMillis};
    const auto bucketDoc = makeNewBucketDocument(options, handle.id, measurement, metaElem);
    auto status = bucketCollection->insertDocument(
        opCtx, InsertStatement(bucketDoc), &CurOp::get(opCtx)->debug(), true, false);
    if (!status.isOK()) {
        return status;
    }
    BucketCatalog::get().upsertBucket(key, handle);
    return Status::OK();
}

Status updateBucket(OperationContext* opCtx,
                    Collection* bucketCollection,
                    const BucketKey& key,
                    const TimeseriesOptions& options,
                    const BSONObj& oldBucket,
                    const RecordId& recordId,
                    const BSONObj& measurement) {
    const auto replacement = makeReplacementBucketDocument(options, oldBucket, measurement);
    OplogUpdateEntryArgs args;
    bucketCollection->updateDocument(opCtx,
                                     recordId,
                                     Snapshotted<BSONObj>(opCtx->recoveryUnit()->getSnapshotId(),
                                                          oldBucket),
                                     replacement,
                                     true,
                                     true,
                                     &CurOp::get(opCtx)->debug(),
                                     &args);

    const auto newCount = static_cast<std::size_t>(
        replacement.getObjectField("control").getField("count").numberLong());
    const auto minTimeMillis =
        replacement.getObjectField("control").getObjectField("min").getField(options.timeField)
            .Date()
            .toMillisSinceEpoch();
    BucketHandle handle{replacement["_id"].OID(), newCount, minTimeMillis, key.roundedMillis};
    if (newCount >= kMaxMeasurementsPerBucket) {
        BucketCatalog::get().closeBucket(key, handle.id);
    } else {
        BucketCatalog::get().upsertBucket(key, handle);
    }
    return Status::OK();
}

}  // namespace

Status routeInsert(OperationContext* opCtx,
                   const NamespaceString& logicalNss,
                   const CollectionOptions& options,
                   const std::vector<InsertStatement>& input,
                   Collection* bucketCollection) {
    invariant(options.timeseries);

    const auto& tsOptions = *options.timeseries;
    const auto roundingMillis = bucketRoundingMillis(tsOptions);
    const auto maxSpanMillis = bucketMaxSpanMillis(tsOptions);

    for (const auto& measurement : input) {
        auto swTimeElem = extractTimeElement(tsOptions, measurement.doc);
        if (!swTimeElem.isOK()) {
            return swTimeElem.getStatus();
        }
        auto timeElem = swTimeElem.getValue();
        BSONElement metaElem;
        if (tsOptions.hasMetaField()) {
            metaElem = measurement.doc.getField(tsOptions.metaField);
        }

        const auto measurementTimeMillis = timeElem.Date().toMillisSinceEpoch();
        const auto roundedMillis = (measurementTimeMillis / roundingMillis) * roundingMillis;
        BucketKey key{logicalNss, wrapMetaForKey(metaElem), roundedMillis};

        BSONObj existingBucket;
        BucketHandle handle;
        auto swRecordId = findReusableBucketRecord(opCtx,
                                                   bucketCollection,
                                                   tsOptions,
                                                   key,
                                                   measurementTimeMillis,
                                                   maxSpanMillis,
                                                   &existingBucket,
                                                   &handle);
        if (!swRecordId.isOK()) {
            return swRecordId.getStatus();
        }

        if (!swRecordId.getValue().isNull()) {
            auto status = updateBucket(opCtx,
                                       bucketCollection,
                                       key,
                                       tsOptions,
                                       existingBucket,
                                       swRecordId.getValue(),
                                       measurement.doc);
            if (!status.isOK()) {
                return status;
            }
            continue;
        }

        auto status = insertNewBucket(opCtx,
                                      bucketCollection,
                                      key,
                                      tsOptions,
                                      measurement.doc,
                                      metaElem,
                                      measurementTimeMillis);
        if (!status.isOK()) {
            return status;
        }
    }

    return Status::OK();
}

}  // namespace timeseries
}  // namespace mongo
