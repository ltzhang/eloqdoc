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
            } else if (out.opName == "$expMovingAvg") {
                MutableDocument avgSpec;
                avgSpec["input"] = out.argument->serialize(false);
                avgSpec["alpha"] = numericValue(out.expMovingAvgAlpha);
                outSpec[out.opName] = avgSpec.freezeToValue();
            } else if (out.opName == "$integral" || out.opName == "$derivative") {
                MutableDocument operatorSpec;
                operatorSpec["input"] = out.argument->serialize(false);
                outSpec[out.opName] = operatorSpec.freezeToValue();
            } else if (isNValueOperator(out.opName)) {
                MutableDocument operatorSpec;
                operatorSpec["input"] = out.argument->serialize(false);
                operatorSpec["n"] = Value(out.nValueCount);
                outSpec[out.opName] = operatorSpec.freezeToValue();
            } else if (isPercentileOperator(out.opName)) {
                MutableDocument operatorSpec;
                operatorSpec["input"] = out.argument->serialize(false);
                operatorSpec["method"] = Value("approximate"_sd);
                if (out.opName == "$percentile") {
                    std::vector<Value> pValues;
                    for (double percentile : out.percentiles) {
                        pValues.push_back(numericValue(percentile));
                    }
                    operatorSpec["p"] = Value(pValues);
                }
                outSpec[out.opName] = operatorSpec.freezeToValue();
            } else if (isTopBottomOperator(out.opName)) {
                MutableDocument operatorSpec;
                operatorSpec["sortBy"] = Value(Document(out.topBottomSortBy));
                operatorSpec["output"] = out.argument->serialize(false);
                if (isTopBottomNOperator(out.opName)) {
                    operatorSpec["n"] = Value(out.nValueCount);
                }
                outSpec[out.opName] = operatorSpec.freezeToValue();
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
                    if (!out.windowUnit.empty()) {
                        window["unit"] = Value(out.windowUnit);
                    }
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
        double expMovingAvgAlpha = 0;
        int nValueCount = 0;
        std::vector<double> percentiles;
        BSONObj topBottomSortBy;
        bool hasWindow = false;
        WindowType windowType = WindowType::kDocuments;
        double windowUnitMillis = 1;
        std::string windowUnit;
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
                       0,
                       0,
                       {},
                       {},
                       false,
                       WindowType::kDocuments,
                       1,
                       {},
                       {},
                       {}};
        uassert(6789329,
                str::stream() << "unsupported $setWindowFields operator '" << out.opName << "'",
                out.opName == "$sum" || out.opName == "$avg" || out.opName == "$count" ||
                    out.opName == "$min" || out.opName == "$max" || out.opName == "$first" ||
                    out.opName == "$last" || out.opName == "$documentNumber" ||
                    out.opName == "$rank" || out.opName == "$denseRank" ||
                    out.opName == "$shift" || out.opName == "$expMovingAvg" ||
                    out.opName == "$locf" || out.opName == "$covariancePop" ||
                    out.opName == "$covarianceSamp" || out.opName == "$integral" ||
                    out.opName == "$derivative" || isNValueOperator(out.opName) ||
                    isPercentileOperator(out.opName) || isTopBottomOperator(out.opName));

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
        } else if (out.opName == "$expMovingAvg") {
            parseExpMovingAvgSpec(expCtx, *opElem, &out);
        } else if (out.opName == "$integral" || out.opName == "$derivative") {
            parseIntegralDerivativeSpec(expCtx, *opElem, &out);
        } else if (isNValueOperator(out.opName)) {
            parseNValueSpec(expCtx, *opElem, &out);
        } else if (isPercentileOperator(out.opName)) {
            parsePercentileSpec(expCtx, *opElem, &out);
        } else if (isTopBottomOperator(out.opName)) {
            parseTopBottomSpec(expCtx, *opElem, &out);
        } else {
            if (out.opName == "$covariancePop" || out.opName == "$covarianceSamp") {
                uassert(6789355,
                        str::stream() << out.opName << " argument must be an array",
                        opElem->type() == BSONType::Array);
                int argumentCount = 0;
                for (auto&& argument : opElem->Obj()) {
                    (void)argument;
                    ++argumentCount;
                }
                uassert(6789356,
                        str::stream() << out.opName << " requires exactly two arguments",
                        argumentCount == 2);
            }
            VariablesParseState vps = expCtx->variablesParseState;
            out.argument = Expression::parseOperand(expCtx, *opElem, vps)->optimize();
        }

        if (out.opName == "$documentNumber" || out.opName == "$rank" ||
            out.opName == "$denseRank" || out.opName == "$shift" ||
            out.opName == "$expMovingAvg" || out.opName == "$locf") {
            uassert(6789338,
                    str::stream() << out.opName << " does not accept a window option",
                    !windowElem);
        } else {
            parseWindow(windowElem, &out);
            if (out.opName == "$integral" || out.opName == "$derivative") {
                uassert(6789361,
                        str::stream() << out.opName << " requires an explicit window option",
                        out.hasWindow);
            }
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

    static void parseExpMovingAvgSpec(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                                      BSONElement opElem,
                                      OutputSpec* out) {
        uassert(6789349,
                "$setWindowFields $expMovingAvg argument must be an object",
                opElem.type() == BSONType::Object);
        auto spec = opElem.Obj();
        auto inputElem = spec["input"];
        auto nElem = spec["N"];
        auto alphaElem = spec["alpha"];
        uassert(6789350, "$setWindowFields $expMovingAvg requires input", inputElem);
        uassert(6789351,
                "$setWindowFields $expMovingAvg requires exactly one of N or alpha",
                static_cast<bool>(nElem) != static_cast<bool>(alphaElem));

        VariablesParseState vps = expCtx->variablesParseState;
        out->argument = Expression::parseOperand(expCtx, inputElem, vps)->optimize();
        if (nElem) {
            uassert(6789352,
                    "$setWindowFields $expMovingAvg N must be a positive integer",
                    nElem.isNumber() && nElem.numberInt() == nElem.numberDouble() &&
                        nElem.numberInt() > 0);
            out->expMovingAvgAlpha = 2.0 / (static_cast<double>(nElem.numberInt()) + 1.0);
        } else {
            uassert(6789353,
                    "$setWindowFields $expMovingAvg alpha must be a number in (0, 1]",
                    alphaElem.isNumber() && alphaElem.numberDouble() > 0 &&
                        alphaElem.numberDouble() <= 1);
            out->expMovingAvgAlpha = alphaElem.numberDouble();
        }

        for (auto&& option : spec) {
            auto fieldName = option.fieldNameStringData();
            uassert(6789354,
                    str::stream() << "unknown $setWindowFields $expMovingAvg option '"
                                  << fieldName << "'",
                    fieldName == "input"_sd || fieldName == "N"_sd || fieldName == "alpha"_sd);
        }
    }

    static void parseIntegralDerivativeSpec(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                                            BSONElement opElem,
                                            OutputSpec* out) {
        uassert(6789357,
                str::stream() << out->opName << " argument must be an object",
                opElem.type() == BSONType::Object);
        auto spec = opElem.Obj();
        auto inputElem = spec["input"];
        uassert(6789358, str::stream() << out->opName << " requires input", inputElem);

        VariablesParseState vps = expCtx->variablesParseState;
        out->argument = Expression::parseOperand(expCtx, inputElem, vps)->optimize();

        for (auto&& option : spec) {
            auto fieldName = option.fieldNameStringData();
            uassert(6789359,
                    str::stream() << "unknown $setWindowFields " << out->opName << " option '"
                                  << fieldName << "'",
                    fieldName == "input"_sd || fieldName == "unit"_sd);
            uassert(6789360,
                    str::stream() << out->opName
                                  << " unit is not supported in this checkpoint",
                    fieldName != "unit"_sd);
        }
    }

    static bool isNValueOperator(const std::string& opName) {
        return opName == "$firstN" || opName == "$lastN" || opName == "$minN" ||
            opName == "$maxN";
    }

    static void parseNValueSpec(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                                BSONElement opElem,
                                OutputSpec* out) {
        uassert(6789364,
                str::stream() << out->opName << " argument must be an object",
                opElem.type() == BSONType::Object);
        auto spec = opElem.Obj();
        auto inputElem = spec["input"];
        auto nElem = spec["n"];
        uassert(6789365, str::stream() << out->opName << " requires input", inputElem);
        uassert(6789366,
                str::stream() << out->opName << " requires positive integer n",
                nElem.isNumber() && nElem.numberInt() == nElem.numberDouble() &&
                    nElem.numberInt() > 0);

        VariablesParseState vps = expCtx->variablesParseState;
        out->argument = Expression::parseOperand(expCtx, inputElem, vps)->optimize();
        out->nValueCount = nElem.numberInt();

        for (auto&& option : spec) {
            auto fieldName = option.fieldNameStringData();
            uassert(6789367,
                    str::stream() << "unknown $setWindowFields " << out->opName << " option '"
                                  << fieldName << "'",
                    fieldName == "input"_sd || fieldName == "n"_sd);
        }
    }

    static bool isPercentileOperator(const std::string& opName) {
        return opName == "$percentile" || opName == "$median";
    }

    static void parsePercentileSpec(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                                    BSONElement opElem,
                                    OutputSpec* out) {
        uassert(6789368,
                str::stream() << out->opName << " argument must be an object",
                opElem.type() == BSONType::Object);
        auto spec = opElem.Obj();
        auto inputElem = spec["input"];
        auto methodElem = spec["method"];
        uassert(6789369, str::stream() << out->opName << " requires input", inputElem);
        uassert(6789370,
                str::stream() << out->opName << " requires method: 'approximate'",
                methodElem.type() == BSONType::String &&
                    methodElem.valueStringData() == "approximate"_sd);

        VariablesParseState vps = expCtx->variablesParseState;
        out->argument = Expression::parseOperand(expCtx, inputElem, vps)->optimize();

        if (out->opName == "$median") {
            out->percentiles = {0.5};
        } else {
            auto pElem = spec["p"];
            uassert(6789371,
                    "$percentile requires p to be a non-empty array",
                    pElem.type() == BSONType::Array && !pElem.Obj().isEmpty());
            for (auto&& percentileElem : pElem.Obj()) {
                uassert(6789372, "$percentile p values must be numeric", percentileElem.isNumber());
                double percentile = percentileElem.numberDouble();
                uassert(6789373,
                        "$percentile p values must be in [0, 1]",
                        percentile >= 0 && percentile <= 1);
                out->percentiles.push_back(percentile);
            }
        }

        for (auto&& option : spec) {
            auto fieldName = option.fieldNameStringData();
            uassert(6789374,
                    str::stream() << "unknown $setWindowFields " << out->opName << " option '"
                                  << fieldName << "'",
                    fieldName == "input"_sd || fieldName == "method"_sd ||
                        (out->opName == "$percentile" && fieldName == "p"_sd));
        }
    }

    static bool isTopBottomOperator(const std::string& opName) {
        return opName == "$top" || opName == "$topN" || opName == "$bottom" ||
            opName == "$bottomN";
    }

    static bool isTopBottomNOperator(const std::string& opName) {
        return opName == "$topN" || opName == "$bottomN";
    }

    static void parseTopBottomSpec(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                                   BSONElement opElem,
                                   OutputSpec* out) {
        uassert(6789375,
                str::stream() << out->opName << " argument must be an object",
                opElem.type() == BSONType::Object);
        auto spec = opElem.Obj();
        auto sortByElem = spec["sortBy"];
        auto outputElem = spec["output"];
        auto nElem = spec["n"];
        uassert(6789376,
                str::stream() << out->opName << " requires object sortBy",
                sortByElem.type() == BSONType::Object);
        uassert(6789377, str::stream() << out->opName << " requires output", outputElem);

        auto sortBy = sortByElem.Obj().getOwned();
        uassert(6789378, str::stream() << out->opName << " sortBy must not be empty", !sortBy.isEmpty());
        for (auto&& sortElem : sortBy) {
            uassert(6789379,
                    str::stream() << out->opName << " sortBy directions must be 1 or -1",
                    sortElem.isNumber() &&
                        (sortElem.numberInt() == 1 || sortElem.numberInt() == -1));
        }
        out->topBottomSortBy = sortBy;

        if (isTopBottomNOperator(out->opName)) {
            uassert(6789380,
                    str::stream() << out->opName << " requires positive integer n",
                    nElem.isNumber() && nElem.numberInt() == nElem.numberDouble() &&
                        nElem.numberInt() > 0);
            out->nValueCount = nElem.numberInt();
        } else {
            uassert(6789381, str::stream() << out->opName << " does not accept n", !nElem);
            out->nValueCount = 1;
        }

        VariablesParseState vps = expCtx->variablesParseState;
        out->argument = Expression::parseOperand(expCtx, outputElem, vps)->optimize();

        for (auto&& option : spec) {
            auto fieldName = option.fieldNameStringData();
            uassert(6789382,
                    str::stream() << "unknown $setWindowFields " << out->opName << " option '"
                                  << fieldName << "'",
                    fieldName == "sortBy"_sd || fieldName == "output"_sd ||
                        (isTopBottomNOperator(out->opName) && fieldName == "n"_sd));
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
            if (fieldName == "unit"_sd) {
                uassert(6789388,
                        "$setWindowFields unit is only valid with range windows",
                        out->windowType == WindowType::kRange);
                uassert(6789389,
                        "$setWindowFields range unit must be a string",
                        option.type() == BSONType::String);
                out->windowUnit = option.str();
                out->windowUnitMillis = unitMillis(out->windowUnit);
                if (out->lower.kind == BoundKind::kOffset) {
                    out->lower.offset *= out->windowUnitMillis;
                }
                if (out->upper.kind == BoundKind::kOffset) {
                    out->upper.offset *= out->windowUnitMillis;
                }
            }
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
        if (outSpec.opName == "$expMovingAvg") {
            return evaluateExpMovingAvg(outSpec, partitionStart, relativeIndex);
        }
        if (outSpec.opName == "$locf") {
            return evaluateLocf(outSpec, partitionStart, relativeIndex);
        }
        if (outSpec.opName == "$covariancePop" || outSpec.opName == "$covarianceSamp") {
            return evaluateCovariance(outSpec, partitionStart, first, last);
        }
        if (outSpec.opName == "$integral") {
            return evaluateIntegral(outSpec, partitionStart, first, last);
        }
        if (outSpec.opName == "$derivative") {
            return evaluateDerivative(outSpec, partitionStart, first, last);
        }
        if (isNValueOperator(outSpec.opName)) {
            return evaluateNValue(outSpec, partitionStart, first, last);
        }
        if (isPercentileOperator(outSpec.opName)) {
            return evaluatePercentile(outSpec, partitionStart, first, last);
        }
        if (isTopBottomOperator(outSpec.opName)) {
            return evaluateTopBottom(outSpec, partitionStart, first, last);
        }
        return evaluateWindow(outSpec, partitionStart, first, last);
    }

    Value evaluateExpMovingAvg(const OutputSpec& outSpec,
                               size_t partitionStart,
                               int relativeIndex) const {
        bool initialized = false;
        double average = 0;
        for (int i = 0; i <= relativeIndex; ++i) {
            auto value = outSpec.argument->evaluate(_buffer[partitionStart + i].doc);
            if (!value.numeric()) {
                continue;
            }
            const double current = value.coerceToDouble();
            if (!initialized) {
                average = current;
                initialized = true;
            } else {
                average = outSpec.expMovingAvgAlpha * current +
                    (1 - outSpec.expMovingAvgAlpha) * average;
            }
        }
        return initialized ? numericValue(average) : Value(BSONNULL);
    }

    Value evaluateLocf(const OutputSpec& outSpec, size_t partitionStart, int relativeIndex) const {
        Value last;
        bool haveValue = false;
        for (int i = 0; i <= relativeIndex; ++i) {
            auto value = outSpec.argument->evaluate(_buffer[partitionStart + i].doc);
            if (value.nullish()) {
                continue;
            }
            last = value;
            haveValue = true;
        }
        return haveValue ? last : Value(BSONNULL);
    }

    Value evaluateCovariance(const OutputSpec& outSpec,
                             size_t partitionStart,
                             int first,
                             int last) const {
        if (first > last) {
            return Value(BSONNULL);
        }

        double sumX = 0;
        double sumY = 0;
        double sumXY = 0;
        int count = 0;
        for (int i = first; i <= last; ++i) {
            auto pair = outSpec.argument->evaluate(_buffer[partitionStart + i].doc);
            if (!pair.isArray() || pair.getArray().size() != 2) {
                continue;
            }
            const auto& values = pair.getArray();
            if (!values[0].numeric() || !values[1].numeric()) {
                continue;
            }
            const double x = values[0].coerceToDouble();
            const double y = values[1].coerceToDouble();
            sumX += x;
            sumY += y;
            sumXY += x * y;
            ++count;
        }
        if (count == 0 || (outSpec.opName == "$covarianceSamp" && count < 2)) {
            return Value(BSONNULL);
        }
        const double covariance = sumXY - (sumX * sumY / count);
        const double denominator = outSpec.opName == "$covarianceSamp" ? count - 1 : count;
        return numericValue(covariance / denominator);
    }

    Value evaluateIntegral(const OutputSpec& outSpec,
                           size_t partitionStart,
                           int first,
                           int last) const {
        if (first >= last) {
            return Value(0);
        }

        bool havePrevious = false;
        double previousX = 0;
        double previousY = 0;
        double area = 0;
        for (int i = first; i <= last; ++i) {
            const double x = numericSortKey(partitionStart, i);
            auto input = outSpec.argument->evaluate(_buffer[partitionStart + i].doc);
            if (!input.numeric()) {
                continue;
            }
            const double y = input.coerceToDouble();
            if (havePrevious) {
                area += ((previousY + y) / 2.0) * (x - previousX);
            }
            previousX = x;
            previousY = y;
            havePrevious = true;
        }
        return numericValue(area);
    }

    Value evaluateDerivative(const OutputSpec& outSpec,
                             size_t partitionStart,
                             int first,
                             int last) const {
        if (first >= last) {
            return Value(BSONNULL);
        }

        bool haveFirst = false;
        double firstX = 0;
        double firstY = 0;
        bool haveLast = false;
        double lastX = 0;
        double lastY = 0;
        for (int i = first; i <= last; ++i) {
            const double x = numericSortKey(partitionStart, i);
            auto input = outSpec.argument->evaluate(_buffer[partitionStart + i].doc);
            if (!input.numeric()) {
                continue;
            }
            const double y = input.coerceToDouble();
            if (!haveFirst) {
                firstX = x;
                firstY = y;
                haveFirst = true;
            }
            lastX = x;
            lastY = y;
            haveLast = true;
        }
        if (!haveFirst || !haveLast || firstX == lastX) {
            return Value(BSONNULL);
        }
        return numericValue((lastY - firstY) / (lastX - firstX));
    }

    double numericSortKey(size_t partitionStart, int relativeIndex) const {
        uassert(6789362,
                "$setWindowFields integral/derivative require exactly one sortBy field",
                _sortDirections.size() == 1);
        auto sortKey = _buffer[partitionStart + relativeIndex].sortKeys[0];
        uassert(6789363,
                "$setWindowFields integral/derivative require numeric sort keys",
                sortKey.numeric());
        return sortKey.coerceToDouble();
    }

    Value evaluateNValue(const OutputSpec& outSpec,
                         size_t partitionStart,
                         int first,
                         int last) const {
        std::vector<Value> values;
        if (first <= last) {
            if (outSpec.opName == "$lastN") {
                first = std::max(first, last - outSpec.nValueCount + 1);
            }
            for (int i = first; i <= last; ++i) {
                values.push_back(outSpec.argument->evaluate(_buffer[partitionStart + i].doc));
            }
        }

        if (outSpec.opName == "$minN" || outSpec.opName == "$maxN") {
            std::sort(values.begin(), values.end(), [this, &outSpec](const auto& left,
                                                                     const auto& right) {
                auto cmp = pExpCtx->getValueComparator().compare(left, right);
                return outSpec.opName == "$minN" ? cmp < 0 : cmp > 0;
            });
        }

        if (static_cast<int>(values.size()) > outSpec.nValueCount) {
            values.resize(outSpec.nValueCount);
        }
        return Value(values);
    }

    Value evaluatePercentile(const OutputSpec& outSpec,
                             size_t partitionStart,
                             int first,
                             int last) const {
        std::vector<double> values;
        for (int i = first; i <= last; ++i) {
            auto value = outSpec.argument->evaluate(_buffer[partitionStart + i].doc);
            if (value.numeric()) {
                values.push_back(value.coerceToDouble());
            }
        }
        std::sort(values.begin(), values.end());

        std::vector<Value> percentileValues;
        percentileValues.reserve(outSpec.percentiles.size());
        for (double percentile : outSpec.percentiles) {
            percentileValues.push_back(interpolatePercentile(values, percentile));
        }
        if (outSpec.opName == "$median") {
            return percentileValues.empty() ? Value(BSONNULL) : percentileValues.front();
        }
        return Value(percentileValues);
    }

    Value evaluateTopBottom(const OutputSpec& outSpec,
                            size_t partitionStart,
                            int first,
                            int last) const {
        struct Candidate {
            std::vector<Value> sortKeys;
            Value output;
            size_t ordinal;
        };

        std::vector<int> directions;
        std::vector<FieldPath> sortFields;
        for (auto&& sortElem : outSpec.topBottomSortBy) {
            directions.push_back(sortElem.numberInt());
            sortFields.emplace_back(sortElem.fieldName());
        }

        std::vector<Candidate> candidates;
        size_t ordinal = 0;
        for (int i = first; i <= last; ++i) {
            const auto& doc = _buffer[partitionStart + i].doc;
            std::vector<Value> sortKeys;
            sortKeys.reserve(sortFields.size());
            for (auto&& field : sortFields) {
                sortKeys.push_back(doc.getNestedField(field));
            }
            candidates.push_back({std::move(sortKeys), outSpec.argument->evaluate(doc), ordinal++});
        }
        if (candidates.empty()) {
            return Value(BSONNULL);
        }

        const bool top = outSpec.opName == "$top" || outSpec.opName == "$topN";
        std::stable_sort(candidates.begin(), candidates.end(), [&](const Candidate& left,
                                                                   const Candidate& right) {
            for (size_t i = 0; i < directions.size(); ++i) {
                auto cmp = pExpCtx->getValueComparator().compare(left.sortKeys[i],
                                                                 right.sortKeys[i]) *
                    directions[i];
                if (cmp != 0) {
                    return top ? cmp < 0 : cmp > 0;
                }
            }
            return left.ordinal < right.ordinal;
        });

        if (!isTopBottomNOperator(outSpec.opName)) {
            return candidates.front().output;
        }

        std::vector<Value> output;
        const size_t count = std::min(candidates.size(), static_cast<size_t>(outSpec.nValueCount));
        output.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            output.push_back(candidates[i].output);
        }
        return Value(output);
    }

    static Value interpolatePercentile(const std::vector<double>& values, double percentile) {
        if (values.empty()) {
            return Value(BSONNULL);
        }
        if (values.size() == 1) {
            return numericValue(values.front());
        }

        double rank = percentile * static_cast<double>(values.size() - 1);
        size_t lower = static_cast<size_t>(std::floor(rank));
        size_t upper = static_cast<size_t>(std::ceil(rank));
        if (lower == upper) {
            return numericValue(values[lower]);
        }

        double fraction = rank - static_cast<double>(lower);
        return numericValue(values[lower] + (values[upper] - values[lower]) * fraction);
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
        const double current = rangeSortKeyValue(currentKey, !outSpec.windowUnit.empty());
        const double lower = rangeBoundary(outSpec.lower, current, true);
        const double upper = rangeBoundary(outSpec.upper, current, false);
        if (lower > upper) {
            return {1, 0};
        }

        int first = -1;
        int last = -2;
        for (int i = 0; i < static_cast<int>(partitionEnd - partitionStart); ++i) {
            auto sortKey = _buffer[partitionStart + i].sortKeys[0];
            const double value = rangeSortKeyValue(sortKey, !outSpec.windowUnit.empty());
            if (value >= lower && value <= upper) {
                if (first < 0) {
                    first = i;
                }
                last = i;
            }
        }
        return {first < 0 ? 1 : first, last};
    }

    static double rangeSortKeyValue(const Value& value, bool dateMode) {
        if (dateMode) {
            uassert(6789347,
                    "$setWindowFields range windows with unit require date sort keys",
                    value.getType() == BSONType::Date);
            return static_cast<double>(value.getDate().toMillisSinceEpoch());
        }
        uassert(6789348,
                "$setWindowFields range windows require numeric sort keys",
                value.numeric());
        return value.coerceToDouble();
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

    static double unitMillis(const std::string& unit) {
        if (unit == "millisecond") {
            return 1;
        }
        if (unit == "second") {
            return 1000;
        }
        if (unit == "minute") {
            return 60 * 1000;
        }
        if (unit == "hour") {
            return 60 * 60 * 1000;
        }
        if (unit == "day") {
            return 24 * 60 * 60 * 1000;
        }
        if (unit == "week") {
            return 7 * 24 * 60 * 60 * 1000;
        }
        uasserted(6789390,
                  "$setWindowFields range unit must be millisecond, second, minute, hour, day, or week");
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
