/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

#include "mongo/db/jsobj.h"
#include "mongo/db/namespace_string.h"

namespace mongo {
namespace timeseries {

struct BucketKey {
    NamespaceString logicalNss;
    BSONObj meta;
    long long roundedMillis = 0;
};

struct BucketHandle {
    OID id;
    std::size_t count = 0;
};

class BucketCatalog {
public:
    static BucketCatalog& get();

    BucketHandle getOrCreateBucket(const BucketKey& key);
    void recordInsert(const BucketKey& key);

private:
    std::string makeKeyString(const BucketKey& key) const;

    std::mutex _mutex;
    std::unordered_map<std::string, BucketHandle> _openBuckets;
};

}  // namespace timeseries
}  // namespace mongo
