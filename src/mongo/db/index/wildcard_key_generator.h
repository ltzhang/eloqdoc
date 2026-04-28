/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include <string>
#include <vector>

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
    WildcardKeyGenerator(const BSONObj& keyPattern, const BSONObj& wildcardProjection);

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
    bool shouldDescendIntoPath(const std::string& path) const;
    bool shouldIndexLeafPath(const std::string& path) const;

    std::string _rootPath;
    bool _hasProjection = false;
    bool _includeProjection = false;
    std::vector<std::string> _projectionPaths;
};

}  // namespace mongo
