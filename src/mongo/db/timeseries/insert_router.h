/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include "mongo/base/status.h"
#include "mongo/db/catalog/collection_options.h"
#include "mongo/db/catalog/collection.h"
#include "mongo/db/namespace_string.h"
#include "mongo/db/operation_context.h"
#include "mongo/db/repl/oplog.h"

namespace mongo {
namespace timeseries {

Status routeInsert(OperationContext* opCtx,
                   const NamespaceString& logicalNss,
                   const CollectionOptions& options,
                   const std::vector<InsertStatement>& input,
                   Collection* bucketCollection);

}  // namespace timeseries
}  // namespace mongo
