/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/pipeline/document_source_union_with.h"

#include "mongo/db/pipeline/aggregation_request.h"
#include "mongo/db/pipeline/document.h"
#include "mongo/db/pipeline/expression_context.h"
#include "mongo/db/pipeline/mongo_process_interface.h"
#include "mongo/stdx/memory.h"

namespace mongo {

using boost::intrusive_ptr;

namespace {

NamespaceString parseUnionNamespace(const NamespaceString& baseNss, StringData coll) {
    NamespaceString unionNss(baseNss.db(), coll);
    uassert(ErrorCodes::InvalidNamespace,
            str::stream() << "invalid $unionWith namespace: " << unionNss.ns(),
            unionNss.isValid() && !unionNss.isCommand());
    return unionNss;
}

std::vector<BSONObj> parseUnionPipeline(BSONElement pipelineElem) {
    if (!pipelineElem) {
        return {};
    }
    return uassertStatusOK(AggregationRequest::parsePipelineFromBSON(pipelineElem));
}

}  // namespace

std::unique_ptr<DocumentSourceUnionWith::LiteParsed> DocumentSourceUnionWith::LiteParsed::parse(
    const AggregationRequest& request, const BSONElement& spec) {
    NamespaceString unionNss;
    std::vector<BSONObj> pipeline;

    if (spec.type() == BSONType::String) {
        unionNss = parseUnionNamespace(request.getNamespaceString(), spec.valueStringData());
    } else if (spec.type() == BSONType::Object) {
        auto specObj = spec.Obj();
        auto collElem = specObj["coll"];
        uassert(ErrorCodes::FailedToParse,
                "$unionWith object form requires a string 'coll' field",
                collElem.type() == BSONType::String);
        unionNss = parseUnionNamespace(request.getNamespaceString(), collElem.valueStringData());
        pipeline = parseUnionPipeline(specObj["pipeline"]);

        for (auto&& elem : specObj) {
            const auto fieldName = elem.fieldNameStringData();
            uassert(ErrorCodes::FailedToParse,
                    str::stream() << "unknown $unionWith option '" << fieldName << "'",
                    fieldName == "coll"_sd || fieldName == "pipeline"_sd);
        }
    } else {
        uasserted(ErrorCodes::TypeMismatch,
                  str::stream() << "$unionWith must be a string or object, got "
                                << typeName(spec.type()));
    }

    stdx::unordered_set<NamespaceString> involvedNamespaces{unionNss};
    boost::optional<LiteParsedPipeline> liteParsedPipeline;
    if (!pipeline.empty()) {
        AggregationRequest unionRequest(unionNss, pipeline);
        liteParsedPipeline.emplace(unionRequest);
        auto pipelineNamespaces = liteParsedPipeline->getInvolvedNamespaces();
        involvedNamespaces.insert(pipelineNamespaces.begin(), pipelineNamespaces.end());
    }

    return stdx::make_unique<DocumentSourceUnionWith::LiteParsed>(
        std::move(unionNss), std::move(involvedNamespaces), std::move(liteParsedPipeline));
}

PrivilegeVector DocumentSourceUnionWith::LiteParsed::requiredPrivileges(bool isMongos) const {
    PrivilegeVector requiredPrivileges;
    Privilege::addPrivilegeToPrivilegeVector(
        &requiredPrivileges,
        Privilege(ResourcePattern::forExactNamespace(_unionNss), ActionType::find));

    if (_liteParsedPipeline) {
        Privilege::addPrivilegesToPrivilegeVector(
            &requiredPrivileges, _liteParsedPipeline->requiredPrivileges(isMongos));
    }

    return requiredPrivileges;
}

REGISTER_DOCUMENT_SOURCE(unionWith,
                         DocumentSourceUnionWith::LiteParsed::parse,
                         DocumentSourceUnionWith::createFromBson);

DocumentSourceUnionWith::DocumentSourceUnionWith(
    const intrusive_ptr<ExpressionContext>& expCtx,
    NamespaceString unionNss,
    std::vector<BSONObj> pipeline)
    : DocumentSource(expCtx),
      _unionNss(std::move(unionNss)),
      _pipeline(std::move(pipeline)),
      _unionExpCtx(expCtx->copyWith(_unionNss)) {}

intrusive_ptr<DocumentSource> DocumentSourceUnionWith::createFromBson(
    BSONElement elem, const intrusive_ptr<ExpressionContext>& expCtx) {
    NamespaceString unionNss;
    std::vector<BSONObj> pipeline;

    if (elem.type() == BSONType::String) {
        unionNss = parseUnionNamespace(expCtx->ns, elem.valueStringData());
    } else if (elem.type() == BSONType::Object) {
        auto specObj = elem.Obj();
        auto collElem = specObj["coll"];
        uassert(ErrorCodes::FailedToParse,
                "$unionWith object form requires a string 'coll' field",
                collElem.type() == BSONType::String);
        unionNss = parseUnionNamespace(expCtx->ns, collElem.valueStringData());
        pipeline = parseUnionPipeline(specObj["pipeline"]);

        for (auto&& option : specObj) {
            const auto fieldName = option.fieldNameStringData();
            uassert(ErrorCodes::FailedToParse,
                    str::stream() << "unknown $unionWith option '" << fieldName << "'",
                    fieldName == "coll"_sd || fieldName == "pipeline"_sd);
        }
    } else {
        uasserted(ErrorCodes::TypeMismatch,
                  str::stream() << "$unionWith must be a string or object, got "
                                << typeName(elem.type()));
    }

    return new DocumentSourceUnionWith(expCtx, std::move(unionNss), std::move(pipeline));
}

void DocumentSourceUnionWith::initializeUnionPipeline() {
    if (_unionPipeline) {
        return;
    }

    _unionPipeline =
        uassertStatusOK(pExpCtx->mongoProcessInterface->makePipeline(_pipeline, _unionExpCtx));
    _unionPipeline.get_deleter().dismissDisposal();
}

DocumentSource::GetNextResult DocumentSourceUnionWith::getNext() {
    pExpCtx->checkForInterrupt();

    if (_drainingSource) {
        auto input = pSource->getNext();
        if (input.isAdvanced() || input.isPaused()) {
            return input;
        }
        _drainingSource = false;
    }

    initializeUnionPipeline();
    if (auto doc = _unionPipeline->getNext()) {
        return std::move(*doc);
    }

    return GetNextResult::makeEOF();
}

void DocumentSourceUnionWith::serializeToArray(
    std::vector<Value>& array, boost::optional<ExplainOptions::Verbosity> explain) const {
    std::vector<Value> pipelineValues;
    pipelineValues.reserve(_pipeline.size());
    for (auto&& stage : _pipeline) {
        pipelineValues.emplace_back(stage);
    }

    MutableDocument spec;
    spec["coll"] = Value(_unionNss.coll());
    if (!_pipeline.empty()) {
        spec["pipeline"] = Value(std::move(pipelineValues));
    }
    array.push_back(Value(Document{{"$unionWith", spec.freezeToValue()}}));
}

void DocumentSourceUnionWith::detachFromOperationContext() {
    if (_unionPipeline) {
        _unionPipeline->detachFromOperationContext();
    }
    _unionExpCtx->opCtx = nullptr;
}

void DocumentSourceUnionWith::reattachToOperationContext(OperationContext* opCtx) {
    _unionExpCtx->opCtx = opCtx;
    if (_unionPipeline) {
        _unionPipeline->reattachToOperationContext(opCtx);
    }
}

void DocumentSourceUnionWith::doDispose() {
    if (_unionPipeline) {
        _unionPipeline->dispose(pExpCtx->opCtx);
        _unionPipeline.reset();
    }
}

}  // namespace mongo
