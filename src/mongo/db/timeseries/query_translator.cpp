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

#include <set>

namespace mongo {
namespace timeseries {
namespace {

std::string makeControlPath(StringData minOrMax, StringData timeField) {
    return str::stream() << "control." << minOrMax << "." << timeField;
}

std::string makeFieldControlPath(StringData minOrMax, StringData field) {
    return str::stream() << "control." << minOrMax << "." << field;
}

void appendRangePredicate(BSONObjBuilder* builder,
                          StringData path,
                          StringData opName,
                          const BSONElement& value) {
    BSONObjBuilder predicate(builder->subobjStart(path));
    predicate.appendAs(value, opName);
    predicate.doneFast();
}

bool appendTimeInPredicate(BSONObjBuilder* builder,
                           StringData timeField,
                           const BSONElement& predicate);

bool appendMeasurementInPredicate(BSONObjBuilder* builder,
                                  StringData field,
                                  const BSONElement& predicate);

bool appendMeasurementPredicate(BSONObjBuilder* builder,
                                StringData field,
                                const BSONElement& predicate) {
    const auto minPath = makeFieldControlPath("min", field);
    const auto maxPath = makeFieldControlPath("max", field);

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

        if (opName == "$gt" || opName == "$gte") {
            maxBuilder.appendAs(op, opName);
            hasMaxPredicate = true;
            hasPredicate = true;
        } else if (opName == "$lt" || opName == "$lte") {
            minBuilder.appendAs(op, opName);
            hasMinPredicate = true;
            hasPredicate = true;
        } else if (opName == "$eq") {
            eqPredicate = op;
            hasEqPredicate = true;
            hasPredicate = true;
        } else if (opName == "$in") {
            return appendMeasurementInPredicate(builder, field, op);
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
        } else if (opName == "$in") {
            return appendTimeInPredicate(builder, timeField, op);
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

bool appendTimeInPredicate(BSONObjBuilder* builder,
                           StringData timeField,
                           const BSONElement& predicate) {
    if (predicate.type() != mongo::Array) {
        return false;
    }

    const auto minPath = makeControlPath("min", timeField);
    const auto maxPath = makeControlPath("max", timeField);

    BSONArrayBuilder choices;
    bool hasChoice = false;
    BSONForEach(value, predicate.Obj()) {
        if (value.type() != mongo::Date) {
            return false;
        }

        BSONObjBuilder choice;
        appendRangePredicate(&choice, minPath, "$lte", value);
        appendRangePredicate(&choice, maxPath, "$gte", value);
        choices.append(choice.obj());
        hasChoice = true;
    }

    if (!hasChoice) {
        return false;
    }

    builder->append("$or", choices.arr());
    return true;
}

bool appendMeasurementInPredicate(BSONObjBuilder* builder,
                                  StringData field,
                                  const BSONElement& predicate) {
    if (predicate.type() != mongo::Array) {
        return false;
    }

    const auto minPath = makeFieldControlPath("min", field);
    const auto maxPath = makeFieldControlPath("max", field);

    BSONArrayBuilder choices;
    bool hasChoice = false;
    BSONForEach(value, predicate.Obj()) {
        if (!value.isNumber()) {
            return false;
        }

        BSONObjBuilder choice;
        appendRangePredicate(&choice, minPath, "$lte", value);
        appendRangePredicate(&choice, maxPath, "$gte", value);
        choices.append(choice.obj());
        hasChoice = true;
    }

    if (!hasChoice) {
        return false;
    }

    builder->append("$or", choices.arr());
    return true;
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

BSONObj makeBucketMatchPredicate(const CollectionOptions& options, const BSONObj& userMatch);

BSONObj makeExactBucketMetaMatchPredicate(const CollectionOptions& options,
                                          const BSONObj& userMatch);

bool appendLogicalPredicate(BSONObjBuilder* builder,
                            const CollectionOptions& options,
                            const BSONElement& predicate) {
    const auto opName = predicate.fieldNameStringData();
    if (predicate.type() != mongo::Array) {
        return false;
    }

    BSONArrayBuilder translatedChildren;
    bool hasChildPredicate = false;

    BSONForEach(child, predicate.Obj()) {
        if (child.type() != mongo::Object) {
            if (opName == "$or") {
                return false;
            }
            continue;
        }

        const auto translatedChild = makeBucketMatchPredicate(options, child.Obj());
        if (translatedChild.isEmpty()) {
            if (opName == "$or") {
                return false;
            }
            continue;
        }

        translatedChildren.append(translatedChild);
        hasChildPredicate = true;
    }

    if (!hasChildPredicate) {
        return false;
    }

    builder->append(opName, translatedChildren.arr());
    return true;
}

BSONObj makeBucketMatchPredicate(const CollectionOptions& options, const BSONObj& userMatch) {
    invariant(options.timeseries);
    const auto& tsOptions = *options.timeseries;

    BSONObjBuilder bucketMatch;
    bool hasBucketPredicate = false;

    BSONForEach(predicate, userMatch) {
        const auto field = predicate.fieldNameStringData();
        if (field == "$and" || field == "$or") {
            hasBucketPredicate =
                appendLogicalPredicate(&bucketMatch, options, predicate) || hasBucketPredicate;
            continue;
        }

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
            continue;
        }

        hasBucketPredicate =
            appendMeasurementPredicate(&bucketMatch, field, predicate) || hasBucketPredicate;
    }

    if (!hasBucketPredicate) {
        return BSONObj();
    }

    return bucketMatch.obj();
}

BSONObj makeExactBucketMetaMatchPredicate(const CollectionOptions& options,
                                          const BSONObj& userMatch) {
    invariant(options.timeseries);
    const auto& tsOptions = *options.timeseries;

    BSONObjBuilder bucketMatch;
    bool hasBucketPredicate = false;

    BSONForEach(predicate, userMatch) {
        const auto field = predicate.fieldNameStringData();
        if (field == "$and" || field == "$or") {
            if (predicate.type() != mongo::Array) {
                return BSONObj();
            }

            BSONArrayBuilder translatedChildren;
            bool hasChildPredicate = false;
            BSONForEach(child, predicate.Obj()) {
                if (child.type() != mongo::Object) {
                    return BSONObj();
                }

                const auto translatedChild = makeExactBucketMetaMatchPredicate(options, child.Obj());
                if (translatedChild.isEmpty()) {
                    return BSONObj();
                }

                translatedChildren.append(translatedChild);
                hasChildPredicate = true;
            }

            if (!hasChildPredicate) {
                return BSONObj();
            }

            bucketMatch.append(field, translatedChildren.arr());
            hasBucketPredicate = true;
            continue;
        }

        std::string metaPath;
        if (!translateMetaPath(field, tsOptions.metaField, &metaPath)) {
            return BSONObj();
        }

        bucketMatch.appendAs(predicate, metaPath);
        hasBucketPredicate = true;
    }

    if (!hasBucketPredicate) {
        return BSONObj();
    }

    return bucketMatch.obj();
}

BSONObj makeBucketSortPredicate(const CollectionOptions& options, const BSONObj& userSort) {
    invariant(options.timeseries);
    const auto& tsOptions = *options.timeseries;

    BSONObjBuilder bucketSort;
    BSONForEach(sortField, userSort) {
        const auto field = sortField.fieldNameStringData();
        if (field == tsOptions.timeField) {
            const auto controlField = sortField.numberInt() < 0 ? "max" : "min";
            bucketSort.appendAs(sortField, makeControlPath(controlField, tsOptions.timeField));
            continue;
        }

        std::string metaPath;
        if (translateMetaPath(field, tsOptions.metaField, &metaPath)) {
            bucketSort.appendAs(sortField, metaPath);
            continue;
        }

        return BSONObj();
    }

    return bucketSort.obj();
}

bool appendLeadingSortPushdown(const CollectionOptions& options,
                               const BSONObj& stage,
                               std::vector<BSONObj>* translated) {
    const auto firstElem = stage.firstElement();
    if (firstElem.fieldNameStringData() != "$sort" || firstElem.type() != mongo::Object) {
        return false;
    }

    const auto bucketSort = makeBucketSortPredicate(options, firstElem.Obj());
    if (bucketSort.isEmpty()) {
        return false;
    }

    translated->push_back(BSON("$sort" << bucketSort));
    return true;
}

bool appendLeadingLimitPushdown(const BSONObj& stage, std::vector<BSONObj>* translated) {
    const auto firstElem = stage.firstElement();
    if (firstElem.fieldNameStringData() != "$limit" || !firstElem.isNumber()) {
        return false;
    }

    translated->push_back(stage.getOwned());
    return true;
}

bool isTruthyProjectionValue(const BSONElement& value) {
    if (value.type() == mongo::Bool) {
        return value.booleanSafe();
    }
    return value.isNumber() && value.numberInt() == 1;
}

bool isFalsyProjectionValue(const BSONElement& value) {
    if (value.type() == mongo::Bool) {
        return !value.booleanSafe();
    }
    return value.isNumber() && value.numberInt() == 0;
}

void addBucketProjectionField(const TimeseriesOptions& tsOptions,
                              StringData field,
                              BSONObjBuilder* bucketProject) {
    std::string metaPath;
    if (translateMetaPath(field, tsOptions.metaField, &metaPath)) {
        bucketProject->append(metaPath, 1);
    } else {
        bucketProject->append(str::stream() << "data." << field, 1);
    }
}

void collectMatchDependencies(const BSONObj& match, std::set<std::string>* fields) {
    BSONForEach(predicate, match) {
        const StringData field = predicate.fieldNameStringData();
        if (field == "$and" || field == "$or") {
            if (predicate.type() != mongo::Array) {
                continue;
            }

            BSONForEach(child, predicate.Obj()) {
                if (child.type() == mongo::Object) {
                    collectMatchDependencies(child.Obj(), fields);
                }
            }
            continue;
        }

        if (!field.startsWith("$")) {
            fields->insert(field.toString());
        }
    }
}

void collectSortDependencies(const BSONObj& sort, std::set<std::string>* fields) {
    BSONForEach(sortField, sort) {
        const StringData field = sortField.fieldNameStringData();
        if (!field.startsWith("$")) {
            fields->insert(field.toString());
        }
    }
}

bool logicalFieldCovers(StringData projectedField, StringData requiredField) {
    if (projectedField == requiredField) {
        return true;
    }

    const std::string prefix = str::stream() << projectedField << ".";
    return requiredField.startsWith(prefix);
}

bool isCoveredByProjectedFields(StringData requiredField, const std::set<std::string>& projected) {
    for (const auto& projectedField : projected) {
        if (logicalFieldCovers(projectedField, requiredField)) {
            return true;
        }
    }
    return false;
}

bool appendLeadingProjectPushdown(const CollectionOptions& options,
                                  const BSONObj& stage,
                                  const std::set<std::string>& requiredFields,
                                  std::vector<BSONObj>* translated) {
    invariant(options.timeseries);
    const auto& tsOptions = *options.timeseries;

    const auto firstElem = stage.firstElement();
    if (firstElem.fieldNameStringData() != "$project" || firstElem.type() != mongo::Object) {
        return false;
    }

    BSONObjBuilder bucketProject;
    bucketProject.append("_id", 0);
    bucketProject.append("control.count", 1);
    bool hasFieldProjection = false;
    std::set<std::string> projectedFields;

    BSONForEach(projection, firstElem.Obj()) {
        const auto field = projection.fieldNameStringData();
        if (field == "_id") {
            if (!isFalsyProjectionValue(projection)) {
                return false;
            }
            continue;
        }

        if (!isTruthyProjectionValue(projection)) {
            return false;
        }

        addBucketProjectionField(tsOptions, field, &bucketProject);
        projectedFields.insert(field.toString());
        hasFieldProjection = true;
    }

    if (!hasFieldProjection) {
        return false;
    }

    for (const auto& field : requiredFields) {
        if (isCoveredByProjectedFields(field, projectedFields)) {
            continue;
        }
        addBucketProjectionField(tsOptions, field, &bucketProject);
    }

    translated->push_back(BSON("$project" << bucketProject.obj()));
    return true;
}

bool containsExpressionReference(const BSONElement& value) {
    if (value.type() == mongo::String) {
        return StringData(value.String()).startsWith("$");
    }

    if (value.type() != mongo::Object && value.type() != mongo::Array) {
        return false;
    }

    BSONForEach(child, value.Obj()) {
        if (StringData(child.fieldName()).startsWith("$") || containsExpressionReference(child)) {
            return true;
        }
    }
    return false;
}

bool appendLeadingMetaAddFieldsPushdown(const CollectionOptions& options,
                                        const BSONObj& stage,
                                        std::vector<BSONObj>* translated) {
    invariant(options.timeseries);
    const auto& tsOptions = *options.timeseries;
    if (!tsOptions.hasMetaField()) {
        return false;
    }

    const auto firstElem = stage.firstElement();
    const auto stageName = firstElem.fieldNameStringData();
    if ((stageName != "$addFields" && stageName != "$set") || firstElem.type() != mongo::Object) {
        return false;
    }

    BSONObjBuilder bucketFields;
    bool hasField = false;
    BSONForEach(field, firstElem.Obj()) {
        std::string metaPath;
        if (!translateMetaPath(field.fieldNameStringData(), tsOptions.metaField, &metaPath) ||
            containsExpressionReference(field)) {
            return false;
        }

        bucketFields.appendAs(field, metaPath);
        hasField = true;
    }

    if (!hasField) {
        return false;
    }

    BSONObjBuilder stageBuilder;
    stageBuilder.append(stageName, bucketFields.obj());
    translated->push_back(stageBuilder.obj());
    return true;
}

bool appendWholeCollectionGroupRewrite(const CollectionOptions& options,
                                       const BSONObj& stage,
                                       std::vector<BSONObj>* translated) {
    invariant(options.timeseries);
    const auto& tsOptions = *options.timeseries;

    const auto firstElem = stage.firstElement();
    if (firstElem.fieldNameStringData() != "$group" || firstElem.type() != mongo::Object) {
        return false;
    }

    const auto groupSpec = firstElem.Obj();
    const auto idElem = groupSpec["_id"];
    if (idElem.eoo()) {
        return false;
    }

    BSONObjBuilder bucketGroup;
    if (idElem.type() == mongo::jstNULL) {
        bucketGroup.appendNull("_id");
    } else if (idElem.type() == mongo::String) {
        const StringData idExpression(idElem.String());
        if (!idExpression.startsWith("$") || idExpression.startsWith("$$")) {
            return false;
        }

        const auto logicalField = idExpression.substr(1);
        std::string metaPath;
        if (!translateMetaPath(logicalField, tsOptions.metaField, &metaPath)) {
            return false;
        }
        const std::string bucketIdPath = str::stream() << "$" << metaPath;
        bucketGroup.append("_id", bucketIdPath);
    } else {
        return false;
    }

    bool hasAccumulator = false;

    BSONForEach(field, groupSpec) {
        const auto outputField = field.fieldNameStringData();
        if (outputField == "_id") {
            continue;
        }

        if (field.type() != mongo::Object) {
            return false;
        }

        const auto accumulator = field.Obj();
        if (accumulator.nFields() != 1) {
            return false;
        }

        const auto op = accumulator.firstElement();
        const auto opName = op.fieldNameStringData();
        if (opName == "$sum" && op.isNumber() && op.numberInt() == 1) {
            bucketGroup.append(outputField, BSON("$sum" << "$control.count"));
            hasAccumulator = true;
            continue;
        }

        const std::string logicalTimePath = str::stream() << "$" << tsOptions.timeField;
        if ((opName == "$min" || opName == "$max") && op.type() == mongo::String &&
            op.String() == logicalTimePath) {
            const auto controlField = opName == "$min" ? "min" : "max";
            const std::string inputPath =
                str::stream() << "$" << makeControlPath(controlField, tsOptions.timeField);
            bucketGroup.append(outputField, BSON(opName << inputPath));
            hasAccumulator = true;
            continue;
        }

        if ((opName == "$min" || opName == "$max") && op.type() == mongo::String) {
            const StringData expression(op.String());
            if (!expression.startsWith("$") || expression.startsWith("$$")) {
                return false;
            }

            const auto logicalField = expression.substr(1);
            std::string metaPath;
            if (translateMetaPath(logicalField, tsOptions.metaField, &metaPath)) {
                const std::string inputPath = str::stream() << "$" << metaPath;
                bucketGroup.append(outputField, BSON(opName << inputPath));
                hasAccumulator = true;
                continue;
            }

            if (logicalField == "_id") {
                return false;
            }

            const auto controlField = opName == "$min" ? "min" : "max";
            const std::string inputPath =
                str::stream() << "$" << makeFieldControlPath(controlField, logicalField);
            bucketGroup.append(outputField, BSON(opName << inputPath));
            hasAccumulator = true;
            continue;
        }

        if (opName == "$addToSet" && op.type() == mongo::String) {
            const StringData expression(op.String());
            if (!expression.startsWith("$") || expression.startsWith("$$")) {
                return false;
            }

            std::string metaPath;
            if (!translateMetaPath(expression.substr(1), tsOptions.metaField, &metaPath)) {
                return false;
            }

            const std::string inputPath = str::stream() << "$" << metaPath;
            bucketGroup.append(outputField, BSON("$addToSet" << inputPath));
            hasAccumulator = true;
            continue;
        }

        return false;
    }

    if (!hasAccumulator) {
        return false;
    }

    translated->push_back(BSON("$group" << bucketGroup.obj()));
    return true;
}

bool appendCountRewrite(const BSONObj& stage, std::vector<BSONObj>* translated) {
    const auto firstElem = stage.firstElement();
    if (firstElem.fieldNameStringData() != "$count" || firstElem.type() != mongo::String) {
        return false;
    }

    const auto countField = firstElem.String();
    if (countField.empty() || countField.find('.') != std::string::npos ||
        countField[0] == '$') {
        return false;
    }

    translated->push_back(
        BSON("$group" << BSON("_id" << BSONNULL << countField << BSON("$sum"
                                                                       << "$control.count"))));
    translated->push_back(BSON("$project" << BSON("_id" << 0 << countField << 1)));
    return true;
}

bool appendMetaSortByCountRewrite(const CollectionOptions& options,
                                  const BSONObj& stage,
                                  std::vector<BSONObj>* translated) {
    invariant(options.timeseries);
    const auto& tsOptions = *options.timeseries;

    const auto firstElem = stage.firstElement();
    if (firstElem.fieldNameStringData() != "$sortByCount" ||
        firstElem.type() != mongo::String) {
        return false;
    }

    const StringData expression(firstElem.String());
    if (!expression.startsWith("$") || expression.startsWith("$$")) {
        return false;
    }

    std::string metaPath;
    if (!translateMetaPath(expression.substr(1), tsOptions.metaField, &metaPath)) {
        return false;
    }

    const std::string bucketIdPath = str::stream() << "$" << metaPath;
    translated->push_back(
        BSON("$group" << BSON("_id" << bucketIdPath << "count" << BSON("$sum"
                                                                        << "$control.count"))));
    translated->push_back(BSON("$sort" << BSON("count" << -1)));
    return true;
}

BSONObj makeMetaDistinctGroupRewrite(const CollectionOptions& options,
                                     const BSONObj& unwindStage,
                                     const BSONObj& groupStage) {
    invariant(options.timeseries);
    const auto& tsOptions = *options.timeseries;

    const auto unwindElem = unwindStage.firstElement();
    if (unwindElem.fieldNameStringData() != "$unwind" || unwindElem.type() != mongo::Object) {
        return BSONObj();
    }

    const auto unwindSpec = unwindElem.Obj();
    if (unwindSpec.nFields() != 2 ||
        !unwindSpec["preserveNullAndEmptyArrays"].trueValue() ||
        unwindSpec["path"].type() != mongo::String) {
        return BSONObj();
    }

    const StringData unwindPath(unwindSpec["path"].String());
    if (!unwindPath.startsWith("$") || unwindPath.startsWith("$$")) {
        return BSONObj();
    }

    const auto groupElem = groupStage.firstElement();
    if (groupElem.fieldNameStringData() != "$group" || groupElem.type() != mongo::Object) {
        return BSONObj();
    }

    const auto groupSpec = groupElem.Obj();
    if (groupSpec.nFields() != 2 || groupSpec["_id"].type() != mongo::jstNULL) {
        return BSONObj();
    }

    BSONElement distinctElem;
    BSONForEach(field, groupSpec) {
        if (field.fieldNameStringData() != "_id") {
            distinctElem = field;
            break;
        }
    }

    if (distinctElem.eoo() || distinctElem.type() != mongo::Object) {
        return BSONObj();
    }

    const auto distinctSpec = distinctElem.Obj();
    if (distinctSpec.nFields() != 1) {
        return BSONObj();
    }

    const auto accumulator = distinctSpec.firstElement();
    if (accumulator.fieldNameStringData() != "$addToSet" ||
        accumulator.type() != mongo::String ||
        accumulator.String() != unwindPath) {
        return BSONObj();
    }

    std::string metaPath;
    if (!translateMetaPath(unwindPath.substr(1), tsOptions.metaField, &metaPath)) {
        return BSONObj();
    }

    const std::string bucketMetaPath = str::stream() << "$" << metaPath;
    BSONObjBuilder bucketGroup;
    bucketGroup.appendNull("_id");
    bucketGroup.append(distinctElem.fieldNameStringData(), BSON("$addToSet" << bucketMetaPath));
    return BSON("$group" << bucketGroup.obj());
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

    if (userPipeline.size() == 1 &&
        appendWholeCollectionGroupRewrite(options, userPipeline.front(), &translated)) {
        return translated;
    }

    if (userPipeline.size() == 1 && appendCountRewrite(userPipeline.front(), &translated)) {
        return translated;
    }

    if (userPipeline.size() == 1 &&
        appendMetaSortByCountRewrite(options, userPipeline.front(), &translated)) {
        return translated;
    }

    if (userPipeline.size() == 2) {
        auto distinctGroup =
            makeMetaDistinctGroupRewrite(options, userPipeline.front(), userPipeline.back());
        if (!distinctGroup.isEmpty()) {
            translated.push_back(distinctGroup);
            return translated;
        }
    }

    if (userPipeline.size() >= 3) {
        auto distinctGroup = makeMetaDistinctGroupRewrite(
            options, userPipeline[userPipeline.size() - 2], userPipeline.back());
        if (!distinctGroup.isEmpty()) {
            std::vector<BSONObj> exactMetaMatches;
            exactMetaMatches.reserve(userPipeline.size() - 2);
            bool hasOnlyExactMetaMatches = true;
            for (size_t i = 0; i + 2 < userPipeline.size(); ++i) {
                const auto firstElem = userPipeline[i].firstElement();
                if (firstElem.fieldNameStringData() != "$match" ||
                    firstElem.type() != mongo::Object) {
                    hasOnlyExactMetaMatches = false;
                    break;
                }

                const auto bucketMatch = makeExactBucketMetaMatchPredicate(options, firstElem.Obj());
                if (bucketMatch.isEmpty()) {
                    hasOnlyExactMetaMatches = false;
                    break;
                }

                exactMetaMatches.push_back(BSON("$match" << bucketMatch));
            }

            if (hasOnlyExactMetaMatches && !exactMetaMatches.empty()) {
                translated.insert(translated.end(), exactMetaMatches.begin(), exactMetaMatches.end());
                translated.push_back(distinctGroup);
                return translated;
            }
        }
    }

    if (userPipeline.size() >= 2 &&
        (appendWholeCollectionGroupRewrite(options, userPipeline.back(), &translated) ||
         appendCountRewrite(userPipeline.back(), &translated) ||
         appendMetaSortByCountRewrite(options, userPipeline.back(), &translated))) {
        std::vector<BSONObj> exactMetaMatches;
        exactMetaMatches.reserve(userPipeline.size() - 1);
        bool hasOnlyExactMetaMatches = true;
        for (size_t i = 0; i + 1 < userPipeline.size(); ++i) {
            const auto firstElem = userPipeline[i].firstElement();
            if (firstElem.fieldNameStringData() != "$match" || firstElem.type() != mongo::Object) {
                hasOnlyExactMetaMatches = false;
                break;
            }

            const auto bucketMatch = makeExactBucketMetaMatchPredicate(options, firstElem.Obj());
            if (bucketMatch.isEmpty()) {
                hasOnlyExactMetaMatches = false;
                break;
            }

            exactMetaMatches.push_back(BSON("$match" << bucketMatch));
        }

        if (hasOnlyExactMetaMatches && !exactMetaMatches.empty()) {
            translated.insert(translated.begin(), exactMetaMatches.begin(), exactMetaMatches.end());
            return translated;
        }

        translated.clear();
    }

    bool lastPushdownWasSort = false;
    std::set<std::string> fieldsRequiredAfterUnpack;
    for (const auto& stage : userPipeline) {
        const auto firstElem = stage.firstElement();
        if (firstElem.fieldNameStringData() == "$match" && firstElem.type() == mongo::Object) {
            const auto bucketMatch = makeBucketMatchPredicate(options, firstElem.Obj());
            if (!bucketMatch.isEmpty()) {
                translated.push_back(BSON("$match" << bucketMatch));
                collectMatchDependencies(firstElem.Obj(), &fieldsRequiredAfterUnpack);
                lastPushdownWasSort = false;
                continue;
            }
        }

        if (appendLeadingSortPushdown(options, stage, &translated)) {
            collectSortDependencies(firstElem.Obj(), &fieldsRequiredAfterUnpack);
            lastPushdownWasSort = true;
            continue;
        }

        if ((translated.empty() || lastPushdownWasSort) &&
            appendLeadingLimitPushdown(stage, &translated)) {
            break;
        }

        if (appendLeadingProjectPushdown(options, stage, fieldsRequiredAfterUnpack, &translated)) {
            break;
        }

        if (translated.empty() && appendLeadingMetaAddFieldsPushdown(options, stage, &translated)) {
            break;
        }

        break;
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
