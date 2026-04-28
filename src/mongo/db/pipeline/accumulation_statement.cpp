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

#include <string>

#include "mongo/db/pipeline/accumulation_statement.h"

#include "mongo/bson/bsonobjbuilder.h"
#include "mongo/db/pipeline/accumulator.h"
#include "mongo/db/pipeline/value.h"
#include "mongo/util/assert_util.h"
#include "mongo/util/mongoutils/str.h"
#include "mongo/util/string_map.h"

namespace mongo {

using boost::intrusive_ptr;
using std::string;

namespace {
// Used to keep track of which Accumulators are registered under which name.
static StringMap<Accumulator::Factory> factoryMap;

bool isTopBottomAccumulator(StringData accName) {
    return accName == "$top"_sd || accName == "$topN"_sd || accName == "$bottom"_sd ||
        accName == "$bottomN"_sd;
}

bool isTopBottomNAccumulator(StringData accName) {
    return accName == "$topN"_sd || accName == "$bottomN"_sd;
}

BSONObj rewriteTopBottomSpec(StringData accName, BSONElement specElem) {
    uassert(6789300,
            str::stream() << accName << " requires an object argument",
            specElem.type() == BSONType::Object);

    BSONObj spec = specElem.Obj();
    BSONElement sortBy = spec["sortBy"];
    BSONElement output = spec["output"];
    BSONElement n = spec["n"];
    uassert(6789301,
            str::stream() << accName << " requires an object 'sortBy'",
            sortBy.type() == BSONType::Object && !sortBy.Obj().isEmpty());
    uassert(6789302, str::stream() << accName << " requires 'output'", !output.eoo());
    uassert(6789303,
            str::stream() << accName << " requires 'n'",
            !isTopBottomNAccumulator(accName) || !n.eoo());
    uassert(6789304,
            str::stream() << accName << " does not accept 'n'",
            isTopBottomNAccumulator(accName) || n.eoo());

    BSONObjBuilder rewritten;
    BSONObjBuilder rewrittenSortBy;
    {
        BSONObjBuilder sortKey(rewritten.subobjStart("__sortKey"));
        int sortKeyIndex = 0;
        for (auto&& sortElem : sortBy.Obj()) {
            uassert(6789305,
                    str::stream() << accName << " sortBy fields must have direction 1 or -1",
                    sortElem.isNumber() &&
                        (sortElem.numberInt() == 1 || sortElem.numberInt() == -1));
            std::string internalName = str::stream() << "__sortKey" << sortKeyIndex++;
            sortKey.append(internalName, str::stream() << "$" << sortElem.fieldName());
            rewrittenSortBy.append(internalName, sortElem.numberInt());
        }
    }
    rewritten.append("__sortBy", rewrittenSortBy.obj());
    rewritten.appendAs(output, "__output");
    if (isTopBottomNAccumulator(accName)) {
        rewritten.appendAs(n, "__n");
    } else {
        rewritten.append("__n", 1);
    }
    return rewritten.obj();
}
}  // namespace

void AccumulationStatement::registerAccumulator(std::string name, Accumulator::Factory factory) {
    auto it = factoryMap.find(name);
    massert(28722,
            str::stream() << "Duplicate accumulator (" << name << ") registered.",
            it == factoryMap.end());
    factoryMap[name] = factory;
}

Accumulator::Factory AccumulationStatement::getFactory(StringData name) {
    auto it = factoryMap.find(name);
    uassert(
        15952, str::stream() << "unknown group operator '" << name << "'", it != factoryMap.end());
    return it->second;
}

boost::intrusive_ptr<Accumulator> AccumulationStatement::makeAccumulator(
    const boost::intrusive_ptr<ExpressionContext>& expCtx) const {
    return _factory(expCtx);
}

AccumulationStatement AccumulationStatement::parseAccumulationStatement(
    const boost::intrusive_ptr<ExpressionContext>& expCtx,
    const BSONElement& elem,
    const VariablesParseState& vps) {
    auto fieldName = elem.fieldNameStringData();
    uassert(40234,
            str::stream() << "The field '" << fieldName << "' must be an accumulator object",
            elem.type() == BSONType::Object &&
                elem.embeddedObject().firstElementFieldName()[0] == '$');

    uassert(40235,
            str::stream() << "The field name '" << fieldName << "' cannot contain '.'",
            fieldName.find('.') == string::npos);

    uassert(40236,
            str::stream() << "The field name '" << fieldName << "' cannot be an operator name",
            fieldName[0] != '$');

    uassert(40238,
            str::stream() << "The field '" << fieldName << "' must specify one accumulator",
            elem.Obj().nFields() == 1);

    auto specElem = elem.Obj().firstElement();
    auto accName = specElem.fieldNameStringData();
    uassert(40237,
            str::stream() << "The " << accName << " accumulator is a unary operator",
            specElem.type() != BSONType::Array);

    if (accName == "$count"_sd) {
        uassert(ErrorCodes::FailedToParse,
                "The $count accumulator takes no arguments",
                specElem.type() == BSONType::Object && specElem.Obj().isEmpty());
        return {fieldName.toString(),
                ExpressionConstant::create(expCtx, Value(1)),
                AccumulationStatement::getFactory(accName)};
    }

    if (isTopBottomAccumulator(accName)) {
        return {fieldName.toString(),
                Expression::parseObject(expCtx, rewriteTopBottomSpec(accName, specElem), vps),
                AccumulationStatement::getFactory(accName)};
    }

    return {fieldName.toString(),
            Expression::parseOperand(expCtx, specElem, vps),
            AccumulationStatement::getFactory(accName)};
}

}  // namespace mongo
