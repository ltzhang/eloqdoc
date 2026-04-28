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

#include <deque>
#include <cmath>
#include <vector>

#include "mongo/db/jsobj.h"
#include "mongo/db/pipeline/document.h"
#include "mongo/db/pipeline/document_source.h"
#include "mongo/db/pipeline/expression.h"
#include "mongo/db/pipeline/field_path.h"
#include "mongo/db/pipeline/lite_parsed_document_source.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {
namespace {

class DocumentSourceFill final : public DocumentSource {
public:
    static constexpr StringData kStageName = "$fill"_sd;

    enum class RuleType {
        kLiteral,
        kLocf,
        kLinear,
    };

    struct Rule {
        FieldPath field;
        RuleType type;
        Value literal;
        Value lastSeen;
    };

    struct SortField {
        FieldPath field;
        int direction;
    };

    DocumentSourceFill(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                       std::vector<Rule> rules,
                       BSONObj sortBy,
                       std::vector<FieldPath> partitionByFields,
                       boost::intrusive_ptr<Expression> partitionByExpression,
                       std::vector<SortField> sortFields)
        : DocumentSource(expCtx),
          _rules(std::move(rules)),
          _sortBy(sortBy.getOwned()),
          _partitionByFields(std::move(partitionByFields)),
          _partitionByExpression(std::move(partitionByExpression)),
          _sortFields(std::move(sortFields)) {}

    GetNextResult getNext() final {
        pExpCtx->checkForInterrupt();

        if (_hasLinearRule) {
            loadBufferedOutput();
            if (_bufferedOutput.empty()) {
                return GetNextResult::makeEOF();
            }

            auto output = std::move(_bufferedOutput.front());
            _bufferedOutput.pop_front();
            return GetNextResult(std::move(output));
        }

        auto next = pSource->getNext();
        if (!next.isAdvanced()) {
            return next;
        }

        auto input = next.releaseDocument();
        resetLocfStateForNewPartition(input);
        validateSortOrder(input);

        MutableDocument output(input);
        for (auto& rule : _rules) {
            auto current = input.getNestedField(rule.field);
            if (rule.type == RuleType::kLiteral) {
                if (current.nullish()) {
                    output.setNestedField(rule.field, rule.literal);
                }
                continue;
            }

            if (current.nullish()) {
                if (!rule.lastSeen.missing()) {
                    output.setNestedField(rule.field, rule.lastSeen);
                }
            } else {
                rule.lastSeen = current;
            }
        }

        return GetNextResult(output.freeze());
    }

    const char* getSourceName() const final {
        return kStageName.rawData();
    }

    Value serialize(boost::optional<ExplainOptions::Verbosity> explain = boost::none) const final {
        MutableDocument output;
        for (const auto& rule : _rules) {
            MutableDocument ruleSpec;
            if (rule.type == RuleType::kLiteral) {
                ruleSpec["value"] = rule.literal;
            } else if (rule.type == RuleType::kLocf) {
                ruleSpec["method"] = Value("locf"_sd);
            } else {
                ruleSpec["method"] = Value("linear"_sd);
            }
            output[rule.field.fullPath()] = ruleSpec.freezeToValue();
        }

        MutableDocument spec;
        if (_partitionByExpression) {
            spec["partitionBy"] = _partitionByExpression->serialize(false);
        }
        if (!_partitionByFields.empty()) {
            std::vector<Value> fields;
            for (const auto& field : _partitionByFields) {
                fields.push_back(Value(field.fullPath()));
            }
            spec["partitionByFields"] = Value(std::move(fields));
        }
        if (!_sortBy.isEmpty()) {
            spec["sortBy"] = Value(_sortBy);
        }
        spec["output"] = output.freezeToValue();
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
        uassert(6789120, "$fill requires an object specification", elem.type() == BSONType::Object);

        BSONObj spec = elem.Obj();
        boost::intrusive_ptr<Expression> partitionByExpression;
        if (auto partitionByElem = spec["partitionBy"]) {
            VariablesParseState vps = expCtx->variablesParseState;
            partitionByExpression = Expression::parseOperand(expCtx, partitionByElem, vps);
        }

        std::vector<FieldPath> partitionByFields;
        if (auto partitionElem = spec["partitionByFields"]) {
            uassert(6789121,
                    "$fill cannot specify both partitionBy and partitionByFields",
                    !partitionByExpression);
            uassert(6789122,
                    "$fill partitionByFields must be an array of strings",
                    partitionElem.type() == BSONType::Array);
            for (auto&& fieldElem : partitionElem.Obj()) {
                uassert(6789130,
                        "$fill partitionByFields entries must be strings",
                        fieldElem.type() == BSONType::String);
                partitionByFields.emplace_back(fieldElem.str());
            }
        }

        BSONObj sortBy;
        std::vector<SortField> sortFields;
        if (auto sortByElem = spec["sortBy"]) {
            uassert(6789123, "$fill sortBy must be an object", sortByElem.type() == BSONType::Object);
            sortBy = sortByElem.Obj().getOwned();
            for (auto&& sortElem : sortBy) {
                uassert(6789131,
                        "$fill sortBy fields must have direction 1 or -1",
                        sortElem.isNumber() &&
                            (sortElem.numberInt() == 1 || sortElem.numberInt() == -1));
                sortFields.push_back(
                    {FieldPath(sortElem.fieldNameStringData()), sortElem.numberInt()});
            }
        }

        BSONElement outputElem = spec["output"];
        uassert(6789124, "$fill requires an object 'output'", outputElem.type() == BSONType::Object);

        std::vector<Rule> rules;
        for (auto&& fieldSpecElem : outputElem.Obj()) {
            uassert(6789125,
                    "$fill output field specification must be an object",
                    fieldSpecElem.type() == BSONType::Object);

            BSONObj fieldSpec = fieldSpecElem.Obj();
            BSONElement methodElem = fieldSpec["method"];
            BSONElement valueElem = fieldSpec["value"];
            uassert(6789126,
                    "$fill output fields require exactly one of 'method' or 'value'",
                    methodElem.eoo() != valueElem.eoo());

            Rule rule{
                FieldPath(fieldSpecElem.fieldNameStringData()), RuleType::kLiteral, Value(), Value()};
            if (!methodElem.eoo()) {
                uassert(6789127,
                        "$fill method must be a string",
                        methodElem.type() == BSONType::String);
                uassert(6789128,
                        "$fill currently supports only method: 'locf' or 'linear'",
                        methodElem.str() == "locf" || methodElem.str() == "linear");
                if (methodElem.str() == "locf") {
                    rule.type = RuleType::kLocf;
                } else {
                    uassert(6789133,
                            "$fill method: 'linear' requires sortBy",
                            !sortBy.isEmpty());
                    rule.type = RuleType::kLinear;
                }
            } else {
                rule.literal = Value(valueElem);
            }
            rules.push_back(std::move(rule));
        }
        uassert(6789129, "$fill requires at least one output field", !rules.empty());

        return new DocumentSourceFill(expCtx,
                                      std::move(rules),
                                      sortBy,
                                      std::move(partitionByFields),
                                      std::move(partitionByExpression),
                                      std::move(sortFields));
    }

private:
    bool hasLinearRule() const {
        for (const auto& rule : _rules) {
            if (rule.type == RuleType::kLinear) {
                return true;
            }
        }
        return false;
    }

    void resetLocfStateForNewPartition(const Document& input) {
        if (!hasPartitioning()) {
            return;
        }

        auto key = makePartitionKey(input);
        if (!_havePartition ||
            pExpCtx->getValueComparator().compare(key, _currentPartitionKey) != 0) {
            for (auto& rule : _rules) {
                rule.lastSeen = Value();
            }
            _currentPartitionKey = key;
            _havePartition = true;
            _haveLastSortKey = false;
            _lastSortKey.clear();
        }
    }

    Value makePartitionKey(const Document& input) const {
        if (_partitionByExpression) {
            return _partitionByExpression->evaluate(input);
        }

        std::vector<Value> key;
        key.reserve(_partitionByFields.size());
        for (const auto& field : _partitionByFields) {
            auto value = input.getNestedField(field);
            key.push_back(value.missing() ? Value(BSONNULL) : value);
        }
        return Value(std::move(key));
    }

    std::vector<Value> makeSortKey(const Document& input) const {
        std::vector<Value> key;
        key.reserve(_sortFields.size());
        for (const auto& sortField : _sortFields) {
            key.push_back(input.getNestedField(sortField.field));
        }
        return key;
    }

    void validateSortOrder(const Document& input) {
        if (_sortFields.empty()) {
            return;
        }

        auto key = makeSortKey(input);
        if (_haveLastSortKey) {
            const auto& comparator = pExpCtx->getValueComparator();
            for (size_t i = 0; i < key.size(); ++i) {
                int cmp = comparator.compare(_lastSortKey[i], key[i]) * _sortFields[i].direction;
                if (cmp < 0) {
                    break;
                }
                uassert(6789132,
                        "$fill requires input documents to be sorted by sortBy within each partition",
                        cmp == 0);
            }
        }

        _lastSortKey = std::move(key);
        _haveLastSortKey = true;
    }

    double extractLinearSortValue(const Document& input) const {
        invariant(!_sortFields.empty());
        auto value = input.getNestedField(_sortFields.front().field);
        if (value.getType() == BSONType::Date) {
            return static_cast<double>(value.getDate().toMillisSinceEpoch());
        }
        uassert(6789134, "$fill linear sortBy field must be numeric or date", value.numeric());
        return value.coerceToDouble();
    }

    enum class LinearValueType { kNumeric, kDate };

    LinearValueType linearValueType(const Value& value) const {
        uassert(6789383,
                "$fill linear values must be numeric, date, or null",
                value.numeric() || value.getType() == BSONType::Date);
        return value.getType() == BSONType::Date ? LinearValueType::kDate
                                                 : LinearValueType::kNumeric;
    }

    double linearValueAsDouble(const Value& value, LinearValueType type) const {
        if (type == LinearValueType::kDate) {
            return static_cast<double>(value.getDate().toMillisSinceEpoch());
        }
        return value.coerceToDouble();
    }

    Value makeLinearValue(double value, LinearValueType type) const {
        if (type == LinearValueType::kDate) {
            return Value(Date_t::fromMillisSinceEpoch(static_cast<long long>(std::llround(value))));
        }
        return Value(value);
    }

    void setBufferedField(std::vector<Document>* docs,
                          size_t index,
                          const FieldPath& field,
                          const Value& value) const {
        MutableDocument output((*docs)[index]);
        output.setNestedField(field, value);
        (*docs)[index] = output.freeze();
    }

    void interpolateLinearRule(std::vector<Document>* docs,
                               size_t begin,
                               size_t end,
                               const Rule& rule) const {
        boost::optional<size_t> previousAnchor;
        for (size_t i = begin; i < end; ++i) {
            auto current = (*docs)[i].getNestedField(rule.field);
            if (current.nullish()) {
                continue;
            }

            linearValueType(current);
            if (previousAnchor) {
                double previousX = extractLinearSortValue((*docs)[*previousAnchor]);
                double nextX = extractLinearSortValue((*docs)[i]);
                auto previousValue = (*docs)[*previousAnchor].getNestedField(rule.field);
                auto previousType = linearValueType(previousValue);
                auto nextType = linearValueType(current);
                uassert(6789137,
                        "$fill linear requires matching numeric/date value types",
                        previousType == nextType);
                double previousY = linearValueAsDouble(previousValue, previousType);
                double nextY = linearValueAsDouble(current, nextType);

                if (nextX != previousX) {
                    for (size_t gap = *previousAnchor + 1; gap < i; ++gap) {
                        auto gapValue = (*docs)[gap].getNestedField(rule.field);
                        if (!gapValue.nullish()) {
                            continue;
                        }

                        double gapX = extractLinearSortValue((*docs)[gap]);
                        double ratio = (gapX - previousX) / (nextX - previousX);
                        setBufferedField(
                            docs,
                            gap,
                            rule.field,
                            makeLinearValue(previousY + (nextY - previousY) * ratio,
                                            previousType));
                    }
                }
            }
            previousAnchor = i;
        }
    }

    void applyLinearRules(std::vector<Document>* docs) const {
        size_t partitionStart = 0;
        Value previousPartitionKey;
        bool havePartition = false;

        for (size_t i = 0; i <= docs->size(); ++i) {
            bool atEnd = i == docs->size();
            Value partitionKey;
            if (!atEnd && hasPartitioning()) {
                partitionKey = makePartitionKey((*docs)[i]);
            }

            bool startsNewPartition = atEnd;
            if (!atEnd && hasPartitioning()) {
                startsNewPartition = !havePartition ||
                    pExpCtx->getValueComparator().compare(partitionKey, previousPartitionKey) != 0;
            }

            if (startsNewPartition) {
                if (havePartition || atEnd) {
                    for (const auto& rule : _rules) {
                        if (rule.type == RuleType::kLinear) {
                            interpolateLinearRule(docs, partitionStart, i, rule);
                        }
                    }
                }
                partitionStart = i;
                previousPartitionKey = partitionKey;
                havePartition = !atEnd;
            }
        }
    }

    bool hasPartitioning() const {
        return _partitionByExpression || !_partitionByFields.empty();
    }

    void loadBufferedOutput() {
        if (_loadedBufferedOutput) {
            return;
        }
        _loadedBufferedOutput = true;

        std::vector<Document> docs;
        while (true) {
            auto next = pSource->getNext();
            uassert(6789136, "$fill linear does not support paused input", !next.isPaused());
            if (next.isEOF()) {
                break;
            }

            auto input = next.releaseDocument();
            resetLocfStateForNewPartition(input);
            validateSortOrder(input);

            MutableDocument output(input);
            for (auto& rule : _rules) {
                auto current = input.getNestedField(rule.field);
                if (rule.type == RuleType::kLiteral) {
                    if (current.nullish()) {
                        output.setNestedField(rule.field, rule.literal);
                    }
                } else if (rule.type == RuleType::kLocf) {
                    if (current.nullish()) {
                        if (!rule.lastSeen.missing()) {
                            output.setNestedField(rule.field, rule.lastSeen);
                        }
                    } else {
                        rule.lastSeen = current;
                    }
                }
            }
            docs.push_back(output.freeze());
        }

        applyLinearRules(&docs);
        for (auto& doc : docs) {
            _bufferedOutput.push_back(std::move(doc));
        }
    }

    std::vector<Rule> _rules;
    BSONObj _sortBy;
    std::vector<FieldPath> _partitionByFields;
    boost::intrusive_ptr<Expression> _partitionByExpression;
    std::vector<SortField> _sortFields;
    bool _havePartition = false;
    Value _currentPartitionKey;
    bool _haveLastSortKey = false;
    std::vector<Value> _lastSortKey;
    bool _hasLinearRule = hasLinearRule();
    bool _loadedBufferedOutput = false;
    std::deque<Document> _bufferedOutput;
};

constexpr StringData DocumentSourceFill::kStageName;

}  // namespace

REGISTER_DOCUMENT_SOURCE(fill,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourceFill::createFromBson);

}  // namespace mongo
