/**
 *    Copyright (C) 2018-present MongoDB, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the Server Side Public License, version 1,
 *    as published by MongoDB, Inc.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Server Side Public License for more details.
 *
 *    You should have received a copy of the Server Side Public License
 *    along with this program. If not, see
 *    <http://www.mongodb.com/licensing/server-side-public-license>.
 */

#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kDefault

#include "mongo/platform/basic.h"

#include "mongo/db/pipeline/expression.h"

#include <cmath>
#include <limits>

namespace mongo {
namespace {

template <typename DoubleFn, typename DecimalFn>
Value evaluateUnboundedTrig(const Value& numericArg, DoubleFn doubleFn, DecimalFn decimalFn) {
    if (numericArg.getType() == NumberDecimal) {
        return Value(decimalFn(numericArg.getDecimal()));
    }
    return Value(doubleFn(numericArg.coerceToDouble()));
}

template <typename DoubleFn, typename DecimalFn>
Value evaluateBoundedTrig(const char* opName,
                          const Value& numericArg,
                          double lowerBound,
                          bool lowerInclusive,
                          double upperBound,
                          bool upperInclusive,
                          DoubleFn doubleFn,
                          DecimalFn decimalFn) {
    if (numericArg.getType() == NumberDecimal) {
        auto input = numericArg.getDecimal();
        if (input.isNaN()) {
            return numericArg;
        }

        auto lower = Decimal128(lowerBound);
        auto upper = Decimal128(upperBound);
        bool lowerOk = lowerInclusive ? input.isGreaterEqual(lower) : input.isGreater(lower);
        bool upperOk = upperInclusive ? input.isLessEqual(upper) : input.isLess(upper);
        uassert(51090,
                str::stream() << "cannot apply " << opName << " to " << input.toString()
                              << ", value must be in " << (lowerInclusive ? "[" : "(")
                              << lowerBound << "," << upperBound
                              << (upperInclusive ? "]" : ")"),
                lowerOk && upperOk);
        return Value(decimalFn(input));
    }

    auto input = numericArg.coerceToDouble();
    if (std::isnan(input)) {
        return numericArg;
    }

    bool lowerOk = lowerInclusive ? input >= lowerBound : input > lowerBound;
    bool upperOk = upperInclusive ? input <= upperBound : input < upperBound;
    uassert(51091,
            str::stream() << "cannot apply " << opName << " to " << input
                          << ", value must be in " << (lowerInclusive ? "[" : "(")
                          << lowerBound << "," << upperBound
                          << (upperInclusive ? "]" : ")"),
            lowerOk && upperOk);
    return Value(doubleFn(input));
}

Value evaluateDegreeRadiansConversion(const Value& numericArg,
                                      Decimal128 decimalFactor,
                                      double doubleFactor) {
    if (numericArg.getType() == NumberDecimal) {
        return Value(numericArg.getDecimal().multiply(decimalFactor));
    }
    return Value(numericArg.coerceToDouble() * doubleFactor);
}

#define DECLARE_UNBOUNDED_TRIG_CLASS(className, opName, doubleFunc, decimalFunc)         \
    class className final : public ExpressionSingleNumericArg<className> {               \
    public:                                                                              \
        explicit className(const boost::intrusive_ptr<ExpressionContext>& expCtx)         \
            : ExpressionSingleNumericArg<className>(expCtx) {}                           \
        Value evaluateNumericArg(const Value& numericArg) const final {                  \
            return evaluateUnboundedTrig(                                                \
                numericArg,                                                              \
                [](double arg) { return doubleFunc(arg); },                              \
                [](const Decimal128& arg) { return arg.decimalFunc(); });                \
        }                                                                                \
        const char* getOpName() const final {                                            \
            return opName;                                                               \
        }                                                                                \
    };

#define DECLARE_BOUNDED_TRIG_CLASS(                                                     \
    className, opName, lower, lowerInc, upper, upperInc, doubleFunc, decimalFunc)        \
    class className final : public ExpressionSingleNumericArg<className> {               \
    public:                                                                              \
        explicit className(const boost::intrusive_ptr<ExpressionContext>& expCtx)         \
            : ExpressionSingleNumericArg<className>(expCtx) {}                           \
        Value evaluateNumericArg(const Value& numericArg) const final {                  \
            return evaluateBoundedTrig(                                                  \
                opName,                                                                  \
                numericArg,                                                              \
                lower,                                                                   \
                lowerInc,                                                                \
                upper,                                                                   \
                upperInc,                                                                \
                [](double arg) { return doubleFunc(arg); },                              \
                [](const Decimal128& arg) { return arg.decimalFunc(); });                \
        }                                                                                \
        const char* getOpName() const final {                                            \
            return opName;                                                               \
        }                                                                                \
    };

DECLARE_BOUNDED_TRIG_CLASS(ExpressionSine,
                           "$sin",
                           -std::numeric_limits<double>::infinity(),
                           false,
                           std::numeric_limits<double>::infinity(),
                           false,
                           std::sin,
                           sin)
DECLARE_BOUNDED_TRIG_CLASS(ExpressionCosine,
                           "$cos",
                           -std::numeric_limits<double>::infinity(),
                           false,
                           std::numeric_limits<double>::infinity(),
                           false,
                           std::cos,
                           cos)
DECLARE_BOUNDED_TRIG_CLASS(ExpressionTangent,
                           "$tan",
                           -std::numeric_limits<double>::infinity(),
                           false,
                           std::numeric_limits<double>::infinity(),
                           false,
                           std::tan,
                           tan)
DECLARE_BOUNDED_TRIG_CLASS(
    ExpressionArcSine, "$asin", -1.0, true, 1.0, true, std::asin, asin)
DECLARE_BOUNDED_TRIG_CLASS(
    ExpressionArcCosine, "$acos", -1.0, true, 1.0, true, std::acos, acos)
DECLARE_UNBOUNDED_TRIG_CLASS(ExpressionArcTangent, "$atan", std::atan, atan)
DECLARE_UNBOUNDED_TRIG_CLASS(ExpressionHyperbolicSine, "$sinh", std::sinh, sinh)
DECLARE_UNBOUNDED_TRIG_CLASS(ExpressionHyperbolicCosine, "$cosh", std::cosh, cosh)
DECLARE_UNBOUNDED_TRIG_CLASS(ExpressionHyperbolicTangent, "$tanh", std::tanh, tanh)
DECLARE_UNBOUNDED_TRIG_CLASS(ExpressionHyperbolicArcSine, "$asinh", std::asinh, asinh)
DECLARE_BOUNDED_TRIG_CLASS(ExpressionHyperbolicArcCosine,
                           "$acosh",
                           1.0,
                           true,
                           std::numeric_limits<double>::infinity(),
                           false,
                           std::acosh,
                           acosh)
DECLARE_BOUNDED_TRIG_CLASS(
    ExpressionHyperbolicArcTangent, "$atanh", -1.0, true, 1.0, true, std::atanh, atanh)

#undef DECLARE_BOUNDED_TRIG_CLASS
#undef DECLARE_UNBOUNDED_TRIG_CLASS

class ExpressionArcTangent2 final : public ExpressionFixedArity<ExpressionArcTangent2, 2> {
public:
    explicit ExpressionArcTangent2(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionFixedArity<ExpressionArcTangent2, 2>(expCtx) {}

    Value evaluate(const Document& root) const final {
        auto first = vpOperand[0]->evaluate(root);
        if (first.nullish()) {
            return Value(BSONNULL);
        }
        uassert(51092,
                str::stream() << getOpName() << " only supports numeric types, not "
                              << typeName(first.getType()),
                first.numeric());

        auto second = vpOperand[1]->evaluate(root);
        if (second.nullish()) {
            return Value(BSONNULL);
        }
        uassert(51093,
                str::stream() << getOpName() << " only supports numeric types, not "
                              << typeName(second.getType()),
                second.numeric());

        if (first.getType() == NumberDecimal || second.getType() == NumberDecimal) {
            return Value(first.coerceToDecimal().atan2(second.coerceToDecimal()));
        }
        return Value(std::atan2(first.coerceToDouble(), second.coerceToDouble()));
    }

    const char* getOpName() const final {
        return "$atan2";
    }
};

class ExpressionDegreesToRadians final
    : public ExpressionSingleNumericArg<ExpressionDegreesToRadians> {
public:
    explicit ExpressionDegreesToRadians(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionSingleNumericArg<ExpressionDegreesToRadians>(expCtx) {}

    Value evaluateNumericArg(const Value& numericArg) const final {
        static const double kDoublePiOver180 = 3.141592653589793 / 180.0;
        return evaluateDegreeRadiansConversion(
            numericArg, Decimal128::kPiOver180, kDoublePiOver180);
    }

    const char* getOpName() const final {
        return "$degreesToRadians";
    }
};

class ExpressionRadiansToDegrees final
    : public ExpressionSingleNumericArg<ExpressionRadiansToDegrees> {
public:
    explicit ExpressionRadiansToDegrees(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionSingleNumericArg<ExpressionRadiansToDegrees>(expCtx) {}

    Value evaluateNumericArg(const Value& numericArg) const final {
        static const double kDouble180OverPi = 180.0 / 3.141592653589793;
        return evaluateDegreeRadiansConversion(
            numericArg, Decimal128::k180OverPi, kDouble180OverPi);
    }

    const char* getOpName() const final {
        return "$radiansToDegrees";
    }
};

}  // namespace

REGISTER_EXPRESSION(sin, ExpressionSine::parse);
REGISTER_EXPRESSION(cos, ExpressionCosine::parse);
REGISTER_EXPRESSION(tan, ExpressionTangent::parse);
REGISTER_EXPRESSION(asin, ExpressionArcSine::parse);
REGISTER_EXPRESSION(acos, ExpressionArcCosine::parse);
REGISTER_EXPRESSION(atan, ExpressionArcTangent::parse);
REGISTER_EXPRESSION(atan2, ExpressionArcTangent2::parse);
REGISTER_EXPRESSION(sinh, ExpressionHyperbolicSine::parse);
REGISTER_EXPRESSION(cosh, ExpressionHyperbolicCosine::parse);
REGISTER_EXPRESSION(tanh, ExpressionHyperbolicTangent::parse);
REGISTER_EXPRESSION(asinh, ExpressionHyperbolicArcSine::parse);
REGISTER_EXPRESSION(acosh, ExpressionHyperbolicArcCosine::parse);
REGISTER_EXPRESSION(atanh, ExpressionHyperbolicArcTangent::parse);
REGISTER_EXPRESSION(degreesToRadians, ExpressionDegreesToRadians::parse);
REGISTER_EXPRESSION(radiansToDegrees, ExpressionRadiansToDegrees::parse);

}  // namespace mongo
