/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/db/index/wildcard_access_method.h"

#include "mongo/db/catalog/index_catalog_entry.h"

namespace mongo {

WildcardAccessMethod::WildcardAccessMethod(IndexCatalogEntry* btreeState,
                                           SortedDataInterface* btree)
    : IndexAccessMethod(btreeState, btree),
      _keyGenerator(new WildcardKeyGenerator(
          _descriptor->keyPattern(),
          _descriptor->infoObj()
              .getObjectField(IndexDescriptor::kWildcardProjectionFieldName.toString()))) {
    uassert(6789100,
            "Currently only single-field wildcard indexes are supported.",
            _descriptor->getNumFields() == 1);
    uassert(6789101, "Wildcard indexes cannot guarantee uniqueness.", !_descriptor->unique());
}

void WildcardAccessMethod::doGetKeys(const BSONObj& obj,
                                     BSONObjSet* keys,
                                     MultikeyPaths* multikeyPaths) const {
    _keyGenerator->getKeys(obj, keys, multikeyPaths);
}

}  // namespace mongo
