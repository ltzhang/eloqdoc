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

boost::intrusive_ptr<Expression> parseCurrentExpression(
    const boost::intrusive_ptr<ExpressionContext>& expCtx, const VariablesParseState& vps) {
    auto obj = BSON("" << "$$CURRENT");
    return Expression::parseOperand(expCtx, obj.firstElement(), vps);
}

std::string evaluateFieldName(StringData opName,
                              const boost::intrusive_ptr<Expression>& field,
                              const Document& root) {
    auto fieldValue = field->evaluate(root);
    uassert(ErrorCodes::TypeMismatch,
            str::stream() << opName << " requires 'field' to evaluate to a string, got "
                          << typeName(fieldValue.getType()),
            fieldValue.getType() == BSONType::String);
    return fieldValue.getString();
}

Value evaluateInputObject(StringData opName,
                          const boost::intrusive_ptr<Expression>& input,
                          const Document& root) {
    auto inputValue = input->evaluate(root);
    if (inputValue.nullish()) {
        return Value(BSONNULL);
    }

    uassert(ErrorCodes::TypeMismatch,
            str::stream() << opName << " requires 'input' to evaluate to an object, got "
                          << typeName(inputValue.getType()),
            inputValue.getType() == BSONType::Object);
    return inputValue;
}

class ExpressionGetField final : public Expression {
public:
    ExpressionGetField(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                       boost::intrusive_ptr<Expression> field,
                       boost::intrusive_ptr<Expression> input)
        : Expression(expCtx), _field(std::move(field)), _input(std::move(input)) {}

    static boost::intrusive_ptr<Expression> parse(
        const boost::intrusive_ptr<ExpressionContext>& expCtx,
        BSONElement expr,
        const VariablesParseState& vps) {
        boost::intrusive_ptr<Expression> field;
        boost::intrusive_ptr<Expression> input = parseCurrentExpression(expCtx, vps);

        if (expr.type() == BSONType::Object) {
            for (auto&& elem : expr.Obj()) {
                auto name = elem.fieldNameStringData();
                if (name == "field"_sd) {
                    field = Expression::parseOperand(expCtx, elem, vps);
                } else if (name == "input"_sd) {
                    input = Expression::parseOperand(expCtx, elem, vps);
                } else {
                    uasserted(ErrorCodes::FailedToParse,
                              str::stream() << "$getField found unknown argument: " << name);
                }
            }
        } else {
            field = Expression::parseOperand(expCtx, expr, vps);
        }

        uassert(ErrorCodes::FailedToParse, "$getField requires 'field' to be specified", field);
        return new ExpressionGetField(expCtx, field, input);
    }

    boost::intrusive_ptr<Expression> optimize() final {
        _field = _field->optimize();
        _input = _input->optimize();
        return this;
    }

    Value evaluate(const Document& root) const final {
        auto inputValue = evaluateInputObject("$getField"_sd, _input, root);
        if (inputValue.nullish()) {
            return Value(BSONNULL);
        }

        auto fieldName = evaluateFieldName("$getField"_sd, _field, root);
        return inputValue.getDocument().getField(fieldName);
    }

    Value serialize(bool explain) const final {
        return Value(DOC("$getField" << DOC("field" << _field->serialize(explain) << "input"
                                                  << _input->serialize(explain))));
    }

private:
    void _doAddDependencies(DepsTracker* deps) const final {
        _field->addDependencies(deps);
        _input->addDependencies(deps);
    }

    boost::intrusive_ptr<Expression> _field;
    boost::intrusive_ptr<Expression> _input;
};

class ExpressionSetField final : public Expression {
public:
    ExpressionSetField(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                       boost::intrusive_ptr<Expression> field,
                       boost::intrusive_ptr<Expression> input,
                       boost::intrusive_ptr<Expression> value)
        : Expression(expCtx),
          _field(std::move(field)),
          _input(std::move(input)),
          _value(std::move(value)) {}

    static boost::intrusive_ptr<Expression> parse(
        const boost::intrusive_ptr<ExpressionContext>& expCtx,
        BSONElement expr,
        const VariablesParseState& vps) {
        uassert(ErrorCodes::FailedToParse,
                "$setField requires an object argument",
                expr.type() == BSONType::Object);

        boost::intrusive_ptr<Expression> field;
        boost::intrusive_ptr<Expression> input;
        boost::intrusive_ptr<Expression> value;

        for (auto&& elem : expr.Obj()) {
            auto name = elem.fieldNameStringData();
            if (name == "field"_sd) {
                field = Expression::parseOperand(expCtx, elem, vps);
            } else if (name == "input"_sd) {
                input = Expression::parseOperand(expCtx, elem, vps);
            } else if (name == "value"_sd) {
                value = Expression::parseOperand(expCtx, elem, vps);
            } else {
                uasserted(ErrorCodes::FailedToParse,
                          str::stream() << "$setField found unknown argument: " << name);
            }
        }

        uassert(ErrorCodes::FailedToParse, "$setField requires 'field' to be specified", field);
        uassert(ErrorCodes::FailedToParse, "$setField requires 'input' to be specified", input);
        uassert(ErrorCodes::FailedToParse, "$setField requires 'value' to be specified", value);
        return new ExpressionSetField(expCtx, field, input, value);
    }

    boost::intrusive_ptr<Expression> optimize() final {
        _field = _field->optimize();
        _input = _input->optimize();
        _value = _value->optimize();
        return this;
    }

    Value evaluate(const Document& root) const final {
        auto inputValue = evaluateInputObject("$setField"_sd, _input, root);
        if (inputValue.nullish()) {
            return Value(BSONNULL);
        }

        auto fieldName = evaluateFieldName("$setField"_sd, _field, root);
        auto value = _value->evaluate(root);

        MutableDocument output(inputValue.getDocument());
        output.setField(fieldName, value);
        return output.freezeToValue();
    }

    Value serialize(bool explain) const final {
        return Value(DOC("$setField" << DOC("field" << _field->serialize(explain) << "input"
                                                  << _input->serialize(explain) << "value"
                                                  << _value->serialize(explain))));
    }

private:
    void _doAddDependencies(DepsTracker* deps) const final {
        _field->addDependencies(deps);
        _input->addDependencies(deps);
        _value->addDependencies(deps);
    }

    boost::intrusive_ptr<Expression> _field;
    boost::intrusive_ptr<Expression> _input;
    boost::intrusive_ptr<Expression> _value;
};

class ExpressionUnsetField final : public Expression {
public:
    ExpressionUnsetField(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                         boost::intrusive_ptr<Expression> field,
                         boost::intrusive_ptr<Expression> input)
        : Expression(expCtx), _field(std::move(field)), _input(std::move(input)) {}

    static boost::intrusive_ptr<Expression> parse(
        const boost::intrusive_ptr<ExpressionContext>& expCtx,
        BSONElement expr,
        const VariablesParseState& vps) {
        uassert(ErrorCodes::FailedToParse,
                "$unsetField requires an object argument",
                expr.type() == BSONType::Object);

        boost::intrusive_ptr<Expression> field;
        boost::intrusive_ptr<Expression> input;

        for (auto&& elem : expr.Obj()) {
            auto name = elem.fieldNameStringData();
            if (name == "field"_sd) {
                field = Expression::parseOperand(expCtx, elem, vps);
            } else if (name == "input"_sd) {
                input = Expression::parseOperand(expCtx, elem, vps);
            } else {
                uasserted(ErrorCodes::FailedToParse,
                          str::stream() << "$unsetField found unknown argument: " << name);
            }
        }

        uassert(ErrorCodes::FailedToParse, "$unsetField requires 'field' to be specified", field);
        uassert(ErrorCodes::FailedToParse, "$unsetField requires 'input' to be specified", input);
        return new ExpressionUnsetField(expCtx, field, input);
    }

    boost::intrusive_ptr<Expression> optimize() final {
        _field = _field->optimize();
        _input = _input->optimize();
        return this;
    }

    Value evaluate(const Document& root) const final {
        auto inputValue = evaluateInputObject("$unsetField"_sd, _input, root);
        if (inputValue.nullish()) {
            return Value(BSONNULL);
        }

        auto fieldName = evaluateFieldName("$unsetField"_sd, _field, root);

        MutableDocument output(inputValue.getDocument());
        output.remove(fieldName);
        return output.freezeToValue();
    }

    Value serialize(bool explain) const final {
        return Value(DOC("$unsetField" << DOC("field" << _field->serialize(explain) << "input"
                                                    << _input->serialize(explain))));
    }

private:
    void _doAddDependencies(DepsTracker* deps) const final {
        _field->addDependencies(deps);
        _input->addDependencies(deps);
    }

    boost::intrusive_ptr<Expression> _field;
    boost::intrusive_ptr<Expression> _input;
};

}  // namespace

REGISTER_EXPRESSION(getField, ExpressionGetField::parse);
REGISTER_EXPRESSION(setField, ExpressionSetField::parse);
REGISTER_EXPRESSION(unsetField, ExpressionUnsetField::parse);

}  // namespace mongo
