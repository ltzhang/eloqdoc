/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/timeseries_namespace.h"

#include "mongo/db/catalog/database.h"
#include "mongo/db/concurrency/write_conflict_exception.h"
#include "mongo/db/db_raii.h"
#include "mongo/db/storage/write_unit_of_work.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {
namespace timeseries {

NamespaceString makeBucketNamespace(const NamespaceString& logicalNss) {
    return NamespaceString(logicalNss.db(), "system.buckets." + logicalNss.coll());
}

bool isBucketNamespace(const NamespaceString& nss) {
    return nss.coll().startsWith("system.buckets.");
}

Status ensureBucketCollection(OperationContext* opCtx, const NamespaceString& logicalNss) {
    const auto bucketNss = makeBucketNamespace(logicalNss);
    return writeConflictRetry(opCtx, "ensureTimeSeriesBucketCollection", bucketNss.ns(), [&] {
        Lock::DBLock dbXLock(opCtx, logicalNss.db(), MODE_X);
        const bool shardVersionCheck = true;
        OldClientContext ctx(opCtx, logicalNss.ns(), shardVersionCheck);
        Database* db = ctx.db();
        if (!db) {
            return Status(ErrorCodes::NamespaceNotFound,
                          str::stream() << "database not found for " << logicalNss.ns());
        }

        if (db->getCollection(opCtx, bucketNss, true)) {
            return Status::OK();
        }

        CollectionOptions bucketOptions;
        bucketOptions.autoIndexId = CollectionOptions::NO;

        WriteUnitOfWork wuow(opCtx);
        Collection* bucketCollection =
            db->createCollection(opCtx, bucketNss.ns(), bucketOptions, false, BSONObj());
        if (!bucketCollection) {
            return Status(ErrorCodes::OperationFailed,
                          str::stream() << "failed to create bucket collection "
                                        << bucketNss.ns());
        }
        wuow.commit();
        return Status::OK();
    });
}

}  // namespace timeseries
}  // namespace mongo
