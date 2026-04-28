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
#include <cmath>

#include "mongo/db/pipeline/accumulation_statement.h"
#include "mongo/db/pipeline/document.h"
#include "mongo/db/pipeline/expression_context.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {
namespace {

using boost::intrusive_ptr;
using std::vector;

struct PercentileInput {
    Value input;
    vector<double> percentiles;
};

vector<double> parsePercentiles(const Value& pValue, StringData opName) {
    uassert(6789201,
            str::stream() << opName << " requires 'p' to be an array of numeric values",
            pValue.getType() == BSONType::Array);

    vector<double> percentiles;
    for (const auto& p : pValue.getArray()) {
        uassert(6789202,
                str::stream() << opName << " requires each percentile to be numeric",
                p.numeric());
        double percentile = p.coerceToDouble();
        uassert(6789203,
                str::stream() << opName << " requires percentiles to be in [0, 1]",
                percentile >= 0.0 && percentile <= 1.0);
        percentiles.push_back(percentile);
    }

    uassert(6789204,
            str::stream() << opName << " requires at least one percentile",
            !percentiles.empty());
    return percentiles;
}

void validateMethod(const Value& methodValue, StringData opName) {
    uassert(6789205,
            str::stream() << opName << " requires method: 'approximate'",
            methodValue.getType() == BSONType::String &&
                methodValue.getStringData() == "approximate"_sd);
}

PercentileInput parsePercentileInput(const Value& value, StringData opName, bool median) {
    uassert(6789206,
            str::stream() << opName << " requires an object argument",
            value.getType() == BSONType::Object);

    auto doc = value.getDocument();
    bool hasInput = doc.positionOf("input").found();
    bool hasMethod = doc.positionOf("method").found();
    uassert(6789207, str::stream() << opName << " requires 'input'", hasInput);
    uassert(6789208, str::stream() << opName << " requires 'method'", hasMethod);
    validateMethod(doc["method"], opName);

    if (median) {
        return {doc["input"], vector<double>{0.5}};
    }

    bool hasP = doc.positionOf("p").found();
    uassert(6789209, str::stream() << opName << " requires 'p'", hasP);
    return {doc["input"], parsePercentiles(doc["p"], opName)};
}

bool samePercentiles(const vector<double>& left, const vector<double>& right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (size_t i = 0; i < left.size(); ++i) {
        if (std::abs(left[i] - right[i]) > 1e-12) {
            return false;
        }
    }
    return true;
}

Value interpolate(const vector<double>& values, double percentile) {
    if (values.empty()) {
        return Value(BSONNULL);
    }
    if (values.size() == 1) {
        return Value(values.front());
    }

    double rank = percentile * static_cast<double>(values.size() - 1);
    size_t lower = static_cast<size_t>(std::floor(rank));
    size_t upper = static_cast<size_t>(std::ceil(rank));
    if (lower == upper) {
        return Value(values[lower]);
    }

    double fraction = rank - static_cast<double>(lower);
    return Value(values[lower] + (values[upper] - values[lower]) * fraction);
}

class AccumulatorPercentileBase : public Accumulator {
public:
    AccumulatorPercentileBase(const intrusive_ptr<ExpressionContext>& expCtx,
                              StringData opName,
                              bool median)
        : Accumulator(expCtx), _opName(opName), _median(median) {
        _memUsageBytes = sizeof(*this);
    }

    void processInternal(const Value& input, bool merging) final {
        auto parsed = parsePercentileInput(input, _opName, _median);
        if (_percentiles.empty()) {
            _percentiles = parsed.percentiles;
        } else {
            uassert(6789210,
                    str::stream() << _opName
                                  << " requires a consistent percentile list per group",
                    samePercentiles(_percentiles, parsed.percentiles));
        }

        if (parsed.input.numeric()) {
            _values.push_back(parsed.input.coerceToDouble());
            _memUsageBytes += sizeof(double);
        }
    }

    void reset() final {
        vector<double>().swap(_values);
        vector<double>().swap(_percentiles);
        _memUsageBytes = sizeof(*this);
    }

protected:
    vector<Value> percentileValues() const {
        auto sorted = _values;
        std::sort(sorted.begin(), sorted.end());

        vector<Value> output;
        output.reserve(_percentiles.size());
        for (double percentile : _percentiles) {
            output.push_back(interpolate(sorted, percentile));
        }
        return output;
    }

private:
    vector<double> _values;
    vector<double> _percentiles;
    StringData _opName;
    bool _median;
};

class AccumulatorPercentile final : public AccumulatorPercentileBase {
public:
    explicit AccumulatorPercentile(const intrusive_ptr<ExpressionContext>& expCtx)
        : AccumulatorPercentileBase(expCtx, "$percentile"_sd, false) {}

    Value getValue(bool toBeMerged) final {
        return Value(percentileValues());
    }

    const char* getOpName() const final {
        return "$percentile";
    }

    static intrusive_ptr<Accumulator> create(const intrusive_ptr<ExpressionContext>& expCtx) {
        return new AccumulatorPercentile(expCtx);
    }
};

class AccumulatorMedian final : public AccumulatorPercentileBase {
public:
    explicit AccumulatorMedian(const intrusive_ptr<ExpressionContext>& expCtx)
        : AccumulatorPercentileBase(expCtx, "$median"_sd, true) {}

    Value getValue(bool toBeMerged) final {
        auto values = percentileValues();
        return values.empty() ? Value(BSONNULL) : values.front();
    }

    const char* getOpName() const final {
        return "$median";
    }

    static intrusive_ptr<Accumulator> create(const intrusive_ptr<ExpressionContext>& expCtx) {
        return new AccumulatorMedian(expCtx);
    }
};

}  // namespace

REGISTER_ACCUMULATOR(percentile, AccumulatorPercentile::create);
REGISTER_ACCUMULATOR(median, AccumulatorMedian::create);

}  // namespace mongo
