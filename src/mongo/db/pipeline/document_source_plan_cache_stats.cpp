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

#include <vector>

#include "mongo/db/pipeline/document_source.h"
#include "mongo/db/pipeline/lite_parsed_document_source.h"
#include "mongo/db/pipeline/mongo_process_interface.h"

namespace mongo {
namespace {

class DocumentSourcePlanCacheStats final : public DocumentSource {
public:
    static constexpr StringData kStageName = "$planCacheStats"_sd;

    explicit DocumentSourcePlanCacheStats(const boost::intrusive_ptr<ExpressionContext>& expCtx)
        : DocumentSource(expCtx) {}

    GetNextResult getNext() final {
        pExpCtx->checkForInterrupt();

        if (!_loaded) {
            loadEntries();
            _loaded = true;
        }

        if (_current == _entries.size()) {
            return GetNextResult::makeEOF();
        }

        return GetNextResult{Document(_entries[_current++])};
    }

    const char* getSourceName() const final {
        return kStageName.rawData();
    }

    Value serialize(boost::optional<ExplainOptions::Verbosity> explain = boost::none) const final {
        return Value(DOC(getSourceName() << Document()));
    }

    StageConstraints constraints(Pipeline::SplitState pipeState) const final {
        StageConstraints constraints(StreamType::kStreaming,
                                     PositionRequirement::kFirst,
                                     HostTypeRequirement::kNone,
                                     DiskUseRequirement::kNoDiskUse,
                                     FacetRequirement::kNotAllowed,
                                     TransactionRequirement::kAllowed);
        constraints.requiresInputDocSource = false;
        return constraints;
    }

    static boost::intrusive_ptr<DocumentSource> createFromBson(
        BSONElement elem, const boost::intrusive_ptr<ExpressionContext>& expCtx) {
        uassert(ErrorCodes::FailedToParse,
                "The $planCacheStats stage specification must be an empty object",
                elem.type() == BSONType::Object && elem.Obj().isEmpty());
        return new DocumentSourcePlanCacheStats(expCtx);
    }

private:
    void loadEntries() {
        _entries = pExpCtx->mongoProcessInterface->getPlanCacheStats(pExpCtx->opCtx, pExpCtx->ns);
    }

    bool _loaded = false;
    std::vector<BSONObj> _entries;
    size_t _current = 0;
};

constexpr StringData DocumentSourcePlanCacheStats::kStageName;

}  // namespace

REGISTER_DOCUMENT_SOURCE(planCacheStats,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourcePlanCacheStats::createFromBson);

}  // namespace mongo
