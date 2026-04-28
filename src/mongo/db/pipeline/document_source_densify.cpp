/**
 * Copyright (C) 2026 EloqData Inc.
 *
 * This program is free software: you can redistribute it and/or  modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mongo/platform/basic.h"

#include <cmath>
#include <deque>
#include <limits>

#include "mongo/db/jsobj.h"
#include "mongo/db/pipeline/document.h"
#include "mongo/db/pipeline/document_source.h"
#include "mongo/db/pipeline/field_path.h"
#include "mongo/db/pipeline/lite_parsed_document_source.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {
namespace {

class DocumentSourceDensify final : public DocumentSource {
public:
    static constexpr StringData kStageName = "$densify"_sd;
    enum class BoundsMode { kExplicit, kFull, kPartition };

    DocumentSourceDensify(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                          FieldPath field,
                          double step,
                          double lower,
                          double upper,
                          BoundsMode boundsMode,
                          std::vector<FieldPath> partitionByFields)
        : DocumentSource(expCtx),
          _field(std::move(field)),
          _step(step),
          _lower(lower),
          _upper(upper),
          _boundsMode(boundsMode),
          _partitionByFields(std::move(partitionByFields)),
          _nextValue(lower) {}

    GetNextResult getNext() final {
        pExpCtx->checkForInterrupt();

        if (_boundsMode != BoundsMode::kExplicit) {
            if (!_bufferedInitialized) {
                auto initResult = initializeBufferedOutput();
                if (!initResult.isEOF()) {
                    return initResult;
                }
            }
            if (_pending.empty()) {
                return GetNextResult::makeEOF();
            }
            auto doc = std::move(_pending.front());
            _pending.pop_front();
            return GetNextResult(std::move(doc));
        }

        while (true) {
            if (!_pending.empty()) {
                auto doc = std::move(_pending.front());
                _pending.pop_front();
                return GetNextResult(std::move(doc));
            }

            if (_inputExhausted) {
                if (!_partitionByFields.empty() && !_havePartition) {
                    return GetNextResult::makeEOF();
                }
                if (canGenerateNext()) {
                    return GetNextResult(makeSyntheticDocument(_nextValue));
                }
                return GetNextResult::makeEOF();
            }

            Document doc;
            Value partitionKey;
            if (hasStashedDocument()) {
                doc = std::move(*_stashedDocument);
                partitionKey = std::move(_stashedPartitionKey);
                _stashedDocument = boost::none;
                startPartition(partitionKey);
            } else {
                auto next = pSource->getNext();
                if (next.isPaused()) {
                    return next;
                }
                if (next.isEOF()) {
                    _inputExhausted = true;
                    continue;
                }

                doc = next.releaseDocument();
                partitionKey = makePartitionKey(doc);
            }

            if (isNewPartition(partitionKey)) {
                if (!_havePartition) {
                    startPartition(partitionKey);
                } else if (canGenerateNext()) {
                    _stashedDocument = std::move(doc);
                    _stashedPartitionKey = partitionKey;
                    return GetNextResult(makeSyntheticDocument(_nextValue));
                } else {
                    startPartition(partitionKey);
                }
            }

            double current = extractNumericField(doc);
            if (_haveLastInput) {
                uassert(6789103,
                        "$densify requires input documents to be sorted ascending",
                        current >= _lastInput);
            }
            _haveLastInput = true;
            _lastInput = current;

            if (current >= _lower) {
                while (canGenerateNext() && _nextValue < current) {
                    _pending.push_back(makeSyntheticDocument(_nextValue));
                }
                if (canGenerateNext() && nearlyEqual(_nextValue, current)) {
                    _nextValue += _step;
                }
            }

            _pending.push_back(std::move(doc));
        }
    }

    const char* getSourceName() const final {
        return kStageName.rawData();
    }

    Value serialize(boost::optional<ExplainOptions::Verbosity> explain = boost::none) const final {
        MutableDocument range;
        range["step"] = numericValue(_step);
        switch (_boundsMode) {
            case BoundsMode::kExplicit:
                range["bounds"] = Value(std::vector<Value>{numericValue(_lower), numericValue(_upper)});
                break;
            case BoundsMode::kFull:
                range["bounds"] = Value("full"_sd);
                break;
            case BoundsMode::kPartition:
                range["bounds"] = Value("partition"_sd);
                break;
        }

        MutableDocument spec;
        spec["field"] = Value(_field.fullPath());
        if (!_partitionByFields.empty()) {
            std::vector<Value> fields;
            for (const auto& field : _partitionByFields) {
                fields.push_back(Value(field.fullPath()));
            }
            spec["partitionByFields"] = Value(std::move(fields));
        }
        spec["range"] = range.freezeToValue();
        return Value(Document{{kStageName, spec.freezeToValue()}});
    }

    StageConstraints constraints(Pipeline::SplitState pipeState) const final {
        return StageConstraints(StreamType::kStreaming,
                                PositionRequirement::kNone,
                                HostTypeRequirement::kNone,
                                DiskUseRequirement::kNoDiskUse,
                                FacetRequirement::kAllowed,
                                TransactionRequirement::kAllowed);
    }

    static boost::intrusive_ptr<DocumentSource> createFromBson(
        BSONElement elem, const boost::intrusive_ptr<ExpressionContext>& expCtx) {
        uassert(6789104,
                "$densify requires an object specification",
                elem.type() == BSONType::Object);

        BSONObj spec = elem.Obj();
        BSONElement fieldElem = spec["field"];
        BSONElement rangeElem = spec["range"];
        uassert(6789105,
                "$densify requires a string 'field'",
                fieldElem.type() == BSONType::String);
        uassert(6789106,
                "$densify requires an object 'range'",
                rangeElem.type() == BSONType::Object);
        std::vector<FieldPath> partitionByFields;
        if (auto partitionElem = spec["partitionByFields"]) {
            uassert(6789107,
                    "$densify partitionByFields must be an array of strings",
                    partitionElem.type() == BSONType::Array);
            for (auto&& fieldElem : partitionElem.Obj()) {
                uassert(6789116,
                        "$densify partitionByFields entries must be strings",
                        fieldElem.type() == BSONType::String);
                partitionByFields.emplace_back(fieldElem.str());
            }
        }

        BSONObj range = rangeElem.Obj();
        BSONElement stepElem = range["step"];
        BSONElement boundsElem = range["bounds"];
        uassert(6789108, "$densify requires a numeric 'range.step'", stepElem.isNumber());
        double step = stepElem.numberDouble();
        uassert(6789109, "$densify requires 'range.step' to be positive", step > 0);
        uassert(6789110,
                "$densify date units are not supported in this compatibility checkpoint",
                range["unit"].eoo());
        BoundsMode boundsMode = BoundsMode::kExplicit;
        std::vector<double> bounds{0, 0};
        if (boundsElem.type() == BSONType::Array) {
            bounds.clear();
            for (auto&& bound : boundsElem.Obj()) {
                uassert(6789112, "$densify bounds must be numeric", bound.isNumber());
                bounds.push_back(bound.numberDouble());
            }
            uassert(6789113, "$densify requires exactly two bounds", bounds.size() == 2);
            uassert(6789114, "$densify lower bound must be <= upper bound", bounds[0] <= bounds[1]);
        } else {
            uassert(6789111,
                    "$densify bounds must be an explicit numeric array, 'full', or 'partition'",
                    boundsElem.type() == BSONType::String);
            auto boundsString = boundsElem.valueStringData();
            if (boundsString == "full"_sd) {
                boundsMode = BoundsMode::kFull;
            } else if (boundsString == "partition"_sd) {
                boundsMode = BoundsMode::kPartition;
            } else {
                uasserted(6789117,
                          str::stream() << "unsupported $densify bounds mode '" << boundsString
                                        << "'");
            }
        }

        return new DocumentSourceDensify(expCtx,
                                         FieldPath(fieldElem.str()),
                                         step,
                                         bounds[0],
                                         bounds[1],
                                         boundsMode,
                                         std::move(partitionByFields));
    }

private:
    struct BufferedDoc {
        Document doc;
        Value partitionKey;
        double value;
    };

    bool hasStashedDocument() const {
        return static_cast<bool>(_stashedDocument);
    }

    bool canGenerateNext() {
        if (_nextValue > _upper || nearlyEqual(_nextValue, _upper + _step)) {
            return false;
        }
        return _nextValue <= _upper || nearlyEqual(_nextValue, _upper);
    }

    Document makeSyntheticDocument(double value) {
        MutableDocument doc;
        addPartitionFields(doc);
        doc.setNestedField(_field, numericValue(value));
        _nextValue += _step;
        return doc.freeze();
    }

    Document makeSyntheticDocument(double value, const Value& partitionKey) const {
        MutableDocument doc;
        addPartitionFields(doc, partitionKey);
        doc.setNestedField(_field, numericValue(value));
        return doc.freeze();
    }

    double extractNumericField(const Document& doc) const {
        auto value = doc.getNestedField(_field);
        uassert(6789115, "$densify field must be present and numeric", value.numeric());
        return value.coerceToDouble();
    }

    Value makePartitionKey(const Document& doc) const {
        if (_partitionByFields.empty()) {
            return Value();
        }

        std::vector<Value> key;
        key.reserve(_partitionByFields.size());
        for (const auto& field : _partitionByFields) {
            auto value = doc.getNestedField(field);
            key.push_back(value.missing() ? Value(BSONNULL) : value);
        }
        return Value(std::move(key));
    }

    bool isNewPartition(const Value& partitionKey) const {
        if (_partitionByFields.empty()) {
            return false;
        }
        return !_havePartition ||
            pExpCtx->getValueComparator().compare(partitionKey, _currentPartitionKey) != 0;
    }

    void startPartition(Value partitionKey) {
        _havePartition = true;
        _currentPartitionKey = std::move(partitionKey);
        _nextValue = _lower;
        _haveLastInput = false;
        _lastInput = 0;
    }

    void addPartitionFields(MutableDocument& doc) const {
        if (_partitionByFields.empty()) {
            return;
        }

        const auto& keyValues = _currentPartitionKey.getArray();
        for (size_t i = 0; i < _partitionByFields.size(); ++i) {
            doc.setNestedField(_partitionByFields[i], keyValues[i]);
        }
    }

    void addPartitionFields(MutableDocument& doc, const Value& partitionKey) const {
        if (_partitionByFields.empty()) {
            return;
        }

        const auto& keyValues = partitionKey.getArray();
        for (size_t i = 0; i < _partitionByFields.size(); ++i) {
            doc.setNestedField(_partitionByFields[i], keyValues[i]);
        }
    }

    GetNextResult initializeBufferedOutput() {
        while (!_bufferedInputExhausted) {
            auto next = pSource->getNext();
            if (next.isPaused()) {
                return next;
            }
            if (next.isEOF()) {
                _bufferedInputExhausted = true;
                break;
            }

            auto doc = next.releaseDocument();
            _bufferedInput.push_back({doc, makePartitionKey(doc), extractNumericField(doc)});
        }

        if (_bufferedInput.empty()) {
            _bufferedInitialized = true;
            return GetNextResult::makeEOF();
        }

        double globalLower = _bufferedInput.front().value;
        double globalUpper = _bufferedInput.front().value;
        for (const auto& item : _bufferedInput) {
            globalLower = std::min(globalLower, item.value);
            globalUpper = std::max(globalUpper, item.value);
        }

        for (size_t start = 0; start < _bufferedInput.size();) {
            size_t end = start + 1;
            while (end < _bufferedInput.size() && samePartition(_bufferedInput[start].partitionKey,
                                                                _bufferedInput[end].partitionKey)) {
                ++end;
            }

            double lower = _boundsMode == BoundsMode::kFull ? globalLower : _bufferedInput[start].value;
            double upper = _boundsMode == BoundsMode::kFull ? globalUpper : _bufferedInput[start].value;
            for (size_t i = start; i < end; ++i) {
                lower = std::min(lower, _bufferedInput[i].value);
                upper = std::max(upper, _bufferedInput[i].value);
                if (i > start) {
                    uassert(6789318,
                            "$densify requires input documents to be sorted ascending",
                            _bufferedInput[i].value >= _bufferedInput[i - 1].value);
                }
            }

            double nextValue = lower;
            for (size_t i = start; i < end; ++i) {
                const auto current = _bufferedInput[i].value;
                while (nextValue <= upper && nextValue < current) {
                    _pending.push_back(
                        makeSyntheticDocument(nextValue, _bufferedInput[start].partitionKey));
                    nextValue += _step;
                }
                if (nextValue <= upper && nearlyEqual(nextValue, current)) {
                    nextValue += _step;
                }
                _pending.push_back(std::move(_bufferedInput[i].doc));
            }
            while (nextValue <= upper || nearlyEqual(nextValue, upper)) {
                _pending.push_back(makeSyntheticDocument(nextValue, _bufferedInput[start].partitionKey));
                nextValue += _step;
            }

            start = end;
        }

        _bufferedInitialized = true;
        return GetNextResult::makeEOF();
    }

    bool samePartition(const Value& left, const Value& right) const {
        if (_partitionByFields.empty()) {
            return true;
        }
        return pExpCtx->getValueComparator().compare(left, right) == 0;
    }

    static bool nearlyEqual(double left, double right) {
        return std::abs(left - right) < 1e-9;
    }

    static Value numericValue(double value) {
        long long asLong = static_cast<long long>(value);
        if (nearlyEqual(value, static_cast<double>(asLong))) {
            if (asLong >= std::numeric_limits<int>::min() &&
                asLong <= std::numeric_limits<int>::max()) {
                return Value(static_cast<int>(asLong));
            }
            return Value(asLong);
        }
        return Value(value);
    }

    FieldPath _field;
    double _step;
    double _lower;
    double _upper;
    BoundsMode _boundsMode;
    std::vector<FieldPath> _partitionByFields;
    double _nextValue;
    bool _inputExhausted = false;
    bool _havePartition = false;
    Value _currentPartitionKey;
    bool _haveLastInput = false;
    double _lastInput = 0;
    std::deque<Document> _pending;
    boost::optional<Document> _stashedDocument;
    Value _stashedPartitionKey;
    bool _bufferedInitialized = false;
    bool _bufferedInputExhausted = false;
    std::vector<BufferedDoc> _bufferedInput;
};

constexpr StringData DocumentSourceDensify::kStageName;

}  // namespace

REGISTER_DOCUMENT_SOURCE(densify,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourceDensify::createFromBson);

}  // namespace mongo
