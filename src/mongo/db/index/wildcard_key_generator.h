/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include <string>

#include "mongo/bson/bsonobj_comparator_interface.h"
#include "mongo/db/index/multikey_paths.h"
#include "mongo/db/jsobj.h"

namespace mongo {

/**
 * Generates keys for a single-field wildcard index. Each leaf value is represented as a
 * two-component key: { "": <dotted path>, "": <leaf value> }.
 */
class WildcardKeyGenerator {
public:
    explicit WildcardKeyGenerator(const BSONObj& keyPattern);

    void getKeys(const BSONObj& obj, BSONObjSet* keys, MultikeyPaths* multikeyPaths) const;

private:
    void traverseObject(const BSONObj& obj,
                        const std::string& path,
                        BSONObjSet* keys,
                        MultikeyPaths* multikeyPaths) const;
    void traverseElement(const BSONElement& elem,
                         const std::string& path,
                         BSONObjSet* keys,
                         MultikeyPaths* multikeyPaths) const;
    void addKey(const std::string& path, const BSONElement& elem, BSONObjSet* keys) const;

    std::string _rootPath;
};

}  // namespace mongo
