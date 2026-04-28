/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#pragma once

#include <vector>

#include "mongo/db/pipeline/document_source.h"
#include "mongo/db/pipeline/lite_parsed_pipeline.h"
#include "mongo/db/pipeline/pipeline.h"

namespace mongo {

class DocumentSourceUnionWith final : public DocumentSource {
public:
    class LiteParsed final : public LiteParsedDocumentSource {
    public:
        static std::unique_ptr<LiteParsed> parse(const AggregationRequest& request,
                                                 const BSONElement& spec);

        LiteParsed(NamespaceString unionNss,
                   stdx::unordered_set<NamespaceString> involvedNamespaces,
                   boost::optional<LiteParsedPipeline> liteParsedPipeline)
            : _unionNss(std::move(unionNss)),
              _involvedNamespaces(std::move(involvedNamespaces)),
              _liteParsedPipeline(std::move(liteParsedPipeline)) {}

        stdx::unordered_set<NamespaceString> getInvolvedNamespaces() const final {
            return _involvedNamespaces;
        }

        PrivilegeVector requiredPrivileges(bool isMongos) const final;

    private:
        NamespaceString _unionNss;
        stdx::unordered_set<NamespaceString> _involvedNamespaces;
        boost::optional<LiteParsedPipeline> _liteParsedPipeline;
    };

    static boost::intrusive_ptr<DocumentSource> createFromBson(
        BSONElement elem, const boost::intrusive_ptr<ExpressionContext>& expCtx);

    GetNextResult getNext() final;

    const char* getSourceName() const final {
        return "$unionWith";
    }

    void serializeToArray(
        std::vector<Value>& array,
        boost::optional<ExplainOptions::Verbosity> explain = boost::none) const final;

    Value serialize(boost::optional<ExplainOptions::Verbosity> explain = boost::none) const final {
        MONGO_UNREACHABLE;
    }

    StageConstraints constraints(Pipeline::SplitState pipeState) const final {
        return {StreamType::kStreaming,
                PositionRequirement::kNone,
                HostTypeRequirement::kPrimaryShard,
                DiskUseRequirement::kNoDiskUse,
                FacetRequirement::kAllowed,
                TransactionRequirement::kAllowed};
    }

    void detachFromOperationContext() final;
    void reattachToOperationContext(OperationContext* opCtx) final;

protected:
    void doDispose() final;

private:
    DocumentSourceUnionWith(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                            NamespaceString unionNss,
                            std::vector<BSONObj> pipeline);

    void initializeUnionPipeline();

    NamespaceString _unionNss;
    std::vector<BSONObj> _pipeline;
    boost::intrusive_ptr<ExpressionContext> _unionExpCtx;
    std::unique_ptr<Pipeline, PipelineDeleter> _unionPipeline;
    bool _drainingSource = true;
};

}  // namespace mongo
