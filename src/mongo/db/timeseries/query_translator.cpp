/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/timeseries/query_translator.h"

#include "mongo/base/string_data.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {
namespace timeseries {
namespace {

std::string makeControlPath(StringData minOrMax, StringData timeField) {
    return str::stream() << "control." << minOrMax << "." << timeField;
}

void appendRangePredicate(BSONObjBuilder* builder,
                          StringData path,
                          StringData opName,
                          const BSONElement& value) {
    BSONObjBuilder predicate(builder->subobjStart(path));
    predicate.appendAs(value, opName);
    predicate.doneFast();
}

bool appendTimePredicate(BSONObjBuilder* builder,
                         StringData timeField,
                         const BSONElement& predicate) {
    const auto minPath = makeControlPath("min", timeField);
    const auto maxPath = makeControlPath("max", timeField);

    if (predicate.type() != mongo::Object) {
        appendRangePredicate(builder, minPath, "$lte", predicate);
        appendRangePredicate(builder, maxPath, "$gte", predicate);
        return true;
    }

    BSONObjBuilder minBuilder;
    BSONObjBuilder maxBuilder;
    bool hasMinPredicate = false;
    bool hasMaxPredicate = false;
    bool hasPredicate = false;
    bool hasEqPredicate = false;
    BSONElement eqPredicate;

    BSONForEach(op, predicate.Obj()) {
        const auto opName = op.fieldNameStringData();
        if (!opName.startsWith("$")) {
            return false;
        }

        if (opName == "$gte" || opName == "$gt") {
            maxBuilder.appendAs(op, opName);
            hasMaxPredicate = true;
            hasPredicate = true;
        } else if (opName == "$lte" || opName == "$lt") {
            minBuilder.appendAs(op, opName);
            hasMinPredicate = true;
            hasPredicate = true;
        } else if (opName == "$eq") {
            eqPredicate = op;
            hasEqPredicate = true;
            hasPredicate = true;
        } else {
            return false;
        }
    }

    if (hasEqPredicate) {
        appendRangePredicate(builder, minPath, "$lte", eqPredicate);
        appendRangePredicate(builder, maxPath, "$gte", eqPredicate);
        return true;
    }

    if (hasMaxPredicate) {
        builder->append(maxPath, maxBuilder.obj());
    }
    if (hasMinPredicate) {
        builder->append(minPath, minBuilder.obj());
    }
    return hasPredicate;
}

bool translateMetaPath(StringData field, StringData metaField, std::string* out) {
    if (metaField.empty()) {
        return false;
    }

    if (field == metaField) {
        *out = "meta";
        return true;
    }

    const std::string prefix = str::stream() << metaField << ".";
    if (field.startsWith(prefix)) {
        *out = "meta." + field.substr(prefix.size()).toString();
        return true;
    }

    return false;
}

BSONObj makeBucketMatchPredicate(const CollectionOptions& options, const BSONObj& userMatch) {
    invariant(options.timeseries);
    const auto& tsOptions = *options.timeseries;

    BSONObjBuilder bucketMatch;
    bool hasBucketPredicate = false;

    BSONForEach(predicate, userMatch) {
        const auto field = predicate.fieldNameStringData();
        if (field == tsOptions.timeField) {
            hasBucketPredicate =
                appendTimePredicate(&bucketMatch, tsOptions.timeField, predicate) ||
                hasBucketPredicate;
            continue;
        }

        std::string metaPath;
        if (translateMetaPath(field, tsOptions.metaField, &metaPath)) {
            bucketMatch.appendAs(predicate, metaPath);
            hasBucketPredicate = true;
        }
    }

    if (!hasBucketPredicate) {
        return BSONObj();
    }

    return bucketMatch.obj();
}

bool firstStageIsMatch(const std::vector<BSONObj>& userPipeline, BSONObj* matchSpec) {
    if (userPipeline.empty()) {
        return false;
    }

    const auto firstStage = userPipeline.front();
    const auto firstElem = firstStage.firstElement();
    if (firstElem.fieldNameStringData() != "$match" || firstElem.type() != mongo::Object) {
        return false;
    }

    *matchSpec = firstElem.Obj();
    return true;
}

}  // namespace

std::vector<BSONObj> makeBucketPipeline(const CollectionOptions& options,
                                        const std::vector<BSONObj>& userPipeline) {
    invariant(options.timeseries);

    BSONObjBuilder unpackSpec;
    unpackSpec.append("timeField", options.timeseries->timeField);
    if (options.timeseries->hasMetaField()) {
        unpackSpec.append("metaField", options.timeseries->metaField);
    }

    std::vector<BSONObj> translated;
    translated.reserve(userPipeline.size() + 2);

    BSONObj firstMatch;
    if (firstStageIsMatch(userPipeline, &firstMatch)) {
        const auto bucketMatch = makeBucketMatchPredicate(options, firstMatch);
        if (!bucketMatch.isEmpty()) {
            translated.push_back(BSON("$match" << bucketMatch));
        }
    }

    translated.push_back(BSON("$_internalUnpackBucket" << unpackSpec.obj()));
    for (const auto& stage : userPipeline) {
        translated.push_back(stage.getOwned());
    }
    return translated;
}

AggregationRequest makeBucketAggregationRequest(const NamespaceString& bucketNss,
                                                const CollectionOptions& options,
                                                const AggregationRequest& request) {
    AggregationRequest bucketRequest(bucketNss,
                                     makeBucketPipeline(options, request.getPipeline()));
    bucketRequest.setBatchSize(request.getBatchSize());
    bucketRequest.setCollation(request.getCollation());
    bucketRequest.setHint(request.getHint());
    bucketRequest.setLet(request.getLet());
    bucketRequest.setRuntimeConstants(request.getRuntimeConstants());
    bucketRequest.setComment(request.getComment());
    bucketRequest.setExplain(request.getExplain());
    bucketRequest.setAllowDiskUse(request.shouldAllowDiskUse());
    bucketRequest.setFromMongos(request.isFromMongos());
    bucketRequest.setNeedsMerge(request.needsMerge());
    bucketRequest.setBypassDocumentValidation(request.shouldBypassDocumentValidation());
    bucketRequest.setMaxTimeMS(request.getMaxTimeMS());
    bucketRequest.setReadConcern(request.getReadConcern());
    bucketRequest.setUnwrappedReadPref(request.getUnwrappedReadPref());
    return bucketRequest;
}

}  // namespace timeseries
}  // namespace mongo
