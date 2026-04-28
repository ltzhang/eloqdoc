/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/pipeline/document_source_internal_unpack_bucket.h"

#include "mongo/db/pipeline/document_source_mock.h"
#include "mongo/db/pipeline/document_value_test_util.h"
#include "mongo/unittest/unittest.h"

namespace mongo {
namespace {

boost::intrusive_ptr<DocumentSource> makeUnpackStage(
    const boost::intrusive_ptr<ExpressionContext>& expCtx,
    const BSONObj& spec) {
    return DocumentSourceInternalUnpackBucket::createFromBson(spec.firstElement(), expCtx);
}

boost::intrusive_ptr<ExpressionContext> makeExpCtx() {
    auto expCtx = ObjectPool<ExpressionContext>::newObjectRawPointer(
        static_cast<OperationContext*>(nullptr), static_cast<const CollatorInterface*>(nullptr));
    expCtx->ns = NamespaceString("unittests.unpack_bucket");
    return expCtx;
}

TEST(InternalUnpackBucketTest, UnpacksMeasurementsInOrder) {
    auto expCtx = makeExpCtx();
    auto stage = makeUnpackStage(expCtx,
                                 BSON("$_internalUnpackBucket"
                                      << BSON("timeField"
                                              << "t"
                                              << "metaField"
                                              << "tags")));
    boost::intrusive_ptr<DocumentSourceMock> source =
        new DocumentSourceMock(std::deque<DocumentSource::GetNextResult>{Document{
        {"control", Document{{"version", 1}, {"count", 2}}},
        {"meta", Document{{"host", "a"_sd}}},
        {"data",
         Document{{"t", Document{{"0", Date_t::fromMillisSinceEpoch(1000)},
                                  {"1", Date_t::fromMillisSinceEpoch(2000)}}},
                  {"v", Document{{"0", 10}, {"1", 11}}}}}}}, expCtx);
    stage->setSource(source.get());

    ASSERT_DOCUMENT_EQ(
        stage->getNext().getDocument(),
        (Document{{"t", Date_t::fromMillisSinceEpoch(1000)},
                  {"v", 10},
                  {"tags", Document{{"host", "a"_sd}}}}));
    ASSERT_DOCUMENT_EQ(
        stage->getNext().getDocument(),
        (Document{{"t", Date_t::fromMillisSinceEpoch(2000)},
                  {"v", 11},
                  {"tags", Document{{"host", "a"_sd}}}}));
    ASSERT_TRUE(stage->getNext().isEOF());
}

TEST(InternalUnpackBucketTest, OmitsMetaWhenNoMetaFieldConfigured) {
    auto expCtx = makeExpCtx();
    auto stage = makeUnpackStage(expCtx,
                                 BSON("$_internalUnpackBucket" << BSON("timeField"
                                                                        << "t")));
    boost::intrusive_ptr<DocumentSourceMock> source =
        new DocumentSourceMock(std::deque<DocumentSource::GetNextResult>{Document{
        {"control", Document{{"version", 1}, {"count", 1}}},
        {"meta", Document{{"host", "a"_sd}}},
        {"data",
         Document{{"t", Document{{"0", Date_t::fromMillisSinceEpoch(1000)}}},
                  {"v", Document{{"0", 10}}}}}}}, expCtx);
    stage->setSource(source.get());

    ASSERT_DOCUMENT_EQ(stage->getNext().getDocument(),
                       (Document{{"t", Date_t::fromMillisSinceEpoch(1000)}, {"v", 10}}));
    ASSERT_TRUE(stage->getNext().isEOF());
}

TEST(InternalUnpackBucketTest, SparseDataDoesNotInventMissingValues) {
    auto expCtx = makeExpCtx();
    auto stage = makeUnpackStage(expCtx,
                                 BSON("$_internalUnpackBucket" << BSON("timeField"
                                                                        << "t")));
    boost::intrusive_ptr<DocumentSourceMock> source =
        new DocumentSourceMock(std::deque<DocumentSource::GetNextResult>{Document{
        {"control", Document{{"version", 1}, {"count", 2}}},
        {"data",
         Document{{"t", Document{{"0", Date_t::fromMillisSinceEpoch(1000)},
                                  {"1", Date_t::fromMillisSinceEpoch(2000)}}},
                  {"v", Document{{"0", 10}}}}}}}, expCtx);
    stage->setSource(source.get());

    ASSERT_DOCUMENT_EQ(stage->getNext().getDocument(),
                       (Document{{"t", Date_t::fromMillisSinceEpoch(1000)}, {"v", 10}}));
    ASSERT_DOCUMENT_EQ(stage->getNext().getDocument(),
                       (Document{{"t", Date_t::fromMillisSinceEpoch(2000)}}));
    ASSERT_TRUE(stage->getNext().isEOF());
}

}  // namespace
}  // namespace mongo
