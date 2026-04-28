/**
 *    Copyright (C) 2026 EloqData Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/pipeline/document_source_internal_unpack_bucket.h"

#include "mongo/db/pipeline/lite_parsed_document_source.h"

namespace mongo {

using boost::intrusive_ptr;

REGISTER_DOCUMENT_SOURCE(_internalUnpackBucket,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourceInternalUnpackBucket::createFromBson);

constexpr StringData DocumentSourceInternalUnpackBucket::kStageName;

DocumentSourceInternalUnpackBucket::DocumentSourceInternalUnpackBucket(
    const intrusive_ptr<ExpressionContext>& expCtx, std::string timeField, std::string metaField)
    : DocumentSource(expCtx), _timeField(std::move(timeField)), _metaField(std::move(metaField)) {}

intrusive_ptr<DocumentSource> DocumentSourceInternalUnpackBucket::createFromBson(
    BSONElement elem, const intrusive_ptr<ExpressionContext>& expCtx) {
    uassert(ErrorCodes::TypeMismatch,
            "$_internalUnpackBucket specification must be an object",
            elem.type() == BSONType::Object);

    auto spec = elem.Obj();
    auto timeFieldElem = spec["timeField"];
    uassert(ErrorCodes::FailedToParse,
            "$_internalUnpackBucket requires a string timeField",
            timeFieldElem.type() == BSONType::String && !timeFieldElem.String().empty());

    std::string metaField;
    auto metaFieldElem = spec["metaField"];
    if (!metaFieldElem.eoo()) {
        uassert(ErrorCodes::TypeMismatch,
                "$_internalUnpackBucket metaField must be a string",
                metaFieldElem.type() == BSONType::String);
        metaField = metaFieldElem.String();
    }

    return new DocumentSourceInternalUnpackBucket(expCtx, timeFieldElem.String(), metaField);
}

DocumentSource::GetNextResult DocumentSourceInternalUnpackBucket::getNext() {
    while (true) {
        pExpCtx->checkForInterrupt();

        if (!_pending.empty()) {
            auto out = _pending.front();
            _pending.pop_front();
            return out;
        }

        auto input = pSource->getNext();
        if (!input.isAdvanced()) {
            return input;
        }

        unpackBucket(input.releaseDocument());
    }
}

void DocumentSourceInternalUnpackBucket::unpackBucket(const Document& bucketDoc) {
    auto bucket = bucketDoc.toBson();
    auto control = bucket["control"];
    auto data = bucket["data"];
    uassert(ErrorCodes::FailedToParse,
            "time-series bucket is missing object control field",
            control.type() == BSONType::Object);
    uassert(ErrorCodes::FailedToParse,
            "time-series bucket is missing object data field",
            data.type() == BSONType::Object);

    auto countElem = control.Obj()["count"];
    uassert(ErrorCodes::FailedToParse,
            "time-series bucket control.count must be numeric",
            countElem.isNumber());

    auto metaElem = bucket["meta"];
    const long long count = countElem.numberLong();
    for (long long i = 0; i < count; ++i) {
        const auto index = std::to_string(i);
        BSONObjBuilder measurement;
        BSONForEach(columnElem, data.Obj()) {
            uassert(ErrorCodes::FailedToParse,
                    "time-series bucket data columns must be objects",
                    columnElem.type() == BSONType::Object);
            auto value = columnElem.Obj()[index];
            if (!value.eoo()) {
                measurement.appendAs(value, columnElem.fieldName());
            }
        }

        if (!_metaField.empty() && !metaElem.eoo()) {
            measurement.appendAs(metaElem, _metaField);
        }

        _pending.emplace_back(measurement.obj());
    }
}

Value DocumentSourceInternalUnpackBucket::serialize(
    boost::optional<ExplainOptions::Verbosity> explain) const {
    MutableDocument spec;
    spec.addField("timeField", Value(_timeField));
    if (!_metaField.empty()) {
        spec.addField("metaField", Value(_metaField));
    }
    return Value(Document{{getSourceName(), spec.freezeToValue()}});
}

}  // namespace mongo
