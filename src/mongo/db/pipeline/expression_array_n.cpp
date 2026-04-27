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

#include <algorithm>

#include "mongo/db/pipeline/document.h"
#include "mongo/db/pipeline/field_path.h"
#include "mongo/db/pipeline/value_comparator.h"

namespace mongo {
namespace {

using boost::intrusive_ptr;

int parseSortDirection(StringData opName, BSONElement elem) {
    uassert(51200,
            str::stream() << opName << " sort direction must be numeric",
            elem.isNumber());
    int direction = elem.numberInt();
    uassert(51201,
            str::stream() << opName << " sort direction must be 1 or -1",
            direction == 1 || direction == -1);
    return direction;
}

struct SortPart {
    FieldPath path;
    int direction;
};

Value extractPathValue(const Value& input, const FieldPath& path) {
    if (input.getType() != BSONType::Object) {
        return Value(BSONNULL);
    }
    auto value = input.getDocument().getNestedField(path);
    return value.missing() ? Value(BSONNULL) : value;
}

class ExpressionSortArray final : public Expression {
public:
    explicit ExpressionSortArray(const intrusive_ptr<ExpressionContext>& expCtx)
        : Expression(expCtx) {}

    static intrusive_ptr<Expression> parse(const intrusive_ptr<ExpressionContext>& expCtx,
                                           BSONElement expr,
                                           const VariablesParseState& vps) {
        uassert(51202, "$sortArray requires an object argument", expr.type() == BSONType::Object);

        intrusive_ptr<ExpressionSortArray> out(new ExpressionSortArray(expCtx));
        bool sawSortBy = false;
        for (auto&& elem : expr.Obj()) {
            auto fieldName = elem.fieldNameStringData();
            if (fieldName == "input") {
                out->_input = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "sortBy") {
                sawSortBy = true;
                if (elem.type() == BSONType::Object) {
                    for (auto&& sortElem : elem.Obj()) {
                        out->_sortParts.push_back(
                            {FieldPath(sortElem.fieldNameStringData()),
                             parseSortDirection("$sortArray"_sd, sortElem)});
                    }
                    uassert(51203,
                            "$sortArray requires at least one field in object sortBy",
                            !out->_sortParts.empty());
                } else {
                    out->_wholeValueDirection = parseSortDirection("$sortArray"_sd, elem);
                }
            } else {
                uasserted(51204,
                          str::stream() << "$sortArray found unknown argument '" << fieldName
                                        << "'");
            }
        }

        uassert(51205, "$sortArray requires 'input'", out->_input);
        uassert(51206, "$sortArray requires 'sortBy'", sawSortBy);
        return out;
    }

    Value evaluate(const Document& root) const final {
        Value input = _input->evaluate(root);
        if (input.nullish() || !input.isArray()) {
            return Value(BSONNULL);
        }

        auto values = input.getArray();
        const auto& comparator = getExpressionContext()->getValueComparator();
        std::stable_sort(values.begin(), values.end(), [&](const Value& left, const Value& right) {
            if (_sortParts.empty()) {
                return comparator.compare(left, right) * _wholeValueDirection < 0;
            }

            for (const auto& part : _sortParts) {
                int cmp =
                    comparator.compare(extractPathValue(left, part.path), extractPathValue(right, part.path));
                if (cmp != 0) {
                    return cmp * part.direction < 0;
                }
            }
            return false;
        });
        return Value(std::move(values));
    }

    Value serialize(bool explain) const final {
        MutableDocument spec;
        spec.addField("input", _input->serialize(explain));
        if (_sortParts.empty()) {
            spec.addField("sortBy", Value(_wholeValueDirection));
        } else {
            MutableDocument sortSpec;
            for (const auto& part : _sortParts) {
                sortSpec.addField(part.path.fullPath(), Value(part.direction));
            }
            spec.addField("sortBy", sortSpec.freezeToValue());
        }
        return Value(Document{{"$sortArray", spec.freezeToValue()}});
    }

    void _doAddDependencies(DepsTracker* deps) const final {
        _input->addDependencies(deps);
    }

    const char* getOpName() const {
        return "$sortArray";
    }

private:
    intrusive_ptr<Expression> _input;
    int _wholeValueDirection = 1;
    std::vector<SortPart> _sortParts;
};

template <typename SubClass>
class ExpressionArrayN final : public Expression {
public:
    explicit ExpressionArrayN(const intrusive_ptr<ExpressionContext>& expCtx) : Expression(expCtx) {}

    static intrusive_ptr<Expression> parse(const intrusive_ptr<ExpressionContext>& expCtx,
                                           BSONElement expr,
                                           const VariablesParseState& vps) {
        uassert(51207,
                str::stream() << SubClass::kName << " requires an object argument",
                expr.type() == BSONType::Object);

        intrusive_ptr<ExpressionArrayN<SubClass>> out(new ExpressionArrayN<SubClass>(expCtx));
        for (auto&& elem : expr.Obj()) {
            auto fieldName = elem.fieldNameStringData();
            if (fieldName == "input") {
                out->_input = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "n") {
                out->_n = Expression::parseOperand(expCtx, elem, vps);
            } else {
                uasserted(51208,
                          str::stream() << SubClass::kName << " found unknown argument '"
                                        << fieldName << "'");
            }
        }

        uassert(51209, str::stream() << SubClass::kName << " requires 'input'", out->_input);
        uassert(51210, str::stream() << SubClass::kName << " requires 'n'", out->_n);
        return out;
    }

    Value evaluate(const Document& root) const final {
        Value input = _input->evaluate(root);
        Value nValue = _n->evaluate(root);
        if (input.nullish()) {
            return Value(BSONNULL);
        }

        uassert(51211,
                str::stream() << SubClass::kName << " requires 'input' to be an array",
                input.isArray());
        uassert(51212,
                str::stream() << SubClass::kName << " requires 'n' to be a numeric integer",
                nValue.numeric() && nValue.integral());

        long long n = nValue.coerceToLong();
        uassert(51213, str::stream() << SubClass::kName << " requires 'n' to be positive", n > 0);

        auto values = input.getArray();
        if (SubClass::kSort) {
            const auto& comparator = getExpressionContext()->getValueComparator();
            std::stable_sort(values.begin(), values.end(), [&](const Value& left, const Value& right) {
                return comparator.compare(left, right) * SubClass::kDirection < 0;
            });
        }

        size_t count = std::min(static_cast<size_t>(n), values.size());
        if (SubClass::kTakeLast) {
            return Value(std::vector<Value>(values.end() - count, values.end()));
        }
        return Value(std::vector<Value>(values.begin(), values.begin() + count));
    }

    Value serialize(bool explain) const final {
        MutableDocument spec;
        spec.addField("input", _input->serialize(explain));
        spec.addField("n", _n->serialize(explain));
        return Value(Document{{SubClass::kName, spec.freezeToValue()}});
    }

    void _doAddDependencies(DepsTracker* deps) const final {
        _input->addDependencies(deps);
        _n->addDependencies(deps);
    }

    const char* getOpName() const {
        return SubClass::kName.rawData();
    }

private:
    intrusive_ptr<Expression> _input;
    intrusive_ptr<Expression> _n;
};

struct FirstN {
    static constexpr StringData kName = "$firstN"_sd;
    static constexpr bool kSort = false;
    static constexpr bool kTakeLast = false;
    static constexpr int kDirection = 1;
};

struct LastN {
    static constexpr StringData kName = "$lastN"_sd;
    static constexpr bool kSort = false;
    static constexpr bool kTakeLast = true;
    static constexpr int kDirection = 1;
};

struct MinN {
    static constexpr StringData kName = "$minN"_sd;
    static constexpr bool kSort = true;
    static constexpr bool kTakeLast = false;
    static constexpr int kDirection = 1;
};

struct MaxN {
    static constexpr StringData kName = "$maxN"_sd;
    static constexpr bool kSort = true;
    static constexpr bool kTakeLast = false;
    static constexpr int kDirection = -1;
};

}  // namespace

REGISTER_EXPRESSION(sortArray, ExpressionSortArray::parse);
REGISTER_EXPRESSION(firstN, ExpressionArrayN<FirstN>::parse);
REGISTER_EXPRESSION(lastN, ExpressionArrayN<LastN>::parse);
REGISTER_EXPRESSION(minN, ExpressionArrayN<MinN>::parse);
REGISTER_EXPRESSION(maxN, ExpressionArrayN<MaxN>::parse);

}  // namespace mongo
