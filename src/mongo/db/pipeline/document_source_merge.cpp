/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/pipeline/document_source_merge.h"

#include "mongo/client/dbclientinterface.h"
#include "mongo/db/auth/action_type.h"
#include "mongo/db/auth/privilege.h"
#include "mongo/db/auth/resource_pattern.h"
#include "mongo/db/pipeline/aggregation_request.h"
#include "mongo/db/pipeline/document.h"
#include "mongo/db/pipeline/expression_context.h"
#include "mongo/db/pipeline/mongo_process_interface.h"
#include "mongo/db/pipeline/variables.h"
#include "mongo/stdx/memory.h"

namespace mongo {

using boost::intrusive_ptr;

namespace {

NamespaceString parseTargetNamespace(const NamespaceString& sourceNss, const BSONElement& spec) {
    if (spec.type() == BSONType::String) {
        NamespaceString targetNss(sourceNss.db(), spec.valueStringData());
        uassert(ErrorCodes::InvalidNamespace,
                str::stream() << "Invalid $merge target namespace, " << targetNss.ns(),
                targetNss.isValid() && !targetNss.isCommand());
        uassert(ErrorCodes::IllegalOperation,
                "$merge cross-database target is not supported by EloqDoc yet",
                targetNss.db() == sourceNss.db());
        return targetNss;
    }

    uassert(ErrorCodes::FailedToParse,
            "$merge 'into' must be a string or object",
            spec.type() == BSONType::Object);

    auto intoObj = spec.Obj();
    auto collElem = intoObj["coll"];
    uassert(ErrorCodes::FailedToParse,
            "$merge 'into' object requires a string 'coll' field",
            collElem.type() == BSONType::String);

    auto dbElem = intoObj["db"];
    const auto dbName = dbElem ? dbElem.valueStringData() : sourceNss.db();
    uassert(ErrorCodes::FailedToParse,
            "$merge 'into.db' must be a string",
            !dbElem || dbElem.type() == BSONType::String);

    for (auto&& elem : intoObj) {
        const auto fieldName = elem.fieldNameStringData();
        uassert(ErrorCodes::FailedToParse,
                str::stream() << "unknown $merge into option '" << fieldName << "'",
                fieldName == "db"_sd || fieldName == "coll"_sd);
    }

    NamespaceString targetNss(dbName, collElem.valueStringData());
    uassert(ErrorCodes::InvalidNamespace,
            str::stream() << "Invalid $merge target namespace, " << targetNss.ns(),
            targetNss.isValid() && !targetNss.isCommand());
    uassert(ErrorCodes::IllegalOperation,
            "$merge cross-database target is not supported by EloqDoc yet",
            targetNss.db() == sourceNss.db());
    return targetNss;
}

std::vector<std::string> parseOnFields(BSONElement elem) {
    if (!elem) {
        return {"_id"};
    }

    if (elem.type() == BSONType::String) {
        return {elem.String()};
    }

    uassert(ErrorCodes::FailedToParse,
            "$merge 'on' must be a string or array of strings",
            elem.type() == BSONType::Array);

    std::vector<std::string> fields;
    for (auto&& fieldElem : elem.Obj()) {
        uassert(ErrorCodes::FailedToParse,
                "$merge 'on' array entries must be strings",
                fieldElem.type() == BSONType::String);
        fields.push_back(fieldElem.String());
    }
    uassert(ErrorCodes::FailedToParse, "$merge 'on' array must not be empty", !fields.empty());
    return fields;
}

DocumentSourceMerge::WhenMatched parseWhenMatched(BSONElement elem) {
    if (!elem) {
        return DocumentSourceMerge::WhenMatched::kMerge;
    }

    if (elem.type() == BSONType::Array) {
        return DocumentSourceMerge::WhenMatched::kPipeline;
    }

    uassert(ErrorCodes::FailedToParse,
            "$merge 'whenMatched' must be a string or pipeline array",
            elem.type() == BSONType::String);

    const auto value = elem.valueStringData();
    if (value == "replace"_sd) {
        return DocumentSourceMerge::WhenMatched::kReplace;
    }
    if (value == "keepExisting"_sd) {
        return DocumentSourceMerge::WhenMatched::kKeepExisting;
    }
    if (value == "merge"_sd) {
        return DocumentSourceMerge::WhenMatched::kMerge;
    }
    if (value == "fail"_sd) {
        return DocumentSourceMerge::WhenMatched::kFail;
    }

    uasserted(ErrorCodes::FailedToParse,
              str::stream() << "unsupported $merge whenMatched mode '" << value << "'");
}

DocumentSourceMerge::WhenNotMatched parseWhenNotMatched(BSONElement elem) {
    if (!elem) {
        return DocumentSourceMerge::WhenNotMatched::kInsert;
    }

    uassert(ErrorCodes::FailedToParse,
            "$merge 'whenNotMatched' must be a string",
            elem.type() == BSONType::String);

    const auto value = elem.valueStringData();
    if (value == "insert"_sd) {
        return DocumentSourceMerge::WhenNotMatched::kInsert;
    }
    if (value == "discard"_sd) {
        return DocumentSourceMerge::WhenNotMatched::kDiscard;
    }
    if (value == "fail"_sd) {
        return DocumentSourceMerge::WhenNotMatched::kFail;
    }

    uasserted(ErrorCodes::FailedToParse,
              str::stream() << "unsupported $merge whenNotMatched mode '" << value << "'");
}

struct MergeSpec {
    NamespaceString targetNss;
    std::vector<std::string> onFields;
    DocumentSourceMerge::WhenMatched whenMatched;
    DocumentSourceMerge::WhenNotMatched whenNotMatched;
    BSONObj whenMatchedPipeline;
    BSONObj letSpec;
};

MergeSpec parseMergeSpec(const NamespaceString& sourceNss, const BSONElement& elem) {
    if (elem.type() == BSONType::String) {
        return {parseTargetNamespace(sourceNss, elem),
                {"_id"},
                DocumentSourceMerge::WhenMatched::kMerge,
                DocumentSourceMerge::WhenNotMatched::kInsert,
                BSONObj(),
                BSONObj()};
    }

    uassert(ErrorCodes::FailedToParse,
            "$merge specification must be a string or object",
            elem.type() == BSONType::Object);

    auto specObj = elem.Obj();
    auto intoElem = specObj["into"];
    uassert(ErrorCodes::FailedToParse, "$merge requires an 'into' field", intoElem);

    for (auto&& option : specObj) {
        const auto fieldName = option.fieldNameStringData();
        uassert(ErrorCodes::FailedToParse,
                str::stream() << "unknown $merge option '" << fieldName << "'",
                fieldName == "into"_sd || fieldName == "on"_sd ||
                    fieldName == "whenMatched"_sd || fieldName == "whenNotMatched"_sd ||
                    fieldName == "let"_sd);
    }

    BSONObj letSpec;
    if (auto letElem = specObj["let"]) {
        uassert(ErrorCodes::FailedToParse,
                "$merge 'let' must be an object",
                letElem.type() == BSONType::Object);
        letSpec = letElem.Obj().getOwned();
    }

    auto whenMatched = parseWhenMatched(specObj["whenMatched"]);
    BSONObj whenMatchedPipeline;
    if (whenMatched == DocumentSourceMerge::WhenMatched::kPipeline) {
        whenMatchedPipeline = specObj["whenMatched"].Obj().getOwned();
        uassert(ErrorCodes::FailedToParse,
                "$merge 'whenMatched' pipeline must not be empty",
                !whenMatchedPipeline.isEmpty());
    }

    return {parseTargetNamespace(sourceNss, intoElem),
            parseOnFields(specObj["on"]),
            whenMatched,
            parseWhenNotMatched(specObj["whenNotMatched"]),
            whenMatchedPipeline,
            letSpec};
}

}  // namespace

std::unique_ptr<LiteParsedDocumentSourceForeignCollections> DocumentSourceMerge::liteParse(
    const AggregationRequest& request, const BSONElement& spec) {
    auto parsed = parseMergeSpec(request.getNamespaceString(), spec);

    ActionSet actions{ActionType::find, ActionType::insert, ActionType::update};
    if (request.shouldBypassDocumentValidation()) {
        actions.addAction(ActionType::bypassDocumentValidation);
    }

    PrivilegeVector privileges{
        Privilege(ResourcePattern::forExactNamespace(parsed.targetNss), actions)};

    return stdx::make_unique<LiteParsedDocumentSourceForeignCollections>(
        std::move(parsed.targetNss), std::move(privileges));
}

REGISTER_DOCUMENT_SOURCE(merge, DocumentSourceMerge::liteParse, DocumentSourceMerge::createFromBson);

DocumentSourceMerge::DocumentSourceMerge(const NamespaceString& targetNss,
                                         std::vector<std::string> onFields,
                                         WhenMatched whenMatched,
                                         WhenNotMatched whenNotMatched,
                                         BSONObj whenMatchedPipeline,
                                         BSONObj letSpec,
                                         const intrusive_ptr<ExpressionContext>& expCtx)
    : DocumentSource(expCtx),
      _targetNss(targetNss),
      _onFields(std::move(onFields)),
      _whenMatched(whenMatched),
      _whenNotMatched(whenNotMatched),
      _whenMatchedPipeline(whenMatchedPipeline.getOwned()) {
    VariablesParseState vps = expCtx->variablesParseState;
    for (auto&& variable : letSpec) {
        auto name = variable.fieldNameStringData();
        Variables::uassertValidNameForUserWrite(name);
        _letVariables.push_back(
            {name.toString(), Expression::parseOperand(expCtx, variable, vps)->optimize()});
    }
}

const char* DocumentSourceMerge::getSourceName() const {
    return "$merge";
}

intrusive_ptr<DocumentSource> DocumentSourceMerge::createFromBson(
    BSONElement elem, const intrusive_ptr<ExpressionContext>& expCtx) {
    uassert(ErrorCodes::OperationNotSupportedInTransaction,
            "$merge cannot be used in a transaction",
            !expCtx->inMultiDocumentTransaction);

    auto parsed = parseMergeSpec(expCtx->ns, elem);
    uassert(6789102,
            "Can't $merge to special collection: " + parsed.targetNss.coll(),
            !parsed.targetNss.isSpecial());

    return new DocumentSourceMerge(parsed.targetNss,
                                   std::move(parsed.onFields),
                                   parsed.whenMatched,
                                   parsed.whenNotMatched,
                                   parsed.whenMatchedPipeline,
                                   parsed.letSpec,
                                   expCtx);
}

BSONObj DocumentSourceMerge::buildQuery(const BSONObj& doc) const {
    BSONObjBuilder query;
    for (auto&& fieldName : _onFields) {
        uassert(ErrorCodes::FailedToParse,
                str::stream() << "$merge 'on' field '" << fieldName
                              << "' must be top-level in this compatibility implementation",
                fieldName.find('.') == std::string::npos);
        auto elem = doc.getField(fieldName);
        uassert(ErrorCodes::NoMatchingDocument,
                str::stream() << "$merge could not find the 'on' field '" << fieldName
                              << "' in document " << doc,
                elem);
        query.appendAs(elem, fieldName);
    }
    return query.obj();
}

BSONObj DocumentSourceMerge::buildSetUpdate(const BSONObj& doc) const {
    BSONObjBuilder update;
    BSONObjBuilder set(update.subobjStart("$set"));
    for (auto&& elem : doc) {
        if (elem.fieldNameStringData() == "_id"_sd) {
            continue;
        }
        set.append(elem);
    }
    set.doneFast();
    return update.obj();
}

void DocumentSourceMerge::assertLastWriteSucceeded(StringData operation) const {
    BSONObj err = pExpCtx->mongoProcessInterface->directClient()->getLastErrorDetailed();
    uassert(51100,
            str::stream() << "$merge " << operation << " failed: " << err,
            DBClientBase::getLastErrorString(err).empty());
}

BSONObj DocumentSourceMerge::buildLetVariables(const BSONObj& doc) const {
    BSONObjBuilder letBuilder;
    letBuilder.append("new", doc);

    Document root(doc);
    for (auto&& variable : _letVariables) {
        variable.expression->evaluate(root).addToBsonObj(&letBuilder, variable.name);
    }

    return letBuilder.obj();
}

void DocumentSourceMerge::applyPipelineUpdate(const BSONObj& query, const BSONObj& doc) {
    BSONObjBuilder updateEntry;
    updateEntry.append("q", query);
    updateEntry.appendArray("u", _whenMatchedPipeline);
    updateEntry.append("multi", false);
    updateEntry.append("upsert", false);

    BSONArrayBuilder updates;
    updates.append(updateEntry.obj());

    BSONObjBuilder cmd;
    cmd.append("update", _targetNss.coll());
    cmd.append("updates", updates.arr());
    cmd.append("ordered", true);
    cmd.append("let", buildLetVariables(doc));

    BSONObj info;
    bool ok = pExpCtx->mongoProcessInterface->directClient()->runCommand(
        _targetNss.db().toString(), cmd.obj(), info);
    uassert(51101, str::stream() << "$merge pipeline update failed: " << info, ok);
}

void DocumentSourceMerge::applyMerge(const BSONObj& doc) {
    DBClientBase* conn = pExpCtx->mongoProcessInterface->directClient();
    const auto query = buildQuery(doc);
    const bool matched = !conn->findOne(_targetNss.ns(), query).isEmpty();

    if (!matched) {
        if (_whenNotMatched == WhenNotMatched::kDiscard) {
            return;
        }
        uassert(ErrorCodes::NoMatchingDocument,
                str::stream() << "$merge could not find a matching document for " << query,
                _whenNotMatched != WhenNotMatched::kFail);
        conn->insert(_targetNss.ns(), doc);
        assertLastWriteSucceeded("insert");
        return;
    }

    switch (_whenMatched) {
        case WhenMatched::kFail:
            uasserted(ErrorCodes::DuplicateKey,
                      str::stream() << "$merge found a matching document for " << query);
        case WhenMatched::kKeepExisting:
            return;
        case WhenMatched::kReplace:
            conn->update(_targetNss.ns(), query, doc, false, false);
            assertLastWriteSucceeded("replace");
            return;
        case WhenMatched::kMerge: {
            auto update = buildSetUpdate(doc);
            if (update["$set"].Obj().isEmpty()) {
                return;
            }
            conn->update(_targetNss.ns(), query, update, false, false);
            assertLastWriteSucceeded("update");
            return;
        }
        case WhenMatched::kPipeline:
            applyPipelineUpdate(query, doc);
            return;
    }
    MONGO_UNREACHABLE;
}

DocumentSource::GetNextResult DocumentSourceMerge::getNext() {
    pExpCtx->checkForInterrupt();

    if (_done) {
        return GetNextResult::makeEOF();
    }

    auto nextInput = pSource->getNext();
    for (; nextInput.isAdvanced(); nextInput = pSource->getNext()) {
        applyMerge(nextInput.releaseDocument().toBson());
    }

    if (nextInput.isPaused()) {
        return nextInput;
    }

    _done = true;
    return GetNextResult::makeEOF();
}

Value DocumentSourceMerge::serialize(boost::optional<ExplainOptions::Verbosity> explain) const {
    MutableDocument spec;
    spec["into"] = _targetNss.db() == pExpCtx->ns.db()
        ? Value(_targetNss.coll())
        : Value(Document{{"db", _targetNss.db()}, {"coll", _targetNss.coll()}});

    if (!(_onFields.size() == 1 && _onFields.front() == "_id")) {
        std::vector<Value> onFields;
        onFields.reserve(_onFields.size());
        for (auto&& field : _onFields) {
            onFields.emplace_back(field);
        }
        spec["on"] = _onFields.size() == 1 ? Value(_onFields.front()) : Value(onFields);
    }

    if (_whenMatched == WhenMatched::kPipeline) {
        std::vector<Value> stages;
        for (auto&& stage : _whenMatchedPipeline) {
            stages.emplace_back(Value(stage.Obj()));
        }
        spec["whenMatched"] = Value(stages);
    }

    if (!_letVariables.empty()) {
        MutableDocument letSpec;
        for (auto&& variable : _letVariables) {
            letSpec[variable.name] = variable.expression->serialize(false);
        }
        spec["let"] = letSpec.freezeToValue();
    }

    return Value(Document{{getSourceName(), spec.freeze()}});
}

DocumentSource::GetDepsReturn DocumentSourceMerge::getDependencies(DepsTracker* deps) const {
    deps->needWholeDocument = true;
    return EXHAUSTIVE_ALL;
}

}  // namespace mongo
