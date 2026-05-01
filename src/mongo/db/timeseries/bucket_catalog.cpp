/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/bucket_catalog.h"

#include <algorithm>
#include <map>

#include "mongo/bson/bsonobjbuilder.h"

namespace mongo {
namespace timeseries {
namespace {

BSONObj canonicalizeMetaForKey(const BSONObj& meta) {
    std::map<std::string, BSONElement> fields;
    BSONForEach(elem, meta) {
        fields.emplace(elem.fieldName(), elem);
    }

    BSONObjBuilder bob;
    for (const auto& entry : fields) {
        bob.appendAs(entry.second, entry.first);
    }
    return bob.obj();
}

}  // namespace

BucketCatalog& BucketCatalog::get() {
    static BucketCatalog catalog;
    return catalog;
}

boost::optional<BucketHandle> BucketCatalog::findOpenBucket(const BucketKey& key,
                                                            long long measurementTimeMillis,
                                                            long long maxSpanMillis,
                                                            std::size_t maxCount) {
    std::lock_guard<std::mutex> lk(_mutex);
    auto keyString = makeKeyString(key);
    auto it = _openBuckets.find(keyString);
    if (it == _openBuckets.end()) {
        return boost::none;
    }

    for (const auto& handle : it->second) {
        if (handle.count >= maxCount) {
            continue;
        }
        if (measurementTimeMillis >= handle.minTimeMillis &&
            measurementTimeMillis < handle.minTimeMillis + maxSpanMillis) {
            return handle;
        }
    }

    return boost::none;
}

void BucketCatalog::upsertBucket(const BucketKey& key, const BucketHandle& handle) {
    std::lock_guard<std::mutex> lk(_mutex);
    auto keyString = makeKeyString(key);
    auto& handles = _openBuckets[keyString];
    for (auto& existing : handles) {
        if (existing.id == handle.id) {
            existing = handle;
            return;
        }
    }
    handles.push_back(handle);
}

void BucketCatalog::closeBucket(const BucketKey& key, const OID& id) {
    std::lock_guard<std::mutex> lk(_mutex);
    auto keyString = makeKeyString(key);
    auto it = _openBuckets.find(keyString);
    if (it == _openBuckets.end()) {
        return;
    }

    auto& handles = it->second;
    handles.erase(std::remove_if(handles.begin(),
                                 handles.end(),
                                 [&](const BucketHandle& handle) { return handle.id == id; }),
                  handles.end());
    if (handles.empty()) {
        _openBuckets.erase(it);
    }
}

void BucketCatalog::closeBucketById(const NamespaceString& logicalNss, const OID& id) {
    std::lock_guard<std::mutex> lk(_mutex);
    const auto nsPrefix = logicalNss.ns() + std::string(1, '\0');

    for (auto it = _openBuckets.begin(); it != _openBuckets.end();) {
        if (it->first.compare(0, nsPrefix.size(), nsPrefix) != 0) {
            ++it;
            continue;
        }

        auto& handles = it->second;
        handles.erase(std::remove_if(handles.begin(),
                                     handles.end(),
                                     [&](const BucketHandle& handle) { return handle.id == id; }),
                      handles.end());
        if (handles.empty()) {
            it = _openBuckets.erase(it);
        } else {
            ++it;
        }
    }
}

void BucketCatalog::closeBuckets(const NamespaceString& logicalNss) {
    std::lock_guard<std::mutex> lk(_mutex);
    const auto nsPrefix = logicalNss.ns() + std::string(1, '\0');

    for (auto it = _openBuckets.begin(); it != _openBuckets.end();) {
        if (it->first.compare(0, nsPrefix.size(), nsPrefix) == 0) {
            it = _openBuckets.erase(it);
        } else {
            ++it;
        }
    }
}

std::string BucketCatalog::makeKeyString(const BucketKey& key) const {
    StringBuilder builder;
    builder << key.logicalNss.ns() << '\0' << key.roundedMillis << '\0'
            << canonicalizeMetaForKey(key.meta).toString(false);
    return builder.str();
}

}  // namespace timeseries
}  // namespace mongo
