/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include <deque>
#include <string>

#include "mongo/db/pipeline/document_source.h"

namespace mongo {

class DocumentSourceInternalUnpackBucket final : public DocumentSource {
public:
    static constexpr StringData kStageName = "$_internalUnpackBucket"_sd;

    static boost::intrusive_ptr<DocumentSource> createFromBson(
        BSONElement elem, const boost::intrusive_ptr<ExpressionContext>& expCtx);

    DocumentSourceInternalUnpackBucket(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                                       std::string timeField,
                                       std::string metaField);

    const char* getSourceName() const final {
        return kStageName.rawData();
    }

    StageConstraints constraints(Pipeline::SplitState pipeState) const final {
        return {StreamType::kStreaming,
                PositionRequirement::kNone,
                HostTypeRequirement::kNone,
                DiskUseRequirement::kNoDiskUse,
                FacetRequirement::kAllowed,
                TransactionRequirement::kAllowed};
    }

    GetNextResult getNext() final;

private:
    void unpackBucket(const Document& bucket);

    Value serialize(boost::optional<ExplainOptions::Verbosity> explain = boost::none) const final;

    std::deque<Document> _pending;
    std::string _timeField;
    std::string _metaField;
};

}  // namespace mongo
