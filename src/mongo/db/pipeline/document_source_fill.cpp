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

#include <vector>

#include "mongo/db/jsobj.h"
#include "mongo/db/pipeline/document.h"
#include "mongo/db/pipeline/document_source.h"
#include "mongo/db/pipeline/field_path.h"
#include "mongo/db/pipeline/lite_parsed_document_source.h"
#include "mongo/db/pipeline/value.h"

namespace mongo {
namespace {

class DocumentSourceFill final : public DocumentSource {
public:
    static constexpr StringData kStageName = "$fill"_sd;

    enum class RuleType {
        kLiteral,
        kLocf,
    };

    struct Rule {
        FieldPath field;
        RuleType type;
        Value literal;
        Value lastSeen;
    };

    DocumentSourceFill(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                       std::vector<Rule> rules,
                       BSONObj sortBy)
        : DocumentSource(expCtx), _rules(std::move(rules)), _sortBy(sortBy.getOwned()) {}

    GetNextResult getNext() final {
        pExpCtx->checkForInterrupt();

        auto next = pSource->getNext();
        if (!next.isAdvanced()) {
            return next;
        }

        auto input = next.releaseDocument();
        MutableDocument output(input);
        for (auto& rule : _rules) {
            auto current = input.getNestedField(rule.field);
            if (rule.type == RuleType::kLiteral) {
                if (current.nullish()) {
                    output.setNestedField(rule.field, rule.literal);
                }
                continue;
            }

            if (current.nullish()) {
                if (!rule.lastSeen.missing()) {
                    output.setNestedField(rule.field, rule.lastSeen);
                }
            } else {
                rule.lastSeen = current;
            }
        }

        return GetNextResult(output.freeze());
    }

    const char* getSourceName() const final {
        return kStageName.rawData();
    }

    Value serialize(boost::optional<ExplainOptions::Verbosity> explain = boost::none) const final {
        MutableDocument output;
        for (const auto& rule : _rules) {
            MutableDocument ruleSpec;
            if (rule.type == RuleType::kLiteral) {
                ruleSpec["value"] = rule.literal;
            } else {
                ruleSpec["method"] = Value("locf"_sd);
            }
            output[rule.field.fullPath()] = ruleSpec.freezeToValue();
        }

        MutableDocument spec;
        if (!_sortBy.isEmpty()) {
            spec["sortBy"] = Value(_sortBy);
        }
        spec["output"] = output.freezeToValue();
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
        uassert(6789120, "$fill requires an object specification", elem.type() == BSONType::Object);

        BSONObj spec = elem.Obj();
        uassert(6789121,
                "$fill partitionBy is not supported in this compatibility checkpoint",
                spec["partitionBy"].eoo());
        uassert(6789122,
                "$fill partitionByFields is not supported in this compatibility checkpoint",
                spec["partitionByFields"].eoo());

        BSONObj sortBy;
        if (auto sortByElem = spec["sortBy"]) {
            uassert(6789123, "$fill sortBy must be an object", sortByElem.type() == BSONType::Object);
            sortBy = sortByElem.Obj().getOwned();
        }

        BSONElement outputElem = spec["output"];
        uassert(6789124, "$fill requires an object 'output'", outputElem.type() == BSONType::Object);

        std::vector<Rule> rules;
        for (auto&& fieldSpecElem : outputElem.Obj()) {
            uassert(6789125,
                    "$fill output field specification must be an object",
                    fieldSpecElem.type() == BSONType::Object);

            BSONObj fieldSpec = fieldSpecElem.Obj();
            BSONElement methodElem = fieldSpec["method"];
            BSONElement valueElem = fieldSpec["value"];
            uassert(6789126,
                    "$fill output fields require exactly one of 'method' or 'value'",
                    methodElem.eoo() != valueElem.eoo());

            Rule rule{FieldPath(fieldSpecElem.fieldNameStringData()), RuleType::kLiteral, Value(), Value()};
            if (!methodElem.eoo()) {
                uassert(6789127,
                        "$fill method must be a string",
                        methodElem.type() == BSONType::String);
                uassert(6789128,
                        "$fill currently supports only method: 'locf'",
                        methodElem.str() == "locf");
                rule.type = RuleType::kLocf;
            } else {
                rule.literal = Value(valueElem);
            }
            rules.push_back(std::move(rule));
        }
        uassert(6789129, "$fill requires at least one output field", !rules.empty());

        return new DocumentSourceFill(expCtx, std::move(rules), sortBy);
    }

private:
    std::vector<Rule> _rules;
    BSONObj _sortBy;
};

constexpr StringData DocumentSourceFill::kStageName;

}  // namespace

REGISTER_DOCUMENT_SOURCE(fill,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourceFill::createFromBson);

}  // namespace mongo
