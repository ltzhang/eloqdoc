/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include <vector>

#include "mongo/base/status.h"
#include "mongo/db/catalog/collection_options.h"
#include "mongo/db/namespace_string.h"
#include "mongo/db/repl/oplog.h"

namespace mongo {
namespace timeseries {

Status routeInsert(const NamespaceString& logicalNss,
                   const CollectionOptions& options,
                   const std::vector<InsertStatement>& input,
                   std::vector<InsertStatement>* bucketStatements);

}  // namespace timeseries
}  // namespace mongo
