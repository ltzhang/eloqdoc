/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "mongo/client/dbclientcursor.h"
#include "mongo/client/dbclientinterface.h"
#include "mongo/db/bson/dotted_path_support.h"
#include "mongo/db/auth/action_set.h"
#include "mongo/db/auth/action_type.h"
#include "mongo/db/auth/authorization_session.h"
#include "mongo/db/auth/privilege.h"
#include "mongo/db/commands.h"
#include "mongo/db/dbdirectclient.h"
#include "mongo/db/jsobj.h"

namespace mongo {
namespace {

namespace dps = dotted_path_support;

bool isAnalyzableScalar(BSONElement elem) {
    switch (elem.type()) {
        case NumberDouble:
        case String:
        case Bool:
        case Date:
        case jstOID:
        case NumberInt:
        case NumberLong:
        case NumberDecimal:
            return true;
        default:
            return false;
    }
}

std::string valueKey(BSONElement elem) {
    return str::stream() << elem.canonicalType() << ":" << elem.toString(false);
}

struct FieldStats {
    long long observed = 0;
    std::set<std::string> uniqueValues;
    std::map<std::string, std::pair<BSONObj, long long>> buckets;
    BSONObj minValue;
    BSONObj maxValue;

    void observe(BSONElement elem) {
        if (!isAnalyzableScalar(elem)) {
            return;
        }

        BSONObj wrapped = elem.wrap("value").getOwned();
        if (observed == 0 || wrapped.firstElement().woCompare(minValue.firstElement(), false) < 0) {
            minValue = wrapped;
        }
        if (observed == 0 || wrapped.firstElement().woCompare(maxValue.firstElement(), false) > 0) {
            maxValue = wrapped;
        }

        const auto key = valueKey(elem);
        uniqueValues.insert(key);
        auto& bucket = buckets[key];
        if (bucket.first.isEmpty()) {
            bucket.first = wrapped;
        }
        bucket.second++;
        observed++;
    }

    BSONObj toBSON(const NamespaceString& sourceNss,
                  StringData fieldName,
                  long long sampledDocuments) const {
        BSONObjBuilder builder;
        builder.append("_id", str::stream() << sourceNss.ns() << ":" << fieldName);
        builder.append("collection", sourceNss.coll());
        builder.append("ns", sourceNss.ns());
        builder.append("field", fieldName);
        builder.appendNumber("sampledDocuments", sampledDocuments);
        builder.appendNumber("observedValues", observed);
        builder.appendNumber("cardinality", static_cast<long long>(uniqueValues.size()));
        if (!minValue.isEmpty()) {
            builder.appendAs(minValue.firstElement(), "min");
        }
        if (!maxValue.isEmpty()) {
            builder.appendAs(maxValue.firstElement(), "max");
        }

        BSONArrayBuilder histogram(builder.subarrayStart("histogram"));
        for (auto&& bucket : buckets) {
            BSONObjBuilder bucketBuilder(histogram.subobjStart());
            bucketBuilder.appendAs(bucket.second.first.firstElement(), "value");
            bucketBuilder.appendNumber("count", bucket.second.second);
            bucketBuilder.doneFast();
        }
        histogram.doneFast();
        return builder.obj();
    }
};

boost::optional<long long> parseSampleSize(const BSONObj& cmdObj) {
    auto elem = cmdObj["sampleSize"];
    if (elem.eoo()) {
        return boost::none;
    }
    uassert(ErrorCodes::TypeMismatch, "analyze sampleSize must be numeric", elem.isNumber());
    const long long sampleSize = elem.numberLong();
    uassert(ErrorCodes::BadValue, "analyze sampleSize must be greater than zero", sampleSize > 0);
    return sampleSize;
}

boost::optional<double> parseSampleRate(const BSONObj& cmdObj) {
    auto elem = cmdObj["sampleRate"];
    if (elem.eoo()) {
        return boost::none;
    }
    uassert(ErrorCodes::TypeMismatch, "analyze sampleRate must be numeric", elem.isNumber());
    const double sampleRate = elem.numberDouble();
    uassert(ErrorCodes::BadValue,
            "analyze sampleRate must be greater than zero and no more than one",
            sampleRate > 0.0 && sampleRate <= 1.0);
    return sampleRate;
}

void assertLastWriteSucceeded(DBDirectClient* client, StringData dbName) {
    auto gle = client->getLastErrorDetailed(dbName.toString());
    auto err = gle["err"];
    uassert(ErrorCodes::OperationFailed,
            str::stream() << "analyze statistics write failed: " << gle,
            err.eoo() || err.isNull());
}

class CmdAnalyze : public BasicCommand {
public:
    CmdAnalyze() : BasicCommand("analyze") {}

    bool adminOnly() const override {
        return false;
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
        return "compute and persist basic collection statistics";
    }

    Status checkAuthForOperation(OperationContext* opCtx,
                                 const std::string& dbName,
                                 const BSONObj& cmdObj) const override {
        const auto sourceNss = CommandHelpers::parseNsCollectionRequired(dbName, cmdObj);
        const NamespaceString statsNss(dbName,
                                       str::stream() << "system.statistics." << sourceNss.coll());

        auto authSession = AuthorizationSession::get(opCtx->getClient());
        if (!authSession->isAuthorizedForActionsOnResource(
                ResourcePattern::forExactNamespace(sourceNss), ActionType::find)) {
            return Status(ErrorCodes::Unauthorized, "unauthorized to read analyzed collection");
        }

        ActionSet statsActions;
        statsActions.addAction(ActionType::insert);
        statsActions.addAction(ActionType::remove);
        if (!authSession->isAuthorizedForActionsOnResource(
                ResourcePattern::forExactNamespace(statsNss), statsActions)) {
            return Status(ErrorCodes::Unauthorized, "unauthorized to write statistics collection");
        }
        return Status::OK();
    }

    bool run(OperationContext* opCtx,
             const std::string& dbName,
             const BSONObj& cmdObj,
             BSONObjBuilder& result) override {
        const auto sourceNss = CommandHelpers::parseNsCollectionRequired(dbName, cmdObj);
        const NamespaceString statsNss(dbName,
                                       str::stream() << "system.statistics." << sourceNss.coll());

        const auto sampleSize = parseSampleSize(cmdObj);
        const auto sampleRate = parseSampleRate(cmdObj);
        uassert(ErrorCodes::BadValue,
                "analyze accepts at most one of sampleSize and sampleRate",
                !(sampleSize && sampleRate));

        boost::optional<std::string> key;
        if (auto keyElem = cmdObj["key"]) {
            uassert(ErrorCodes::TypeMismatch,
                    "analyze key must be a string",
                    keyElem.type() == String);
            key = keyElem.String();
            uassert(ErrorCodes::BadValue, "analyze key must not be empty", !key->empty());
        }

        DBDirectClient client(opCtx);
        const auto totalDocuments = static_cast<long long>(client.count(sourceNss.ns()));
        long long targetSamples = totalDocuments;
        if (sampleSize) {
            targetSamples = std::min(*sampleSize, totalDocuments);
        } else if (sampleRate) {
            targetSamples = std::min(static_cast<long long>(
                                         std::ceil(totalDocuments * *sampleRate)),
                                     totalDocuments);
        }

        std::map<std::string, FieldStats> statsByField;
        long long sampledDocuments = 0;
        auto cursor = client.query(sourceNss.ns(), Query(), 0, 0, nullptr, 0, 0);
        while (cursor && cursor->more() && sampledDocuments < targetSamples) {
            const auto doc = cursor->nextSafe();
            sampledDocuments++;

            if (key) {
                auto elem = dps::extractElementAtPath(doc, *key);
                if (!elem.eoo()) {
                    statsByField[*key].observe(elem);
                }
                continue;
            }

            for (auto&& elem : doc) {
                if (elem.fieldNameStringData() == "_id"_sd || !isAnalyzableScalar(elem)) {
                    continue;
                }
                statsByField[elem.fieldName()].observe(elem);
            }
        }

        client.remove(statsNss.ns(), Query());
        assertLastWriteSucceeded(&client, dbName);
        std::vector<BSONObj> statsDocs;
        for (auto&& fieldStats : statsByField) {
            if (fieldStats.second.observed == 0) {
                continue;
            }
            statsDocs.push_back(
                fieldStats.second.toBSON(sourceNss, fieldStats.first, sampledDocuments));
        }
        if (!statsDocs.empty()) {
            client.insert(statsNss.ns(), statsDocs);
            assertLastWriteSucceeded(&client, dbName);
        }

        result.append("collection", sourceNss.coll());
        result.appendNumber("sampledDocuments", sampledDocuments);
        result.appendNumber("fieldsAnalyzed", static_cast<long long>(statsDocs.size()));
        BSONArrayBuilder fields(result.subarrayStart("fields"));
        for (auto&& statsDoc : statsDocs) {
            fields.append(statsDoc["field"].String());
        }
        fields.doneFast();
        return true;
    }
} cmdAnalyze;

}  // namespace
}  // namespace mongo
