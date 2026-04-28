/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/bucket_catalog.h"

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

BucketHandle BucketCatalog::getOrCreateBucket(const BucketKey& key) {
    std::lock_guard<std::mutex> lk(_mutex);
    auto keyString = makeKeyString(key);
    auto it = _openBuckets.find(keyString);
    if (it != _openBuckets.end()) {
        if (it->second.count > 0) {
            it->second.id = OID::gen();
            it->second.count = 0;
        }
        return it->second;
    }

    BucketHandle handle;
    handle.id = OID::gen();
    _openBuckets.emplace(keyString, handle);
    return handle;
}

void BucketCatalog::recordInsert(const BucketKey& key) {
    std::lock_guard<std::mutex> lk(_mutex);
    auto keyString = makeKeyString(key);
    auto it = _openBuckets.find(keyString);
    if (it != _openBuckets.end()) {
        ++it->second.count;
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
