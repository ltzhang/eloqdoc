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

#include <cmath>
#include <deque>
#include <limits>

#include "mongo/db/jsobj.h"
#include "mongo/db/pipeline/document.h"
#include "mongo/db/pipeline/document_source.h"
#include "mongo/db/pipeline/field_path.h"
#include "mongo/db/pipeline/lite_parsed_document_source.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {
namespace {

class DocumentSourceDensify final : public DocumentSource {
public:
    static constexpr StringData kStageName = "$densify"_sd;

    DocumentSourceDensify(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                          FieldPath field,
                          double step,
                          double lower,
                          double upper)
        : DocumentSource(expCtx),
          _field(std::move(field)),
          _step(step),
          _lower(lower),
          _upper(upper),
          _nextValue(lower) {}

    GetNextResult getNext() final {
        pExpCtx->checkForInterrupt();

        while (true) {
            if (!_pending.empty()) {
                auto doc = std::move(_pending.front());
                _pending.pop_front();
                return GetNextResult(std::move(doc));
            }

            if (_inputExhausted) {
                if (canGenerateNext()) {
                    return GetNextResult(makeSyntheticDocument(_nextValue));
                }
                return GetNextResult::makeEOF();
            }

            auto next = pSource->getNext();
            if (next.isPaused()) {
                return next;
            }
            if (next.isEOF()) {
                _inputExhausted = true;
                continue;
            }

            auto doc = next.releaseDocument();
            double current = extractNumericField(doc);
            if (_haveLastInput) {
                uassert(6789103,
                        "$densify requires input documents to be sorted ascending",
                        current >= _lastInput);
            }
            _haveLastInput = true;
            _lastInput = current;

            if (current >= _lower) {
                while (canGenerateNext() && _nextValue < current) {
                    _pending.push_back(makeSyntheticDocument(_nextValue));
                }
                if (canGenerateNext() && nearlyEqual(_nextValue, current)) {
                    _nextValue += _step;
                }
            }

            _pending.push_back(std::move(doc));
        }
    }

    const char* getSourceName() const final {
        return kStageName.rawData();
    }

    Value serialize(boost::optional<ExplainOptions::Verbosity> explain = boost::none) const final {
        MutableDocument range;
        range["step"] = numericValue(_step);
        range["bounds"] = Value(std::vector<Value>{numericValue(_lower), numericValue(_upper)});

        MutableDocument spec;
        spec["field"] = Value(_field.fullPath());
        spec["range"] = range.freezeToValue();
        return Value(Document{{kStageName, spec.freezeToValue()}});
    }

    StageConstraints constraints(Pipeline::SplitState pipeState) const final {
        return StageConstraints(StreamType::kStreaming,
                                PositionRequirement::kNone,
                                HostTypeRequirement::kNone,
                                DiskUseRequirement::kNoDiskUse,
                                FacetRequirement::kAllowed,
                                TransactionRequirement::kAllowed);
    }

    static boost::intrusive_ptr<DocumentSource> createFromBson(
        BSONElement elem, const boost::intrusive_ptr<ExpressionContext>& expCtx) {
        uassert(6789104,
                "$densify requires an object specification",
                elem.type() == BSONType::Object);

        BSONObj spec = elem.Obj();
        BSONElement fieldElem = spec["field"];
        BSONElement rangeElem = spec["range"];
        uassert(6789105,
                "$densify requires a string 'field'",
                fieldElem.type() == BSONType::String);
        uassert(6789106,
                "$densify requires an object 'range'",
                rangeElem.type() == BSONType::Object);
        uassert(6789107,
                "$densify partitionByFields is not supported in this compatibility checkpoint",
                spec["partitionByFields"].eoo());

        BSONObj range = rangeElem.Obj();
        BSONElement stepElem = range["step"];
        BSONElement boundsElem = range["bounds"];
        uassert(6789108, "$densify requires a numeric 'range.step'", stepElem.isNumber());
        double step = stepElem.numberDouble();
        uassert(6789109, "$densify requires 'range.step' to be positive", step > 0);
        uassert(6789110,
                "$densify date units are not supported in this compatibility checkpoint",
                range["unit"].eoo());
        uassert(6789111,
                "$densify currently supports only explicit numeric bounds",
                boundsElem.type() == BSONType::Array);

        std::vector<double> bounds;
        for (auto&& bound : boundsElem.Obj()) {
            uassert(6789112, "$densify bounds must be numeric", bound.isNumber());
            bounds.push_back(bound.numberDouble());
        }
        uassert(6789113, "$densify requires exactly two bounds", bounds.size() == 2);
        uassert(6789114, "$densify lower bound must be <= upper bound", bounds[0] <= bounds[1]);

        return new DocumentSourceDensify(
            expCtx, FieldPath(fieldElem.str()), step, bounds[0], bounds[1]);
    }

private:
    bool canGenerateNext() {
        if (_nextValue > _upper || nearlyEqual(_nextValue, _upper + _step)) {
            return false;
        }
        return _nextValue <= _upper || nearlyEqual(_nextValue, _upper);
    }

    Document makeSyntheticDocument(double value) {
        MutableDocument doc;
        doc.setNestedField(_field, numericValue(value));
        _nextValue += _step;
        return doc.freeze();
    }

    double extractNumericField(const Document& doc) const {
        auto value = doc.getNestedField(_field);
        uassert(6789115, "$densify field must be present and numeric", value.numeric());
        return value.coerceToDouble();
    }

    static bool nearlyEqual(double left, double right) {
        return std::abs(left - right) < 1e-9;
    }

    static Value numericValue(double value) {
        long long asLong = static_cast<long long>(value);
        if (nearlyEqual(value, static_cast<double>(asLong))) {
            if (asLong >= std::numeric_limits<int>::min() &&
                asLong <= std::numeric_limits<int>::max()) {
                return Value(static_cast<int>(asLong));
            }
            return Value(asLong);
        }
        return Value(value);
    }

    FieldPath _field;
    double _step;
    double _lower;
    double _upper;
    double _nextValue;
    bool _inputExhausted = false;
    bool _haveLastInput = false;
    double _lastInput = 0;
    std::deque<Document> _pending;
};

constexpr StringData DocumentSourceDensify::kStageName;

}  // namespace

REGISTER_DOCUMENT_SOURCE(densify,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourceDensify::createFromBson);

}  // namespace mongo
