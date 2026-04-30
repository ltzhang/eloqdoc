/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include "mongo/db/catalog/collection_options.h"
#include "mongo/db/jsobj.h"
#include "mongo/db/namespace_string.h"

namespace mongo {
namespace timeseries {

BSONObj translateIndexSpecToBucketSchema(const NamespaceString& bucketNss,
                                         const CollectionOptions& options,
                                         const BSONObj& userSpec);

BSONObj translateIndexSpecFromBucketSchema(const NamespaceString& logicalNss,
                                           const CollectionOptions& options,
                                           const BSONObj& bucketSpec);

}  // namespace timeseries
}  // namespace mongo
