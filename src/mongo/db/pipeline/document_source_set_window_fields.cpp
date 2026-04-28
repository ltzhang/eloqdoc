/**
 * Copyright (C) 2026 EloqData Inc.
 *
 * This program is free software: you can redistribute it and/or  modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 */

#include "mongo/platform/basic.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <limits>
#include <tuple>

#include "mongo/db/jsobj.h"
#include "mongo/db/pipeline/document.h"
#include "mongo/db/pipeline/document_source.h"
#include "mongo/db/pipeline/expression.h"
#include "mongo/db/pipeline/field_path.h"
#include "mongo/db/pipeline/lite_parsed_document_source.h"
#include "mongo/db/pipeline/value.h"
#include "mongo/db/pipeline/variables.h"

namespace mongo {
namespace {

class DocumentSourceSetWindowFields final : public DocumentSource {
public:
    static constexpr StringData kStageName = "$setWindowFields"_sd;

    GetNextResult getNext() final {
        pExpCtx->checkForInterrupt();

        if (!_initialized) {
            auto initResult = initialize();
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

    const char* getSourceName() const final {
        return kStageName.rawData();
    }

    Value serialize(boost::optional<ExplainOptions::Verbosity> explain = boost::none) const final {
        MutableDocument spec;
        if (_partitionBy) {
            spec["partitionBy"] = _partitionBy->serialize(false);
        }
        if (!_sortBy.isEmpty()) {
            spec["sortBy"] = Value(Document(_sortBy));
        }

        MutableDocument output;
        for (auto&& out : _outputs) {
            MutableDocument outSpec;
            if (out.opName == "$shift") {
                MutableDocument shiftSpec;
                shiftSpec["output"] = out.argument->serialize(false);
                shiftSpec["by"] = Value(out.shiftBy);
                if (out.shiftDefault) {
                    shiftSpec["default"] = out.shiftDefault->serialize(false);
                }
                outSpec[out.opName] = shiftSpec.freezeToValue();
            } else {
                outSpec[out.opName] = out.argument ? out.argument->serialize(false) : Value(Document());
            }
            MutableDocument window;
            if (out.hasWindow) {
                auto bounds = Value(std::vector<Value>{windowBoundValue(out.lower),
                                                       windowBoundValue(out.upper)});
                if (out.windowType == WindowType::kDocuments) {
                    window["documents"] = bounds;
                } else {
                    window["range"] = bounds;
                }
                outSpec["window"] = window.freezeToValue();
            }
            output[out.field.fullPath()] = outSpec.freezeToValue();
        }
        spec["output"] = output.freezeToValue();
        return Value(Document{{kStageName, spec.freezeToValue()}});
    }

    StageConstraints constraints(Pipeline::SplitState pipeState) const final {
        return StageConstraints(StreamType::kBlocking,
                                PositionRequirement::kNone,
                                HostTypeRequirement::kNone,
                                DiskUseRequirement::kNoDiskUse,
                                FacetRequirement::kAllowed,
                                TransactionRequirement::kAllowed);
    }

    static boost::intrusive_ptr<DocumentSource> createFromBson(
        BSONElement elem, const boost::intrusive_ptr<ExpressionContext>& expCtx) {
        uassert(6789319,
                "$setWindowFields requires an object specification",
                elem.type() == BSONType::Object);

        auto spec = elem.Obj();
        boost::intrusive_ptr<Expression> partitionBy;
        if (auto partitionElem = spec["partitionBy"]) {
            VariablesParseState vps = expCtx->variablesParseState;
            partitionBy = Expression::parseOperand(expCtx, partitionElem, vps)->optimize();
        }

        BSONObj sortBy;
        if (auto sortByElem = spec["sortBy"]) {
            uassert(6789320,
                    "$setWindowFields sortBy must be an object",
                    sortByElem.type() == BSONType::Object);
            sortBy = sortByElem.Obj().getOwned();
            for (auto&& sortElem : sortBy) {
                uassert(6789321,
                        "$setWindowFields sortBy directions must be 1 or -1",
                        sortElem.isNumber() &&
                            (sortElem.numberInt() == 1 || sortElem.numberInt() == -1));
            }
        }

        auto outputElem = spec["output"];
        uassert(6789322,
                "$setWindowFields requires an object output specification",
                outputElem.type() == BSONType::Object);

        std::vector<OutputSpec> outputs;
        for (auto&& outputField : outputElem.Obj()) {
            uassert(6789323,
                    "$setWindowFields output entries must be objects",
                    outputField.type() == BSONType::Object);
            outputs.push_back(parseOutputSpec(expCtx, outputField));
        }
        uassert(6789324, "$setWindowFields output must not be empty", !outputs.empty());

        for (auto&& option : spec) {
            auto fieldName = option.fieldNameStringData();
            uassert(6789325,
                    str::stream() << "unknown $setWindowFields option '" << fieldName << "'",
                    fieldName == "partitionBy"_sd || fieldName == "sortBy"_sd ||
                        fieldName == "output"_sd);
        }

        return new DocumentSourceSetWindowFields(
            expCtx, std::move(partitionBy), sortBy, std::move(outputs));
    }

private:
    enum class BoundKind { kUnbounded, kCurrent, kOffset };
    enum class WindowType { kDocuments, kRange };
    struct WindowBound {
        BoundKind kind = BoundKind::kCurrent;
        double offset = 0;
    };

    struct OutputSpec {
        FieldPath field;
        std::string opName;
        boost::intrusive_ptr<Expression> argument;
        boost::intrusive_ptr<Expression> shiftDefault;
        int shiftBy = 0;
        bool hasWindow = false;
        WindowType windowType = WindowType::kDocuments;
        WindowBound lower;
        WindowBound upper;
    };

    struct BufferedDoc {
        Document doc;
        Value partitionKey;
        std::vector<Value> sortKeys;
        size_t inputOrdinal = 0;
    };

    DocumentSourceSetWindowFields(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                                  boost::intrusive_ptr<Expression> partitionBy,
                                  BSONObj sortBy,
                                  std::vector<OutputSpec> outputs)
        : DocumentSource(expCtx),
          _partitionBy(std::move(partitionBy)),
          _sortBy(sortBy.getOwned()),
          _outputs(std::move(outputs)) {}

    static OutputSpec parseOutputSpec(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                                      BSONElement outputField) {
        auto obj = outputField.Obj();
        boost::optional<BSONElement> opElem;
        BSONElement windowElem;
        for (auto&& elem : obj) {
            auto fieldName = elem.fieldNameStringData();
            if (fieldName == "window"_sd) {
                windowElem = elem;
            } else if (fieldName.startsWith("$")) {
                uassert(6789326,
                        "$setWindowFields output entry must contain exactly one operator",
                        !opElem);
                opElem = elem;
            } else {
                uasserted(6789327,
                          str::stream() << "unknown $setWindowFields output option '" << fieldName
                                        << "'");
            }
        }
        uassert(6789328, "$setWindowFields output entry requires an operator", opElem);

        OutputSpec out{FieldPath(outputField.fieldName()),
                       opElem->fieldName(),
                       nullptr,
                       nullptr,
                       0,
                       false,
                       WindowType::kDocuments,
                       {},
                       {}};
        uassert(6789329,
                str::stream() << "unsupported $setWindowFields operator '" << out.opName << "'",
                out.opName == "$sum" || out.opName == "$avg" || out.opName == "$count" ||
                    out.opName == "$min" || out.opName == "$max" || out.opName == "$first" ||
                    out.opName == "$last" || out.opName == "$documentNumber" ||
                    out.opName == "$rank" || out.opName == "$denseRank" ||
                    out.opName == "$shift");

        if (out.opName == "$count") {
            uassert(6789330,
                    "$setWindowFields $count argument must be an empty object",
                    opElem->type() == BSONType::Object && opElem->Obj().isEmpty());
        } else if (out.opName == "$documentNumber" || out.opName == "$rank" ||
                   out.opName == "$denseRank") {
            uassert(6789337,
                    str::stream() << out.opName << " argument must be an empty object",
                    opElem->type() == BSONType::Object && opElem->Obj().isEmpty());
        } else if (out.opName == "$shift") {
            parseShiftSpec(expCtx, *opElem, &out);
        } else {
            VariablesParseState vps = expCtx->variablesParseState;
            out.argument = Expression::parseOperand(expCtx, *opElem, vps)->optimize();
        }

        if (out.opName == "$documentNumber" || out.opName == "$rank" ||
            out.opName == "$denseRank" || out.opName == "$shift") {
            uassert(6789338,
                    str::stream() << out.opName << " does not accept a window option",
                    !windowElem);
        } else {
            parseWindow(windowElem, &out);
        }
        return out;
    }

    static void parseShiftSpec(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                               BSONElement opElem,
                               OutputSpec* out) {
        uassert(6789339,
                "$setWindowFields $shift argument must be an object",
                opElem.type() == BSONType::Object);
        auto spec = opElem.Obj();
        auto outputElem = spec["output"];
        auto byElem = spec["by"];
        uassert(6789340, "$setWindowFields $shift requires output", outputElem);
        uassert(6789341,
                "$setWindowFields $shift requires integer by",
                byElem.isNumber() && byElem.numberInt() == byElem.numberDouble());
        out->shiftBy = byElem.numberInt();
        uassert(6789342, "$setWindowFields $shift by must not be 0", out->shiftBy != 0);

        VariablesParseState vps = expCtx->variablesParseState;
        out->argument = Expression::parseOperand(expCtx, outputElem, vps)->optimize();
        if (auto defaultElem = spec["default"]) {
            out->shiftDefault = Expression::parseOperand(expCtx, defaultElem, vps)->optimize();
        }

        for (auto&& option : spec) {
            auto fieldName = option.fieldNameStringData();
            uassert(6789343,
                    str::stream() << "unknown $setWindowFields $shift option '" << fieldName
                                  << "'",
                    fieldName == "output"_sd || fieldName == "by"_sd ||
                        fieldName == "default"_sd);
        }
    }

    static void parseWindow(BSONElement windowElem, OutputSpec* out) {
        if (!windowElem) {
            out->lower = {BoundKind::kCurrent, 0};
            out->upper = {BoundKind::kCurrent, 0};
            out->hasWindow = false;
            return;
        }
        out->hasWindow = true;
        uassert(6789331,
                "$setWindowFields window must be an object",
                windowElem.type() == BSONType::Object);
        auto windowObj = windowElem.Obj();
        auto documentsElem = windowObj["documents"];
        auto rangeElem = windowObj["range"];
        uassert(6789332,
                "$setWindowFields window requires documents or range",
                (documentsElem.type() == BSONType::Array) != (rangeElem.type() == BSONType::Array));
        out->windowType = documentsElem.type() == BSONType::Array ? WindowType::kDocuments
                                                                  : WindowType::kRange;
        BSONElement boundsElem = out->windowType == WindowType::kDocuments ? documentsElem : rangeElem;
        std::vector<BSONElement> bounds;
        for (auto&& bound : boundsElem.Obj()) {
            bounds.push_back(bound);
        }
        uassert(6789333,
                "$setWindowFields window requires exactly two bounds",
                bounds.size() == 2);
        out->lower = parseBound(bounds[0], out->windowType);
        out->upper = parseBound(bounds[1], out->windowType);

        for (auto&& option : windowObj) {
            auto fieldName = option.fieldNameStringData();
            uassert(6789334,
                    str::stream() << "unknown $setWindowFields window option '" << fieldName
                                  << "'",
                    fieldName == "documents"_sd || fieldName == "range"_sd ||
                        fieldName == "unit"_sd);
            uassert(6789344,
                    "$setWindowFields range unit is not supported in this checkpoint",
                    fieldName != "unit"_sd);
        }
    }

    static WindowBound parseBound(BSONElement elem, WindowType windowType) {
        if (elem.type() == BSONType::String) {
            auto value = elem.valueStringData();
            if (value == "unbounded"_sd) {
                return {BoundKind::kUnbounded, 0};
            }
            if (value == "current"_sd) {
                return {BoundKind::kCurrent, 0};
            }
            uasserted(6789335,
                      str::stream() << "unsupported $setWindowFields window bound '" << value
                                    << "'");
        }
        uassert(6789336,
                "$setWindowFields window bounds must be numbers or strings",
                elem.isNumber());
        if (windowType == WindowType::kDocuments) {
            uassert(6789345,
                    "$setWindowFields documents window numeric bounds must be integers",
                    elem.numberInt() == elem.numberDouble());
            return {BoundKind::kOffset, static_cast<double>(elem.numberInt())};
        }
        return {BoundKind::kOffset, elem.numberDouble()};
    }

    GetNextResult initialize() {
        while (!_inputExhausted) {
            auto next = pSource->getNext();
            if (next.isPaused()) {
                return next;
            }
            if (next.isEOF()) {
                _inputExhausted = true;
                break;
            }
            auto doc = next.releaseDocument();
            _buffer.push_back({doc, makePartitionKey(doc), makeSortKeys(doc), _buffer.size()});
        }

        std::stable_sort(_buffer.begin(), _buffer.end(), [this](const auto& left, const auto& right) {
            auto partitionCmp =
                pExpCtx->getValueComparator().compare(left.partitionKey, right.partitionKey);
            if (partitionCmp != 0) {
                return partitionCmp < 0;
            }
            for (size_t i = 0; i < left.sortKeys.size(); ++i) {
                auto cmp = pExpCtx->getValueComparator().compare(left.sortKeys[i], right.sortKeys[i]);
                if (cmp != 0) {
                    return _sortDirections[i] > 0 ? cmp < 0 : cmp > 0;
                }
            }
            return left.inputOrdinal < right.inputOrdinal;
        });

        for (size_t start = 0; start < _buffer.size();) {
            size_t end = start + 1;
            while (end < _buffer.size() && samePartition(_buffer[start], _buffer[end])) {
                ++end;
            }
            evaluatePartition(start, end);
            start = end;
        }

        _initialized = true;
        return GetNextResult::makeEOF();
    }

    Value makePartitionKey(const Document& doc) const {
        if (!_partitionBy) {
            return Value();
        }
        return _partitionBy->evaluate(doc);
    }

    std::vector<Value> makeSortKeys(const Document& doc) {
        std::vector<Value> keys;
        if (_sortDirections.empty()) {
            for (auto&& elem : _sortBy) {
                _sortDirections.push_back(elem.numberInt());
            }
        }
        for (auto&& elem : _sortBy) {
            keys.push_back(doc.getNestedField(FieldPath(elem.fieldName())));
        }
        return keys;
    }

    bool samePartition(const BufferedDoc& left, const BufferedDoc& right) const {
        return pExpCtx->getValueComparator().compare(left.partitionKey, right.partitionKey) == 0;
    }

    void evaluatePartition(size_t start, size_t end) {
        const auto partitionSize = static_cast<int>(end - start);
        for (size_t i = start; i < end; ++i) {
            MutableDocument output(_buffer[i].doc);
            const auto relativeIndex = static_cast<int>(i - start);
            for (auto&& outSpec : _outputs) {
                int first;
                int last;
                if (outSpec.windowType == WindowType::kRange) {
                    std::tie(first, last) = rangeIndexes(outSpec, start, end, relativeIndex);
                } else {
                    first = lowerIndex(outSpec.lower, relativeIndex, partitionSize);
                    last = upperIndex(outSpec.upper, relativeIndex, partitionSize);
                }
                output.setNestedField(outSpec.field,
                                      evaluateOutput(outSpec, start, end, relativeIndex, first, last));
            }
            _pending.push_back(output.freeze());
        }
    }

    Value evaluateOutput(const OutputSpec& outSpec,
                         size_t partitionStart,
                         size_t partitionEnd,
                         int relativeIndex,
                         int first,
                         int last) const {
        if (outSpec.opName == "$documentNumber") {
            return Value(relativeIndex + 1);
        }
        if (outSpec.opName == "$rank") {
            return Value(rankFor(partitionStart, partitionEnd, relativeIndex));
        }
        if (outSpec.opName == "$denseRank") {
            return Value(denseRankFor(partitionStart, partitionEnd, relativeIndex));
        }
        if (outSpec.opName == "$shift") {
            const int target = relativeIndex + outSpec.shiftBy;
            if (target < 0 || target >= static_cast<int>(partitionEnd - partitionStart)) {
                if (outSpec.shiftDefault) {
                    return outSpec.shiftDefault->evaluate(_buffer[partitionStart + relativeIndex].doc);
                }
                return Value(BSONNULL);
            }
            return outSpec.argument->evaluate(_buffer[partitionStart + target].doc);
        }
        return evaluateWindow(outSpec, partitionStart, first, last);
    }

    int rankFor(size_t partitionStart, size_t partitionEnd, int relativeIndex) const {
        int rank = 1;
        for (int i = 1; i <= relativeIndex; ++i) {
            if (!sameSortKey(_buffer[partitionStart + i - 1], _buffer[partitionStart + i])) {
                rank = i + 1;
            }
        }
        return rank;
    }

    int denseRankFor(size_t partitionStart, size_t partitionEnd, int relativeIndex) const {
        int rank = 1;
        for (int i = 1; i <= relativeIndex; ++i) {
            if (!sameSortKey(_buffer[partitionStart + i - 1], _buffer[partitionStart + i])) {
                ++rank;
            }
        }
        return rank;
    }

    bool sameSortKey(const BufferedDoc& left, const BufferedDoc& right) const {
        if (left.sortKeys.size() != right.sortKeys.size()) {
            return false;
        }
        for (size_t i = 0; i < left.sortKeys.size(); ++i) {
            if (pExpCtx->getValueComparator().compare(left.sortKeys[i], right.sortKeys[i]) != 0) {
                return false;
            }
        }
        return true;
    }

    static int lowerIndex(const WindowBound& bound, int current, int partitionSize) {
        switch (bound.kind) {
            case BoundKind::kUnbounded:
                return 0;
            case BoundKind::kCurrent:
                return current;
            case BoundKind::kOffset:
                return std::max(0, current + static_cast<int>(bound.offset));
        }
        MONGO_UNREACHABLE;
    }

    static int upperIndex(const WindowBound& bound, int current, int partitionSize) {
        switch (bound.kind) {
            case BoundKind::kUnbounded:
                return partitionSize - 1;
            case BoundKind::kCurrent:
                return current;
            case BoundKind::kOffset:
                return std::min(partitionSize - 1, current + static_cast<int>(bound.offset));
        }
        MONGO_UNREACHABLE;
    }

    std::pair<int, int> rangeIndexes(const OutputSpec& outSpec,
                                     size_t partitionStart,
                                     size_t partitionEnd,
                                     int relativeIndex) const {
        uassert(6789346,
                "$setWindowFields range windows require exactly one sortBy field",
                _sortDirections.size() == 1);

        auto currentKey = _buffer[partitionStart + relativeIndex].sortKeys[0];
        uassert(6789347,
                "$setWindowFields range windows require numeric sort keys",
                currentKey.numeric());
        const double current = currentKey.coerceToDouble();
        const double lower = rangeBoundary(outSpec.lower, current, true);
        const double upper = rangeBoundary(outSpec.upper, current, false);
        if (lower > upper) {
            return {1, 0};
        }

        int first = -1;
        int last = -2;
        for (int i = 0; i < static_cast<int>(partitionEnd - partitionStart); ++i) {
            auto sortKey = _buffer[partitionStart + i].sortKeys[0];
            uassert(6789348,
                    "$setWindowFields range windows require numeric sort keys",
                    sortKey.numeric());
            const double value = sortKey.coerceToDouble();
            if (value >= lower && value <= upper) {
                if (first < 0) {
                    first = i;
                }
                last = i;
            }
        }
        return {first < 0 ? 1 : first, last};
    }

    static double rangeBoundary(const WindowBound& bound, double current, bool lower) {
        switch (bound.kind) {
            case BoundKind::kUnbounded:
                return lower ? -std::numeric_limits<double>::infinity()
                             : std::numeric_limits<double>::infinity();
            case BoundKind::kCurrent:
                return current;
            case BoundKind::kOffset:
                return current + bound.offset;
        }
        MONGO_UNREACHABLE;
    }

    Value evaluateWindow(const OutputSpec& outSpec, size_t partitionStart, int first, int last) const {
        if (first > last) {
            return outSpec.opName == "$count" ? Value(0) : Value(BSONNULL);
        }

        if (outSpec.opName == "$count") {
            return Value(last - first + 1);
        }

        if (outSpec.opName == "$first") {
            return outSpec.argument->evaluate(_buffer[partitionStart + first].doc);
        }
        if (outSpec.opName == "$last") {
            return outSpec.argument->evaluate(_buffer[partitionStart + last].doc);
        }

        bool haveValue = false;
        Value minOrMax;
        double sum = 0;
        int count = 0;
        for (int i = first; i <= last; ++i) {
            auto value = outSpec.argument->evaluate(_buffer[partitionStart + i].doc);
            if (outSpec.opName == "$sum" || outSpec.opName == "$avg") {
                if (value.numeric()) {
                    sum += value.coerceToDouble();
                    ++count;
                }
                continue;
            }
            if (value.nullish()) {
                continue;
            }
            if (!haveValue) {
                minOrMax = value;
                haveValue = true;
                continue;
            }
            auto cmp = pExpCtx->getValueComparator().compare(value, minOrMax);
            if ((outSpec.opName == "$min" && cmp < 0) || (outSpec.opName == "$max" && cmp > 0)) {
                minOrMax = value;
            }
        }

        if (outSpec.opName == "$sum") {
            return numericValue(sum);
        }
        if (outSpec.opName == "$avg") {
            return count == 0 ? Value(BSONNULL) : numericValue(sum / count);
        }
        return haveValue ? minOrMax : Value(BSONNULL);
    }

    static Value numericValue(double value) {
        long long asLong = static_cast<long long>(value);
        if (std::abs(value - static_cast<double>(asLong)) < 1e-9) {
            if (asLong >= std::numeric_limits<int>::min() &&
                asLong <= std::numeric_limits<int>::max()) {
                return Value(static_cast<int>(asLong));
            }
            return Value(asLong);
        }
        return Value(value);
    }

    static Value windowBoundValue(const WindowBound& bound) {
        switch (bound.kind) {
            case BoundKind::kUnbounded:
                return Value("unbounded"_sd);
            case BoundKind::kCurrent:
                return Value("current"_sd);
            case BoundKind::kOffset:
                return numericValue(bound.offset);
        }
        MONGO_UNREACHABLE;
    }

    boost::intrusive_ptr<Expression> _partitionBy;
    BSONObj _sortBy;
    std::vector<OutputSpec> _outputs;
    bool _initialized = false;
    bool _inputExhausted = false;
    std::vector<BufferedDoc> _buffer;
    std::vector<int> _sortDirections;
    std::deque<Document> _pending;
};

constexpr StringData DocumentSourceSetWindowFields::kStageName;

}  // namespace

REGISTER_DOCUMENT_SOURCE(setWindowFields,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourceSetWindowFields::createFromBson);

}  // namespace mongo
