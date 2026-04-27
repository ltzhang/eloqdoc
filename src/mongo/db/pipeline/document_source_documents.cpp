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

namespace mongo {
namespace {

class DocumentSourceDocuments final : public DocumentSource {
public:
    static constexpr StringData kStageName = "$documents"_sd;

    DocumentSourceDocuments(const boost::intrusive_ptr<ExpressionContext>& expCtx,
                            std::vector<Document> docs)
        : DocumentSource(expCtx), _docs(std::move(docs)) {}

    GetNextResult getNext() final {
        pExpCtx->checkForInterrupt();

        if (_current == _docs.size()) {
            return GetNextResult::makeEOF();
        }

        return GetNextResult{std::move(_docs[_current++])};
    }

    const char* getSourceName() const final {
        return kStageName.rawData();
    }

    Value serialize(boost::optional<ExplainOptions::Verbosity> explain = boost::none) const final {
        return Value(Document{{kStageName, Value(_docs)}});
    }

    StageConstraints constraints(Pipeline::SplitState pipeState) const final {
        StageConstraints constraints(StreamType::kStreaming,
                                     PositionRequirement::kFirst,
                                     HostTypeRequirement::kNone,
                                     DiskUseRequirement::kNoDiskUse,
                                     FacetRequirement::kNotAllowed,
                                     TransactionRequirement::kAllowed);
        constraints.isIndependentOfAnyCollection = true;
        constraints.requiresInputDocSource = false;
        return constraints;
    }

    static boost::intrusive_ptr<DocumentSource> createFromBson(
        BSONElement elem, const boost::intrusive_ptr<ExpressionContext>& expCtx) {
        uassert(ErrorCodes::TypeMismatch,
                str::stream() << "$documents must be an array of objects, got "
                              << typeName(elem.type()),
                elem.type() == BSONType::Array);

        std::vector<Document> docs;
        for (auto&& entry : elem.Obj()) {
            uassert(ErrorCodes::TypeMismatch,
                    str::stream() << "$documents entries must be objects, got "
                                  << typeName(entry.type()),
                    entry.type() == BSONType::Object);
            docs.emplace_back(entry.Obj().getOwned());
        }

        return new DocumentSourceDocuments(expCtx, std::move(docs));
    }

private:
    std::vector<Document> _docs;
    size_t _current = 0;
};

constexpr StringData DocumentSourceDocuments::kStageName;

}  // namespace

REGISTER_DOCUMENT_SOURCE(documents,
                         LiteParsedDocumentSourceDefault::parse,
                         DocumentSourceDocuments::createFromBson);

}  // namespace mongo
