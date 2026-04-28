/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/db/index/wildcard_key_generator.h"

#include "mongo/db/bson/dotted_path_support.h"
#include "mongo/util/assert_util.h"

namespace mongo {

namespace {
constexpr StringData kWildcardSuffix = ".$**"_sd;

std::string joinPath(const std::string& prefix, StringData fieldName) {
    if (prefix.empty()) {
        return fieldName.toString();
    }
    return prefix + "." + fieldName.toString();
}

bool pathIsOrHasPrefix(StringData path, StringData prefix) {
    return path == prefix || (path.size() > prefix.size() && path.startsWith(prefix) &&
                              path[prefix.size()] == '.');
}

bool pathCanContainProjection(StringData path, StringData projectionPath) {
    return pathIsOrHasPrefix(path, projectionPath) || pathIsOrHasPrefix(projectionPath, path);
}
}  // namespace

WildcardKeyGenerator::WildcardKeyGenerator(const BSONObj& keyPattern,
                                           const BSONObj& wildcardProjection) {
    invariant(keyPattern.nFields() == 1);

    const BSONElement elem = keyPattern.firstElement();
    StringData fieldName = elem.fieldNameStringData();
    if (fieldName == "$**"_sd) {
        _rootPath.clear();
    } else {
        invariant(fieldName.endsWith(kWildcardSuffix));
        _rootPath = fieldName.substr(0, fieldName.size() - kWildcardSuffix.size()).toString();
    }

    if (!wildcardProjection.isEmpty()) {
        _hasProjection = true;

        bool sawInclusion = false;
        bool sawExclusion = false;
        for (const BSONElement& projectionElem : wildcardProjection) {
            const bool include = projectionElem.trueValue();
            sawInclusion = sawInclusion || include;
            sawExclusion = sawExclusion || !include;
            _projectionPaths.push_back(projectionElem.fieldName());
        }

        _includeProjection = sawInclusion && !sawExclusion;
    }
}

void WildcardKeyGenerator::getKeys(const BSONObj& obj,
                                   BSONObjSet* keys,
                                   MultikeyPaths* multikeyPaths) const {
    if (multikeyPaths) {
        invariant(multikeyPaths->empty());
        multikeyPaths->resize(1);
    }

    if (_rootPath.empty()) {
        traverseObject(obj, "", keys, multikeyPaths);
        return;
    }

    BSONElement root = dotted_path_support::extractElementAtPath(obj, _rootPath);
    if (!root.eoo()) {
        traverseElement(root, _rootPath, keys, multikeyPaths);
    }
}

void WildcardKeyGenerator::traverseObject(const BSONObj& obj,
                                          const std::string& path,
                                          BSONObjSet* keys,
                                          MultikeyPaths* multikeyPaths) const {
    for (const BSONElement& elem : obj) {
        std::string childPath = joinPath(path, elem.fieldNameStringData());
        if (childPath == "_id") {
            continue;
        }
        if (!shouldDescendIntoPath(childPath)) {
            continue;
        }
        traverseElement(elem, childPath, keys, multikeyPaths);
    }
}

void WildcardKeyGenerator::traverseElement(const BSONElement& elem,
                                           const std::string& path,
                                           BSONObjSet* keys,
                                           MultikeyPaths* multikeyPaths) const {
    if (elem.type() == BSONType::Object) {
        traverseObject(elem.Obj(), path, keys, multikeyPaths);
        return;
    }

    if (elem.type() == BSONType::Array) {
        if (multikeyPaths) {
            (*multikeyPaths)[0].insert(0);
        }
        for (const BSONElement& arrayElem : elem.Obj()) {
            if (arrayElem.type() == BSONType::Object) {
                traverseObject(arrayElem.Obj(), path, keys, multikeyPaths);
            } else if (arrayElem.type() == BSONType::Array) {
                traverseElement(arrayElem, path, keys, multikeyPaths);
            } else {
                addKey(path, arrayElem, keys);
            }
        }
        return;
    }

    addKey(path, elem, keys);
}

void WildcardKeyGenerator::addKey(const std::string& path,
                                  const BSONElement& elem,
                                  BSONObjSet* keys) const {
    if (!shouldIndexLeafPath(path)) {
        return;
    }

    BSONObjBuilder builder;
    builder.append("", path);
    builder.appendAs(elem, "");
    keys->insert(builder.obj());
}

bool WildcardKeyGenerator::shouldDescendIntoPath(const std::string& path) const {
    if (!_hasProjection) {
        return true;
    }

    StringData pathData(path);
    if (_includeProjection) {
        for (const auto& projectionPath : _projectionPaths) {
            if (pathCanContainProjection(pathData, projectionPath)) {
                return true;
            }
        }
        return false;
    }

    for (const auto& projectionPath : _projectionPaths) {
        if (pathIsOrHasPrefix(pathData, projectionPath)) {
            return false;
        }
    }
    return true;
}

bool WildcardKeyGenerator::shouldIndexLeafPath(const std::string& path) const {
    if (!_hasProjection) {
        return true;
    }

    StringData pathData(path);
    if (_includeProjection) {
        for (const auto& projectionPath : _projectionPaths) {
            if (pathIsOrHasPrefix(pathData, projectionPath)) {
                return true;
            }
        }
        return false;
    }

    for (const auto& projectionPath : _projectionPaths) {
        if (pathIsOrHasPrefix(pathData, projectionPath)) {
            return false;
        }
    }
    return true;
}

}  // namespace mongo
