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

namespace mongo {
namespace {

struct IntegralValue {
    long long value = 0;
    bool isLong = false;
};

IntegralValue requireIntegralValue(StringData opName, const Value& value) {
    uassert(ErrorCodes::TypeMismatch,
            str::stream() << opName << " only supports int and long, not "
                          << typeName(value.getType()),
            value.getType() == BSONType::NumberInt || value.getType() == BSONType::NumberLong);

    if (value.getType() == BSONType::NumberLong) {
        return {value.getLong(), true};
    }
    return {value.getInt(), false};
}

template <typename Op>
Value evaluateBitwiseNary(StringData opName,
                          const std::vector<boost::intrusive_ptr<Expression>>& operands,
                          long long identity,
                          Op op,
                          const Document& root) {
    long long result = identity;
    bool hasLong = false;

    for (auto&& operand : operands) {
        auto value = operand->evaluate(root);
        if (value.nullish()) {
            return Value(BSONNULL);
        }

        auto integral = requireIntegralValue(opName, value);
        hasLong = hasLong || integral.isLong;
        result = op(result, integral.value);
    }

    return hasLong ? Value(result) : Value(static_cast<int>(result));
}

template <typename SubClass>
class ExpressionBitwiseNary : public ExpressionVariadic<SubClass> {
public:
    explicit ExpressionBitwiseNary(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionVariadic<SubClass>(expCtx) {}

    bool isAssociative() const final {
        return true;
    }

    bool isCommutative() const final {
        return true;
    }
};

class ExpressionBitAnd final : public ExpressionBitwiseNary<ExpressionBitAnd> {
public:
    explicit ExpressionBitAnd(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionBitwiseNary<ExpressionBitAnd>(expCtx) {}

    Value evaluate(const Document& root) const final {
        return evaluateBitwiseNary(
            "$bitAnd"_sd, vpOperand, -1LL, [](long long a, long long b) { return a & b; }, root);
    }

    const char* getOpName() const final {
        return "$bitAnd";
    }
};

class ExpressionBitOr final : public ExpressionBitwiseNary<ExpressionBitOr> {
public:
    explicit ExpressionBitOr(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionBitwiseNary<ExpressionBitOr>(expCtx) {}

    Value evaluate(const Document& root) const final {
        return evaluateBitwiseNary(
            "$bitOr"_sd, vpOperand, 0LL, [](long long a, long long b) { return a | b; }, root);
    }

    const char* getOpName() const final {
        return "$bitOr";
    }
};

class ExpressionBitXor final : public ExpressionBitwiseNary<ExpressionBitXor> {
public:
    explicit ExpressionBitXor(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionBitwiseNary<ExpressionBitXor>(expCtx) {}

    Value evaluate(const Document& root) const final {
        return evaluateBitwiseNary(
            "$bitXor"_sd, vpOperand, 0LL, [](long long a, long long b) { return a ^ b; }, root);
    }

    const char* getOpName() const final {
        return "$bitXor";
    }
};

class ExpressionBitNot final : public ExpressionFixedArity<ExpressionBitNot, 1> {
public:
    explicit ExpressionBitNot(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionFixedArity<ExpressionBitNot, 1>(expCtx) {}

    Value evaluate(const Document& root) const final {
        auto value = vpOperand[0]->evaluate(root);
        if (value.nullish()) {
            return Value(BSONNULL);
        }

        auto integral = requireIntegralValue("$bitNot"_sd, value);
        if (integral.isLong) {
            return Value(~integral.value);
        }
        return Value(~static_cast<int>(integral.value));
    }

    const char* getOpName() const final {
        return "$bitNot";
    }
};

}  // namespace

REGISTER_EXPRESSION(bitAnd, ExpressionBitAnd::parse);
REGISTER_EXPRESSION(bitOr, ExpressionBitOr::parse);
REGISTER_EXPRESSION(bitXor, ExpressionBitXor::parse);
REGISTER_EXPRESSION(bitNot, ExpressionBitNot::parse);

}  // namespace mongo
