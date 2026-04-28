/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include <memory>

#include "mongo/db/index/index_access_method.h"
#include "mongo/db/index/wildcard_key_generator.h"

namespace mongo {

class WildcardAccessMethod final : public IndexAccessMethod {
public:
    WildcardAccessMethod(IndexCatalogEntry* btreeState, SortedDataInterface* btree);

private:
    void doGetKeys(const BSONObj& obj, BSONObjSet* keys, MultikeyPaths* multikeyPaths) const final;

    std::unique_ptr<WildcardKeyGenerator> _keyGenerator;
};

}  // namespace mongo
