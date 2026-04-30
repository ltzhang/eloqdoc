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
#include <vector>

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
    long long minTimeMillis = 0;
    long long roundedMillis = 0;
};

class BucketCatalog {
public:
    static BucketCatalog& get();

    boost::optional<BucketHandle> findOpenBucket(const BucketKey& key,
                                                 long long measurementTimeMillis,
                                                 long long maxSpanMillis,
                                                 std::size_t maxCount);
    void upsertBucket(const BucketKey& key, const BucketHandle& handle);
    void closeBucket(const BucketKey& key, const OID& id);
    void closeBucketById(const NamespaceString& logicalNss, const OID& id);

private:
    std::string makeKeyString(const BucketKey& key) const;

    std::mutex _mutex;
    std::unordered_map<std::string, std::vector<BucketHandle>> _openBuckets;
};

}  // namespace timeseries
}  // namespace mongo
