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

struct TopBottomInput {
    Document sortKey;
    Document sortBy;
    Value output;
    long long n;
};

struct TopBottomEntry {
    Document sortKey;
    Value output;
    size_t ordinal;
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

long long parsePositiveN(const Value& nValue, StringData opName) {
    uassert(6789316,
            str::stream() << opName << " requires 'n' to be a numeric integer",
            nValue.numeric() && nValue.integral());

    long long n = nValue.coerceToLong();
    uassert(6789317, str::stream() << opName << " requires 'n' to be positive", n > 0);
    return n;
}

TopBottomInput parseTopBottomInput(const Value& value, StringData opName) {
    uassert(6789310,
            str::stream() << opName << " requires an object argument",
            value.getType() == BSONType::Object);

    auto doc = value.getDocument();
    auto sortKey = doc["__sortKey"];
    auto sortBy = doc["__sortBy"];
    auto output = doc["__output"];
    auto n = doc["__n"];
    uassert(6789311,
            str::stream() << opName << " requires an object 'sortBy'",
            sortBy.getType() == BSONType::Object);
    uassert(6789312,
            str::stream() << opName << " requires an object sort key",
            sortKey.getType() == BSONType::Object);
    uassert(6789313, str::stream() << opName << " requires 'output'", !output.missing());
    uassert(6789314, str::stream() << opName << " requires 'n'", !n.missing());
    return {sortKey.getDocument(), sortBy.getDocument(), output, parsePositiveN(n, opName)};
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

class AccumulatorTopBottomBase : public Accumulator {
public:
    AccumulatorTopBottomBase(const intrusive_ptr<ExpressionContext>& expCtx,
                             StringData opName,
                             bool top,
                             bool single)
        : Accumulator(expCtx), _opName(opName), _top(top), _single(single) {
        _memUsageBytes = sizeof(*this);
    }

    void processInternal(const Value& input, bool merging) final {
        auto parsed = parseTopBottomInput(input, _opName);
        if (_n == 0) {
            _n = parsed.n;
            _sortBy = parsed.sortBy;
        } else {
            uassert(6789315,
                    str::stream() << _opName << " requires a consistent 'n' value per group",
                    _n == parsed.n);
        }

        _values.push_back({parsed.sortKey, parsed.output, _nextOrdinal++});
        _memUsageBytes += parsed.output.getApproximateSize() + parsed.sortKey.getApproximateSize();
    }

    void reset() final {
        vector<TopBottomEntry>().swap(_values);
        _sortBy = Document();
        _n = 0;
        _nextOrdinal = 0;
        _memUsageBytes = sizeof(*this);
    }

protected:
    Value topBottomValue() {
        if (_values.empty()) {
            return Value(BSONNULL);
        }

        const auto& comparator = getExpressionContext()->getValueComparator();
        std::stable_sort(_values.begin(), _values.end(), [&](const TopBottomEntry& left,
                                                             const TopBottomEntry& right) {
            auto sortFields = _sortBy.fieldIterator();
            while (sortFields.more()) {
                auto sortField = sortFields.next();
                int direction = sortField.second.coerceToInt();
                int cmp = comparator.compare(left.sortKey[sortField.first],
                                             right.sortKey[sortField.first]) *
                    direction;
                if (cmp != 0) {
                    return _top ? cmp < 0 : cmp > 0;
                }
            }
            return left.ordinal < right.ordinal;
        });

        size_t count = std::min(_values.size(), static_cast<size_t>(_n));
        if (_single) {
            return _values.front().output;
        }

        vector<Value> output;
        output.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            output.push_back(_values[i].output);
        }
        return Value(std::move(output));
    }

private:
    vector<TopBottomEntry> _values;
    Document _sortBy;
    long long _n = 0;
    size_t _nextOrdinal = 0;
    StringData _opName;
    bool _top;
    bool _single;
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

class AccumulatorTop final : public AccumulatorTopBottomBase {
public:
    explicit AccumulatorTop(const intrusive_ptr<ExpressionContext>& expCtx)
        : AccumulatorTopBottomBase(expCtx, "$top"_sd, true, true) {}

    Value getValue(bool toBeMerged) final {
        return topBottomValue();
    }

    const char* getOpName() const final {
        return "$top";
    }

    static intrusive_ptr<Accumulator> create(const intrusive_ptr<ExpressionContext>& expCtx) {
        return new AccumulatorTop(expCtx);
    }
};

class AccumulatorTopN final : public AccumulatorTopBottomBase {
public:
    explicit AccumulatorTopN(const intrusive_ptr<ExpressionContext>& expCtx)
        : AccumulatorTopBottomBase(expCtx, "$topN"_sd, true, false) {}

    Value getValue(bool toBeMerged) final {
        return topBottomValue();
    }

    const char* getOpName() const final {
        return "$topN";
    }

    static intrusive_ptr<Accumulator> create(const intrusive_ptr<ExpressionContext>& expCtx) {
        return new AccumulatorTopN(expCtx);
    }
};

class AccumulatorBottom final : public AccumulatorTopBottomBase {
public:
    explicit AccumulatorBottom(const intrusive_ptr<ExpressionContext>& expCtx)
        : AccumulatorTopBottomBase(expCtx, "$bottom"_sd, false, true) {}

    Value getValue(bool toBeMerged) final {
        return topBottomValue();
    }

    const char* getOpName() const final {
        return "$bottom";
    }

    static intrusive_ptr<Accumulator> create(const intrusive_ptr<ExpressionContext>& expCtx) {
        return new AccumulatorBottom(expCtx);
    }
};

class AccumulatorBottomN final : public AccumulatorTopBottomBase {
public:
    explicit AccumulatorBottomN(const intrusive_ptr<ExpressionContext>& expCtx)
        : AccumulatorTopBottomBase(expCtx, "$bottomN"_sd, false, false) {}

    Value getValue(bool toBeMerged) final {
        return topBottomValue();
    }

    const char* getOpName() const final {
        return "$bottomN";
    }

    static intrusive_ptr<Accumulator> create(const intrusive_ptr<ExpressionContext>& expCtx) {
        return new AccumulatorBottomN(expCtx);
    }
};

}  // namespace

REGISTER_ACCUMULATOR(firstN, AccumulatorFirstN::create);
REGISTER_ACCUMULATOR(lastN, AccumulatorLastN::create);
REGISTER_ACCUMULATOR(minN, AccumulatorMinN::create);
REGISTER_ACCUMULATOR(maxN, AccumulatorMaxN::create);
REGISTER_ACCUMULATOR(top, AccumulatorTop::create);
REGISTER_ACCUMULATOR(topN, AccumulatorTopN::create);
REGISTER_ACCUMULATOR(bottom, AccumulatorBottom::create);
REGISTER_ACCUMULATOR(bottomN, AccumulatorBottomN::create);

}  // namespace mongo
