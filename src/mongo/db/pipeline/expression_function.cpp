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

#include "mongo/db/pipeline/document.h"

namespace mongo {
namespace {

using boost::intrusive_ptr;

class ExpressionFunction final : public Expression {
public:
    explicit ExpressionFunction(const intrusive_ptr<ExpressionContext>& expCtx) : Expression(expCtx) {}

    static intrusive_ptr<Expression> parse(const intrusive_ptr<ExpressionContext>& expCtx,
                                           BSONElement expr,
                                           const VariablesParseState& vps) {
        uassert(51220, "$function requires an object argument", expr.type() == BSONType::Object);

        intrusive_ptr<ExpressionFunction> out(new ExpressionFunction(expCtx));
        bool sawBody = false;
        bool sawArgs = false;
        bool sawLang = false;
        for (auto&& elem : expr.Obj()) {
            auto fieldName = elem.fieldNameStringData();
            if (fieldName == "body") {
                sawBody = true;
                out->_body = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "args") {
                sawArgs = true;
                out->_args = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "lang") {
                sawLang = true;
                out->_lang = Expression::parseOperand(expCtx, elem, vps);
            } else {
                uasserted(51221,
                          str::stream() << "$function found unknown argument '" << fieldName
                                        << "'");
            }
        }

        uassert(51222, "$function requires 'body'", sawBody);
        uassert(51223, "$function requires 'args'", sawArgs);
        uassert(51224, "$function requires 'lang'", sawLang);
        return out;
    }

    Value evaluate(const Document& root) const final {
        uasserted(ErrorCodes::NoSuchKey, "server-side scripting disabled for $function in EloqDoc");
    }

    Value serialize(bool explain) const final {
        MutableDocument spec;
        spec.addField("body", _body->serialize(explain));
        spec.addField("args", _args->serialize(explain));
        spec.addField("lang", _lang->serialize(explain));
        return Value(Document{{"$function", spec.freezeToValue()}});
    }

    void _doAddDependencies(DepsTracker* deps) const final {
        _body->addDependencies(deps);
        _args->addDependencies(deps);
        _lang->addDependencies(deps);
    }

    const char* getOpName() const {
        return "$function";
    }

private:
    intrusive_ptr<Expression> _body;
    intrusive_ptr<Expression> _args;
    intrusive_ptr<Expression> _lang;
};

}  // namespace

REGISTER_EXPRESSION(function, ExpressionFunction::parse);

}  // namespace mongo
