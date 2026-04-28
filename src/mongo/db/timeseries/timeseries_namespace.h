/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include "mongo/db/namespace_string.h"

namespace mongo {
namespace timeseries {

NamespaceString makeBucketNamespace(const NamespaceString& logicalNss);
bool isBucketNamespace(const NamespaceString& nss);

}  // namespace timeseries
}  // namespace mongo
