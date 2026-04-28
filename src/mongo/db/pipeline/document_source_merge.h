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
#include "mongo/db/pipeline/expression.h"

namespace mongo {

class DocumentSourceMerge final : public DocumentSource, public NeedsMergerDocumentSource {
public:
    static std::unique_ptr<LiteParsedDocumentSource> liteParse(
        const AggregationRequest& request, const BSONElement& spec);

    ~DocumentSourceMerge() final = default;
    GetNextResult getNext() final;
    const char* getSourceName() const final;
    Value serialize(boost::optional<ExplainOptions::Verbosity> explain = boost::none) const final;
    GetDepsReturn getDependencies(DepsTracker* deps) const final;

    StageConstraints constraints(Pipeline::SplitState pipeState) const final {
        return {StreamType::kStreaming,
                PositionRequirement::kLast,
                HostTypeRequirement::kPrimaryShard,
                DiskUseRequirement::kWritesPersistentData,
                FacetRequirement::kNotAllowed,
                TransactionRequirement::kNotAllowed};
    }

    boost::intrusive_ptr<DocumentSource> getShardSource() final {
        return nullptr;
    }
    std::list<boost::intrusive_ptr<DocumentSource>> getMergeSources() final {
        return {this};
    }

    static boost::intrusive_ptr<DocumentSource> createFromBson(
        BSONElement elem, const boost::intrusive_ptr<ExpressionContext>& expCtx);

    enum class WhenMatched { kReplace, kKeepExisting, kMerge, kFail, kPipeline };
    enum class WhenNotMatched { kInsert, kDiscard, kFail };

private:
    DocumentSourceMerge(const NamespaceString& targetNss,
                        std::vector<std::string> onFields,
                        WhenMatched whenMatched,
                        WhenNotMatched whenNotMatched,
                        BSONObj whenMatchedPipeline,
                        BSONObj letSpec,
                        const boost::intrusive_ptr<ExpressionContext>& expCtx);

    void applyMerge(const BSONObj& doc);
    BSONObj buildQuery(const BSONObj& doc) const;
    BSONObj buildSetUpdate(const BSONObj& doc) const;
    BSONObj buildLetVariables(const BSONObj& doc) const;
    bool runWriteCommandWithFreshRecoveryUnit(const BSONObj& cmd, BSONObj* info);
    void runInsertCommand(const BSONObj& doc);
    void runUpdateCommand(const BSONObj& query, const BSONObj& update, StringData operation);
    void applyPipelineUpdate(const BSONObj& query, const BSONObj& doc);
    void assertWriteCommandSucceeded(StringData operation, bool ok, const BSONObj& info) const;
    void assertLastWriteSucceeded(StringData operation) const;

    struct LetVariable {
        std::string name;
        boost::intrusive_ptr<Expression> expression;
    };

    NamespaceString _targetNss;
    std::vector<std::string> _onFields;
    WhenMatched _whenMatched;
    WhenNotMatched _whenNotMatched;
    BSONObj _whenMatchedPipeline;
    std::vector<LetVariable> _letVariables;
    bool _done = false;
};

}  // namespace mongo
