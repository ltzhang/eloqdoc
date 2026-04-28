/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/timeseries_namespace.h"

namespace mongo {
namespace timeseries {

NamespaceString makeBucketNamespace(const NamespaceString& logicalNss) {
    return NamespaceString(logicalNss.db(), "system.buckets." + logicalNss.coll());
}

bool isBucketNamespace(const NamespaceString& nss) {
    return nss.coll().startsWith("system.buckets.");
}

}  // namespace timeseries
}  // namespace mongo
