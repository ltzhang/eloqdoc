/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/index_schema.h"

#include "mongo/db/json.h"
#include "mongo/unittest/unittest.h"

namespace mongo {
namespace timeseries {
namespace {

CollectionOptions makeOptions() {
    CollectionOptions options;
    timeseries::TimeseriesOptions tsOptions;
    tsOptions.timeField = "t";
    tsOptions.metaField = "tags";
    tsOptions.granularity = "minutes";
    options.timeseries = tsOptions;
    return options;
}

TEST(TimeSeriesIndexSchema, TranslatesLogicalSpecToBucketSchema) {
    const auto translated = translateIndexSpecToBucketSchema(
        NamespaceString("db.system.buckets.metrics"),
        makeOptions(),
        fromjson("{ns: 'db.metrics', key: {'tags.host': 1, t: -1, temp: 1}, name: 'idx'}"));

    ASSERT_BSONOBJ_EQ(
        fromjson("{ns: 'db.system.buckets.metrics', "
                 "key: {'meta.host': 1, 'data.t': -1, 'data.temp': 1}, name: 'idx'}"),
        translated);
}

TEST(TimeSeriesIndexSchema, TranslatesBucketSpecToLogicalSchema) {
    const auto translated = translateIndexSpecFromBucketSchema(
        NamespaceString("db.metrics"),
        makeOptions(),
        fromjson("{ns: 'db.system.buckets.metrics', "
                 "key: {'meta.host': 1, 'data.t': -1, 'data.temp': 1}, name: 'idx'}"));

    ASSERT_BSONOBJ_EQ(
        fromjson("{ns: 'db.metrics', key: {'tags.host': 1, t: -1, temp: 1}, name: 'idx'}"),
        translated);
}

}  // namespace
}  // namespace timeseries
}  // namespace mongo
