/**
 * Copyright (C) 2026 EloqData Inc.
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
 */

#include "mongo/platform/basic.h"

#include "mongo/db/pipeline/accumulator.h"

#include <algorithm>

#include "mongo/db/pipeline/accumulation_statement.h"
#include "mongo/db/pipeline/document.h"
#include "mongo/db/pipeline/expression_context.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {
namespace {

using boost::intrusive_ptr;
using std::vector;

struct NValueInput {
    Value input;
    long long n;
};

NValueInput parseNValueInput(const Value& value, StringData opName) {
    uassert(51214,
            str::stream() << opName << " requires an object argument with 'input' and 'n'",
            value.getType() == BSONType::Object);

    auto doc = value.getDocument();
    bool hasInput = doc.positionOf("input").found();
    bool hasN = doc.positionOf("n").found();
    auto input = doc["input"];
    auto nValue = doc["n"];
    uassert(51215, str::stream() << opName << " requires 'input'", hasInput);
    uassert(51216, str::stream() << opName << " requires 'n'", hasN);
    uassert(51217,
            str::stream() << opName << " requires 'n' to be a numeric integer",
            nValue.numeric() && nValue.integral());

    long long n = nValue.coerceToLong();
    uassert(51218, str::stream() << opName << " requires 'n' to be positive", n > 0);
    return {input, n};
}

class AccumulatorNValueBase : public Accumulator {
public:
    AccumulatorNValueBase(const intrusive_ptr<ExpressionContext>& expCtx, StringData opName)
        : Accumulator(expCtx), _opName(opName) {
        _memUsageBytes = sizeof(*this);
    }

    void reset() final {
        vector<Value>().swap(_values);
        _n = 0;
        _memUsageBytes = sizeof(*this);
    }

protected:
    NValueInput parseAndValidateInput(const Value& input) {
        auto parsed = parseNValueInput(input, _opName);
        if (_n == 0) {
            _n = parsed.n;
        } else {
            uassert(51219,
                    str::stream() << _opName << " requires a consistent 'n' value per group",
                    _n == parsed.n);
        }
        return parsed;
    }

    void appendValue(const Value& value) {
        if (!value.missing()) {
            _values.push_back(value);
            _memUsageBytes += value.getApproximateSize();
        }
    }

    void trimToLastN() {
        while (_values.size() > static_cast<size_t>(_n)) {
            _memUsageBytes -= _values.front().getApproximateSize();
            _values.erase(_values.begin());
        }
    }

    long long n() const {
        return _n;
    }

    const vector<Value>& values() const {
        return _values;
    }

private:
    vector<Value> _values;
    long long _n = 0;
    StringData _opName;
};

class AccumulatorFirstN final : public AccumulatorNValueBase {
public:
    explicit AccumulatorFirstN(const intrusive_ptr<ExpressionContext>& expCtx)
        : AccumulatorNValueBase(expCtx, "$firstN"_sd) {}

    void processInternal(const Value& input, bool merging) final {
        auto parsed = parseAndValidateInput(input);
        if (values().size() < static_cast<size_t>(n())) {
            appendValue(parsed.input);
        }
    }

    Value getValue(bool toBeMerged) final {
        return Value(values());
    }

    const char* getOpName() const final {
        return "$firstN";
    }

    static intrusive_ptr<Accumulator> create(const intrusive_ptr<ExpressionContext>& expCtx) {
        return new AccumulatorFirstN(expCtx);
    }
};

class AccumulatorLastN final : public AccumulatorNValueBase {
public:
    explicit AccumulatorLastN(const intrusive_ptr<ExpressionContext>& expCtx)
        : AccumulatorNValueBase(expCtx, "$lastN"_sd) {}

    void processInternal(const Value& input, bool merging) final {
        auto parsed = parseAndValidateInput(input);
        appendValue(parsed.input);
        trimToLastN();
    }

    Value getValue(bool toBeMerged) final {
        return Value(values());
    }

    const char* getOpName() const final {
        return "$lastN";
    }

    static intrusive_ptr<Accumulator> create(const intrusive_ptr<ExpressionContext>& expCtx) {
        return new AccumulatorLastN(expCtx);
    }
};

class AccumulatorMinMaxN : public AccumulatorNValueBase {
public:
    AccumulatorMinMaxN(const intrusive_ptr<ExpressionContext>& expCtx,
                       StringData opName,
                       int direction)
        : AccumulatorNValueBase(expCtx, opName), _direction(direction) {}

    void processInternal(const Value& input, bool merging) final {
        auto parsed = parseAndValidateInput(input);
        appendValue(parsed.input);
    }

    Value getValue(bool toBeMerged) final {
        auto output = values();
        const auto& comparator = getExpressionContext()->getValueComparator();
        std::stable_sort(output.begin(), output.end(), [&](const Value& left, const Value& right) {
            return comparator.compare(left, right) * _direction < 0;
        });

        size_t count = std::min(output.size(), static_cast<size_t>(n()));
        output.resize(count);
        return Value(std::move(output));
    }

private:
    int _direction;
};

class AccumulatorMinN final : public AccumulatorMinMaxN {
public:
    explicit AccumulatorMinN(const intrusive_ptr<ExpressionContext>& expCtx)
        : AccumulatorMinMaxN(expCtx, "$minN"_sd, 1) {}

    const char* getOpName() const final {
        return "$minN";
    }

    static intrusive_ptr<Accumulator> create(const intrusive_ptr<ExpressionContext>& expCtx) {
        return new AccumulatorMinN(expCtx);
    }
};

class AccumulatorMaxN final : public AccumulatorMinMaxN {
public:
    explicit AccumulatorMaxN(const intrusive_ptr<ExpressionContext>& expCtx)
        : AccumulatorMinMaxN(expCtx, "$maxN"_sd, -1) {}

    const char* getOpName() const final {
        return "$maxN";
    }

    static intrusive_ptr<Accumulator> create(const intrusive_ptr<ExpressionContext>& expCtx) {
        return new AccumulatorMaxN(expCtx);
    }
};

}  // namespace

REGISTER_ACCUMULATOR(firstN, AccumulatorFirstN::create);
REGISTER_ACCUMULATOR(lastN, AccumulatorLastN::create);
REGISTER_ACCUMULATOR(minN, AccumulatorMinN::create);
REGISTER_ACCUMULATOR(maxN, AccumulatorMaxN::create);

}  // namespace mongo
