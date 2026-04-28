/**
 * Copyright (C) 2016 MongoDB Inc.
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
 *
 * As a special exception, the copyright holders give permission to link the
 * code of portions of this program with the OpenSSL library under certain
 * conditions as described in each individual source file and distribute
 * linked combinations including the program with the OpenSSL library. You
 * must comply with the GNU Affero General Public License in all respects
 * for all of the code used other than as permitted herein. If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so. If you do not
 * wish to do so, delete this exception statement from your version. If you
 * delete this exception statement from all source files in the program,
 * then also delete it in the license file.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/pipeline/expression.h"

#include <cmath>
#include <limits>

namespace mongo {
namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;

Value numericResultLikeInput(const Value& input, double result) {
    if (input.getType() == BSONType::NumberDecimal) {
        return Value(Decimal128(result, Decimal128::kRoundTo15Digits));
    }
    return Value(result);
}

double numericToDouble(const Value& input) {
    return input.getType() == BSONType::NumberDecimal ? input.getDecimal().toDouble()
                                                      : input.coerceToDouble();
}

void checkDomain(StringData opName,
                 double value,
                 double lower,
                 bool lowerInclusive,
                 double upper,
                 bool upperInclusive) {
    if (std::isnan(value)) {
        return;
    }

    bool aboveLower = lowerInclusive ? value >= lower : value > lower;
    bool belowUpper = upperInclusive ? value <= upper : value < upper;
    uassert(51091,
            str::stream() << opName << " argument is outside its numeric domain",
            aboveLower && belowUpper);
}

Value evaluateUnaryTrig(StringData opName,
                        const Value& input,
                        double (*mathFn)(double),
                        double lower,
                        bool lowerInclusive,
                        double upper,
                        bool upperInclusive) {
    double asDouble = numericToDouble(input);
    checkDomain(opName, asDouble, lower, lowerInclusive, upper, upperInclusive);
    return numericResultLikeInput(input, mathFn(asDouble));
}

Value evaluateDegreeConversion(const Value& input, double factor) {
    if (input.getType() == BSONType::NumberDecimal) {
        Decimal128 decimalFactor(factor, Decimal128::kRoundTo15Digits);
        return Value(input.getDecimal().multiply(decimalFactor));
    }
    return Value(input.coerceToDouble() * factor);
}

template <typename SubClass>
class ExpressionTrigBase : public ExpressionSingleNumericArg<SubClass> {
public:
    explicit ExpressionTrigBase(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionSingleNumericArg<SubClass>(expCtx) {}

protected:
    Value apply(const Value& input,
                double (*mathFn)(double),
                double lower = -std::numeric_limits<double>::infinity(),
                bool lowerInclusive = false,
                double upper = std::numeric_limits<double>::infinity(),
                bool upperInclusive = false) const {
        return evaluateUnaryTrig(
            this->getOpName(), input, mathFn, lower, lowerInclusive, upper, upperInclusive);
    }
};

class ExpressionSine final : public ExpressionTrigBase<ExpressionSine> {
public:
    explicit ExpressionSine(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionSine>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::sin);
    }
    const char* getOpName() const final {
        return "$sin";
    }
};

class ExpressionCosine final : public ExpressionTrigBase<ExpressionCosine> {
public:
    explicit ExpressionCosine(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionCosine>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::cos);
    }
    const char* getOpName() const final {
        return "$cos";
    }
};

class ExpressionTangent final : public ExpressionTrigBase<ExpressionTangent> {
public:
    explicit ExpressionTangent(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionTangent>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::tan);
    }
    const char* getOpName() const final {
        return "$tan";
    }
};

class ExpressionArcSine final : public ExpressionTrigBase<ExpressionArcSine> {
public:
    explicit ExpressionArcSine(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionArcSine>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::asin, -1.0, true, 1.0, true);
    }
    const char* getOpName() const final {
        return "$asin";
    }
};

class ExpressionArcCosine final : public ExpressionTrigBase<ExpressionArcCosine> {
public:
    explicit ExpressionArcCosine(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionArcCosine>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::acos, -1.0, true, 1.0, true);
    }
    const char* getOpName() const final {
        return "$acos";
    }
};

class ExpressionArcTangent final : public ExpressionTrigBase<ExpressionArcTangent> {
public:
    explicit ExpressionArcTangent(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionArcTangent>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::atan);
    }
    const char* getOpName() const final {
        return "$atan";
    }
};

class ExpressionHyperbolicSine final : public ExpressionTrigBase<ExpressionHyperbolicSine> {
public:
    explicit ExpressionHyperbolicSine(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionHyperbolicSine>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::sinh);
    }
    const char* getOpName() const final {
        return "$sinh";
    }
};

class ExpressionHyperbolicCosine final : public ExpressionTrigBase<ExpressionHyperbolicCosine> {
public:
    explicit ExpressionHyperbolicCosine(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionHyperbolicCosine>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::cosh);
    }
    const char* getOpName() const final {
        return "$cosh";
    }
};

class ExpressionHyperbolicTangent final : public ExpressionTrigBase<ExpressionHyperbolicTangent> {
public:
    explicit ExpressionHyperbolicTangent(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionHyperbolicTangent>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::tanh);
    }
    const char* getOpName() const final {
        return "$tanh";
    }
};

class ExpressionHyperbolicArcSine final
    : public ExpressionTrigBase<ExpressionHyperbolicArcSine> {
public:
    explicit ExpressionHyperbolicArcSine(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionHyperbolicArcSine>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::asinh);
    }
    const char* getOpName() const final {
        return "$asinh";
    }
};

class ExpressionHyperbolicArcCosine final
    : public ExpressionTrigBase<ExpressionHyperbolicArcCosine> {
public:
    explicit ExpressionHyperbolicArcCosine(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionHyperbolicArcCosine>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::acosh, 1.0, true);
    }
    const char* getOpName() const final {
        return "$acosh";
    }
};

class ExpressionHyperbolicArcTangent final
    : public ExpressionTrigBase<ExpressionHyperbolicArcTangent> {
public:
    explicit ExpressionHyperbolicArcTangent(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionTrigBase<ExpressionHyperbolicArcTangent>(expCtx) {}
    Value evaluateNumericArg(const Value& input) const final {
        return apply(input, std::atanh, -1.0, false, 1.0, false);
    }
    const char* getOpName() const final {
        return "$atanh";
    }
};

class ExpressionArcTangent2 final : public ExpressionFixedArity<ExpressionArcTangent2, 2> {
public:
    explicit ExpressionArcTangent2(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionFixedArity<ExpressionArcTangent2, 2>(expCtx) {}

    Value evaluate(const Document& root) const final {
        auto y = vpOperand[0]->evaluate(root);
        if (y.nullish()) {
            return Value(BSONNULL);
        }
        uassert(51092,
                str::stream() << getOpName() << " only supports numeric types, not "
                              << typeName(y.getType()),
                y.numeric());

        auto x = vpOperand[1]->evaluate(root);
        if (x.nullish()) {
            return Value(BSONNULL);
        }
        uassert(51093,
                str::stream() << getOpName() << " only supports numeric types, not "
                              << typeName(x.getType()),
                x.numeric());

        double result = std::atan2(numericToDouble(y), numericToDouble(x));
        return (y.getType() == BSONType::NumberDecimal || x.getType() == BSONType::NumberDecimal)
            ? Value(Decimal128(result, Decimal128::kRoundTo15Digits))
            : Value(result);
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

    Value evaluateNumericArg(const Value& input) const final {
        return evaluateDegreeConversion(input, kPi / 180.0);
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

    Value evaluateNumericArg(const Value& input) const final {
        return evaluateDegreeConversion(input, 180.0 / kPi);
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
