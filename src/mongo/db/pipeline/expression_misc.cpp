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

#include "mongo/db/hasher.h"
#include "mongo/db/pipeline/expression.h"
#include "mongo/platform/random.h"

namespace mongo {
namespace {

class ExpressionRand final : public Expression {
public:
    explicit ExpressionRand(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : Expression(expCtx) {}

    static boost::intrusive_ptr<Expression> parse(
        const boost::intrusive_ptr<ExpressionContext>& expCtx,
        BSONElement expr,
        const VariablesParseState& vps) {
        uassert(ErrorCodes::FailedToParse,
                "$rand does not currently accept arguments",
                expr.type() == BSONType::Object && expr.Obj().isEmpty());
        return new ExpressionRand(expCtx);
    }

    boost::intrusive_ptr<Expression> optimize() final {
        return this;
    }

    Value evaluate(const Document& root) const final {
        thread_local PseudoRandom rng(SecureRandom::create()->nextInt64());
        return Value(rng.nextCanonicalDouble());
    }

    Value serialize(bool explain) const final {
        return Value(DOC("$rand" << Document()));
    }

    const char* getOpName() const {
        return "$rand";
    }

private:
    void _doAddDependencies(DepsTracker* deps) const final {}
};

class ExpressionBinarySize final : public ExpressionFixedArity<ExpressionBinarySize, 1> {
public:
    explicit ExpressionBinarySize(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionFixedArity<ExpressionBinarySize, 1>(expCtx) {}

    Value evaluate(const Document& root) const final {
        auto arg = vpOperand[0]->evaluate(root);
        if (arg.nullish()) {
            return Value(BSONNULL);
        }

        uassert(51276,
                str::stream() << "$binarySize requires a string or BinData argument, found: "
                              << typeName(arg.getType()),
                arg.getType() == BSONType::String || arg.getType() == BSONType::BinData);

        if (arg.getType() == BSONType::String) {
            return Value(static_cast<int>(arg.getStringData().size()));
        }

        return Value(arg.getBinData().length);
    }

    const char* getOpName() const final {
        return "$binarySize";
    }
};

class ExpressionBsonSize final : public ExpressionFixedArity<ExpressionBsonSize, 1> {
public:
    explicit ExpressionBsonSize(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionFixedArity<ExpressionBsonSize, 1>(expCtx) {}

    Value evaluate(const Document& root) const final {
        auto arg = vpOperand[0]->evaluate(root);
        if (arg.nullish()) {
            return Value(BSONNULL);
        }

        uassert(31393,
                str::stream() << "$bsonSize requires a document input, found: "
                              << typeName(arg.getType()),
                arg.getType() == BSONType::Object);

        return Value(arg.getDocument().toBson().objsize());
    }

    const char* getOpName() const final {
        return "$bsonSize";
    }
};

class ExpressionToHashedIndexKey final : public ExpressionFixedArity<ExpressionToHashedIndexKey, 1> {
public:
    explicit ExpressionToHashedIndexKey(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : ExpressionFixedArity<ExpressionToHashedIndexKey, 1>(expCtx) {}

    Value evaluate(const Document& root) const final {
        auto input = vpOperand[0]->evaluate(root);
        if (input.missing()) {
            input = Value(BSONNULL);
        }

        return Value(BSONElementHasher::hash64(BSON("" << input).firstElement(),
                                               BSONElementHasher::DEFAULT_HASH_SEED));
    }

    const char* getOpName() const final {
        return "$toHashedIndexKey";
    }
};

}  // namespace

REGISTER_EXPRESSION(rand, ExpressionRand::parse);
REGISTER_EXPRESSION(binarySize, ExpressionBinarySize::parse);
REGISTER_EXPRESSION(bsonSize, ExpressionBsonSize::parse);
REGISTER_EXPRESSION(toHashedIndexKey, ExpressionToHashedIndexKey::parse);

}  // namespace mongo
