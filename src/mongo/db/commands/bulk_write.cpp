/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/auth/action_set.h"
#include "mongo/db/auth/action_type.h"
#include "mongo/db/auth/authorization_session.h"
#include "mongo/db/auth/privilege.h"
#include "mongo/db/commands.h"
#include "mongo/db/ops/write_ops.h"
#include "mongo/db/ops/write_ops_exec.h"

namespace mongo {
namespace {

struct BulkWriteCounters {
    long long nInserted = 0;
    long long nMatched = 0;
    long long nModified = 0;
    long long nUpserted = 0;
    long long nDeleted = 0;
    long long nErrors = 0;
};

struct BulkWriteOpResult {
    BSONObj reply;
    bool ok = true;
};

BSONObj getArrayEntry(const BSONObj& arrayObj, int index) {
    auto elem = arrayObj[std::to_string(index)];
    uassert(ErrorCodes::BadValue,
            str::stream() << "Missing nsInfo entry for index " << index,
            !elem.eoo());
    uassert(ErrorCodes::TypeMismatch,
            str::stream() << "nsInfo entry " << index << " must be an object",
            elem.type() == BSONType::Object);
    return elem.Obj();
}

NamespaceString parseNamespaceForOp(const BSONObj& nsInfo, int index) {
    auto nsEntry = getArrayEntry(nsInfo, index);
    auto nsElem = nsEntry["ns"];
    uassert(ErrorCodes::TypeMismatch, "nsInfo entries require string field 'ns'", nsElem.type() == String);
    NamespaceString nss(nsElem.String());
    uassert(ErrorCodes::InvalidNamespace,
            str::stream() << "Invalid namespace in bulkWrite nsInfo: " << nss.ns(),
            nss.isValid() && !nss.isCommand());
    return nss;
}

int parseRequiredNamespaceIndex(const BSONObj& op, StringData fieldName) {
    auto elem = op[fieldName];
    uassert(ErrorCodes::NoSuchKey,
            str::stream() << "bulkWrite op missing required field '" << fieldName << "'",
            !elem.eoo());
    uassert(ErrorCodes::TypeMismatch,
            str::stream() << "bulkWrite field '" << fieldName << "' must be an integer",
            elem.isNumber());
    return elem.numberInt();
}

BSONObj parseRequiredObject(const BSONObj& op, StringData fieldName) {
    auto elem = op[fieldName];
    uassert(ErrorCodes::NoSuchKey,
            str::stream() << "bulkWrite op missing required field '" << fieldName << "'",
            !elem.eoo());
    uassert(ErrorCodes::TypeMismatch,
            str::stream() << "bulkWrite field '" << fieldName << "' must be an object",
            elem.type() == BSONType::Object);
    return elem.Obj().getOwned();
}

bool parseOptionalBool(const BSONObj& op, StringData fieldName, bool defaultValue) {
    auto elem = op[fieldName];
    if (elem.eoo()) {
        return defaultValue;
    }
    uassert(ErrorCodes::TypeMismatch,
            str::stream() << "bulkWrite field '" << fieldName << "' must be a bool",
            elem.type() == Bool);
    return elem.Bool();
}

bool parseOptionalBoolCommandField(const BSONObj& cmdObj, StringData fieldName, bool defaultValue) {
    auto elem = cmdObj[fieldName];
    if (elem.eoo()) {
        return defaultValue;
    }
    uassert(ErrorCodes::TypeMismatch,
            str::stream() << "bulkWrite field '" << fieldName << "' must be a bool",
            elem.type() == Bool);
    return elem.Bool();
}

write_ops::WriteCommandBase makeWriteCommandBase(const BSONObj& cmdObj) {
    write_ops::WriteCommandBase base;
    base.setOrdered(true);

    if (auto letElem = cmdObj["let"]) {
        uassert(ErrorCodes::TypeMismatch, "bulkWrite let must be an object", letElem.type() == Object);
        base.setLet(letElem.Obj().getOwned());
    }
    if (auto runtimeConstantsElem = cmdObj["runtimeConstants"]) {
        uassert(ErrorCodes::TypeMismatch,
                "bulkWrite runtimeConstants must be an object",
                runtimeConstantsElem.type() == Object);
        base.setRuntimeConstants(runtimeConstantsElem.Obj().getOwned());
    }
    if (auto bypassElem = cmdObj["bypassDocumentValidation"]) {
        uassert(ErrorCodes::TypeMismatch,
                "bulkWrite bypassDocumentValidation must be a bool",
                bypassElem.type() == Bool);
        base.setBypassDocumentValidation(bypassElem.Bool());
    }

    return base;
}

BSONObj makeSuccessReply(int idx, const SingleWriteResult& result, bool isUpdate) {
    BSONObjBuilder builder;
    builder.append("ok", 1);
    builder.append("idx", idx);
    builder.appendNumber("n", static_cast<long long>(result.getN()));
    if (isUpdate) {
        builder.appendNumber("nModified", static_cast<long long>(result.getNModified()));
        if (auto upsertedId = result.getUpsertedId().firstElement()) {
            builder.appendAs(upsertedId, "upserted");
        }
    }
    return builder.obj();
}

BSONObj makeErrorReply(int idx, const Status& status) {
    BSONObjBuilder builder;
    builder.append("ok", 0);
    builder.append("idx", idx);
    builder.append("code", int(status.code()));
    builder.append("errmsg", status.reason());
    return builder.obj();
}

Status checkAuthForBulkWriteOp(AuthorizationSession* authSession,
                               const BSONObj& cmdObj,
                               const BSONObj& nsInfo,
                               const BSONObj& op) {
    try {
        const bool bypassDocumentValidation =
            parseOptionalBoolCommandField(cmdObj, "bypassDocumentValidation"_sd, false);
        const bool hasInsert = !op["insert"].eoo();
        const bool hasUpdate = !op["update"].eoo();
        const bool hasDelete = !op["delete"].eoo();
        uassert(ErrorCodes::FailedToParse,
                "bulkWrite op must contain exactly one of insert, update, or delete",
                int(hasInsert) + int(hasUpdate) + int(hasDelete) == 1);

        NamespaceString nss;
        ActionSet actions;
        if (hasInsert) {
            nss = parseNamespaceForOp(nsInfo, parseRequiredNamespaceIndex(op, "insert"_sd));
            actions.addAction(ActionType::insert);
        } else if (hasUpdate) {
            nss = parseNamespaceForOp(nsInfo, parseRequiredNamespaceIndex(op, "update"_sd));
            actions.addAction(ActionType::update);
            if (parseOptionalBool(op, "upsert"_sd, false)) {
                actions.addAction(ActionType::insert);
            }
        } else {
            nss = parseNamespaceForOp(nsInfo, parseRequiredNamespaceIndex(op, "delete"_sd));
            actions.addAction(ActionType::remove);
        }

        if (bypassDocumentValidation) {
            actions.addAction(ActionType::bypassDocumentValidation);
        }

        if (!authSession->isAuthorizedForActionsOnResource(
                ResourcePattern::forExactNamespace(nss), actions)) {
            return Status(ErrorCodes::Unauthorized, "unauthorized");
        }
        return Status::OK();
    } catch (const DBException& ex) {
        return ex.toStatus();
    }
}

BulkWriteOpResult performInsertOp(OperationContext* opCtx,
                                  const BSONObj& cmdObj,
                                  const BSONObj& nsInfo,
                                  const BSONObj& op,
                                  int idx,
                                  BulkWriteCounters* counters) {
    const auto nss = parseNamespaceForOp(nsInfo, parseRequiredNamespaceIndex(op, "insert"_sd));
    write_ops::Insert insert(nss);
    insert.setWriteCommandBase(makeWriteCommandBase(cmdObj));
    insert.setDocuments({parseRequiredObject(op, "document"_sd)});

    auto result = performInserts(opCtx, insert);
    invariant(result.results.size() == 1);
    if (!result.results[0].isOK()) {
        counters->nErrors++;
        return {makeErrorReply(idx, result.results[0].getStatus()), false};
    }

    const auto& writeResult = result.results[0].getValue();
    counters->nInserted += writeResult.getN();
    return {makeSuccessReply(idx, writeResult, false), true};
}

BulkWriteOpResult performUpdateOp(OperationContext* opCtx,
                                  const BSONObj& cmdObj,
                                  const BSONObj& nsInfo,
                                  const BSONObj& op,
                                  int idx,
                                  BulkWriteCounters* counters) {
    const auto nss = parseNamespaceForOp(nsInfo, parseRequiredNamespaceIndex(op, "update"_sd));
    auto filter = parseRequiredObject(op, "filter"_sd);
    auto updateElem = op["updateMods"];
    uassert(ErrorCodes::NoSuchKey, "bulkWrite update op missing updateMods", !updateElem.eoo());
    uassert(ErrorCodes::TypeMismatch,
            "bulkWrite updateMods must be an object or array",
            updateElem.type() == Object || updateElem.type() == Array);

    write_ops::UpdateOpEntry entry;
    entry.setQ(filter);
    entry.setU(write_ops::UpdateModification(updateElem.Obj(), updateElem.type() == Array));
    entry.setMulti(parseOptionalBool(op, "multi"_sd, false));
    entry.setUpsert(parseOptionalBool(op, "upsert"_sd, false));
    if (auto hintElem = op["hint"]) {
        entry.setHint(write_ops::readWriteHint(hintElem));
    }
    if (auto collationElem = op["collation"]) {
        uassert(ErrorCodes::TypeMismatch, "bulkWrite collation must be an object", collationElem.type() == Object);
        entry.setCollation(collationElem.Obj().getOwned());
    }
    if (auto arrayFiltersElem = op["arrayFilters"]) {
        uassert(ErrorCodes::TypeMismatch,
                "bulkWrite arrayFilters must be an array",
                arrayFiltersElem.type() == Array);
        std::vector<BSONObj> arrayFilters;
        for (auto&& arrayFilter : arrayFiltersElem.Obj()) {
            uassert(ErrorCodes::TypeMismatch,
                    "bulkWrite arrayFilters entries must be objects",
                    arrayFilter.type() == Object);
            arrayFilters.push_back(arrayFilter.Obj().getOwned());
        }
        entry.setArrayFilters(std::move(arrayFilters));
    }

    write_ops::Update update(nss);
    update.setWriteCommandBase(makeWriteCommandBase(cmdObj));
    update.setUpdates({std::move(entry)});

    auto result = performUpdates(opCtx, update);
    invariant(result.results.size() == 1);
    if (!result.results[0].isOK()) {
        counters->nErrors++;
        return {makeErrorReply(idx, result.results[0].getStatus()), false};
    }

    const auto& writeResult = result.results[0].getValue();
    counters->nMatched += writeResult.getN();
    counters->nModified += writeResult.getNModified();
    if (!writeResult.getUpsertedId().isEmpty()) {
        counters->nUpserted++;
    }
    return {makeSuccessReply(idx, writeResult, true), true};
}

BulkWriteOpResult performDeleteOp(OperationContext* opCtx,
                                  const BSONObj& cmdObj,
                                  const BSONObj& nsInfo,
                                  const BSONObj& op,
                                  int idx,
                                  BulkWriteCounters* counters) {
    const auto nss = parseNamespaceForOp(nsInfo, parseRequiredNamespaceIndex(op, "delete"_sd));
    write_ops::DeleteOpEntry entry;
    entry.setQ(parseRequiredObject(op, "filter"_sd));
    entry.setMulti(parseOptionalBool(op, "multi"_sd, false));
    if (auto hintElem = op["hint"]) {
        entry.setHint(write_ops::readWriteHint(hintElem));
    }
    if (auto collationElem = op["collation"]) {
        uassert(ErrorCodes::TypeMismatch, "bulkWrite collation must be an object", collationElem.type() == Object);
        entry.setCollation(collationElem.Obj().getOwned());
    }

    write_ops::Delete deleteOp(nss);
    deleteOp.setWriteCommandBase(makeWriteCommandBase(cmdObj));
    deleteOp.setDeletes({std::move(entry)});

    auto result = performDeletes(opCtx, deleteOp);
    invariant(result.results.size() == 1);
    if (!result.results[0].isOK()) {
        counters->nErrors++;
        return {makeErrorReply(idx, result.results[0].getStatus()), false};
    }

    const auto& writeResult = result.results[0].getValue();
    counters->nDeleted += writeResult.getN();
    return {makeSuccessReply(idx, writeResult, false), true};
}

BulkWriteOpResult performBulkWriteOp(OperationContext* opCtx,
                                     const BSONObj& cmdObj,
                                     const BSONObj& nsInfo,
                                     const BSONObj& op,
                                     int idx,
                                     BulkWriteCounters* counters) {
    try {
        const bool hasInsert = !op["insert"].eoo();
        const bool hasUpdate = !op["update"].eoo();
        const bool hasDelete = !op["delete"].eoo();
        uassert(ErrorCodes::FailedToParse,
                "bulkWrite op must contain exactly one of insert, update, or delete",
                int(hasInsert) + int(hasUpdate) + int(hasDelete) == 1);

        if (hasInsert) {
            return performInsertOp(opCtx, cmdObj, nsInfo, op, idx, counters);
        }
        if (hasUpdate) {
            return performUpdateOp(opCtx, cmdObj, nsInfo, op, idx, counters);
        }
        return performDeleteOp(opCtx, cmdObj, nsInfo, op, idx, counters);
    } catch (const DBException& ex) {
        counters->nErrors++;
        return {makeErrorReply(idx, ex.toStatus()), false};
    }
}

class CmdBulkWrite : public BasicCommand {
public:
    CmdBulkWrite() : BasicCommand("bulkWrite") {}

    bool adminOnly() const override {
        return true;
    }

    AllowedOnSecondary secondaryAllowed(ServiceContext*) const override {
        return AllowedOnSecondary::kNever;
    }

    bool supportsWriteConcern(const BSONObj&) const override {
        return true;
    }

    ReadWriteType getReadWriteType() const override {
        return ReadWriteType::kWrite;
    }

    std::string help() const override {
        return "perform mixed write operations across namespaces";
    }

    Status checkAuthForOperation(OperationContext* opCtx,
                                 const std::string&,
                                 const BSONObj& cmdObj) const override {
        auto opsElem = cmdObj["ops"];
        auto nsInfoElem = cmdObj["nsInfo"];
        if (opsElem.eoo() || nsInfoElem.eoo()) {
            return Status(ErrorCodes::NoSuchKey, "bulkWrite requires 'ops' and 'nsInfo'");
        }
        if (opsElem.type() != Array || nsInfoElem.type() != Array) {
            return Status(ErrorCodes::TypeMismatch, "bulkWrite ops and nsInfo must be arrays");
        }

        auto authSession = AuthorizationSession::get(opCtx->getClient());
        auto nsInfo = nsInfoElem.Obj();
        for (auto&& opElem : opsElem.Obj()) {
            if (opElem.type() != Object) {
                return Status(ErrorCodes::TypeMismatch, "bulkWrite ops entries must be objects");
            }
            auto status = checkAuthForBulkWriteOp(authSession, cmdObj, nsInfo, opElem.Obj());
            if (!status.isOK()) {
                return status;
            }
        }
        return Status::OK();
    }

    bool run(OperationContext* opCtx,
             const std::string& dbName,
             const BSONObj& cmdObj,
             BSONObjBuilder& result) override {
        auto opsElem = cmdObj["ops"];
        auto nsInfoElem = cmdObj["nsInfo"];
        uassert(ErrorCodes::NoSuchKey, "bulkWrite requires 'ops'", !opsElem.eoo());
        uassert(ErrorCodes::NoSuchKey, "bulkWrite requires 'nsInfo'", !nsInfoElem.eoo());
        uassert(ErrorCodes::TypeMismatch, "bulkWrite ops must be an array", opsElem.type() == Array);
        uassert(ErrorCodes::TypeMismatch, "bulkWrite nsInfo must be an array", nsInfoElem.type() == Array);

        const bool ordered = parseOptionalBoolCommandField(cmdObj, "ordered"_sd, true);
        const bool errorsOnly = parseOptionalBoolCommandField(cmdObj, "errorsOnly"_sd, false);
        auto ops = opsElem.Obj();
        auto nsInfo = nsInfoElem.Obj();

        BulkWriteCounters counters;
        std::vector<BSONObj> replies;
        int idx = 0;
        for (auto&& opElem : ops) {
            uassert(ErrorCodes::TypeMismatch,
                    "bulkWrite ops entries must be objects",
                    opElem.type() == Object);
            auto opResult =
                performBulkWriteOp(opCtx, cmdObj, nsInfo, opElem.Obj().getOwned(), idx, &counters);
            if (!errorsOnly || !opResult.ok) {
                replies.push_back(opResult.reply);
            }
            if (!opResult.ok && ordered) {
                break;
            }
            ++idx;
        }

        result.appendNumber("nInserted", counters.nInserted);
        result.appendNumber("nMatched", counters.nMatched);
        result.appendNumber("nModified", counters.nModified);
        result.appendNumber("nUpserted", counters.nUpserted);
        result.appendNumber("nDeleted", counters.nDeleted);
        result.appendNumber("nErrors", counters.nErrors);

        BSONObjBuilder cursor(result.subobjStart("cursor"));
        cursor.append("id", 0LL);
        cursor.append("ns", NamespaceString(dbName, "$cmd.bulkWrite").ns());
        cursor.append("firstBatch", replies);
        cursor.doneFast();
        return true;
    }
} cmdBulkWrite;

}  // namespace
}  // namespace mongo
