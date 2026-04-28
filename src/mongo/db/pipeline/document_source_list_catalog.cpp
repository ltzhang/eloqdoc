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
 * file(s) with this exception, you may extend this exception to your version
 * of the file(s), but you are not obligated to do so. If you do not wish to
 * do so, delete this exception statement from your version. If you delete
 * this exception statement from all source files in the program, then also
 * delete it in the license file.
 */

#include "mongo/platform/basic.h"

#include <vector>

#include "mongo/db/auth/action_type.h"
#include "mongo/db/auth/privilege.h"
#include "mongo/db/auth/resource_pattern.h"
#include "mongo/db/catalog/collection.h"
#include "mongo/db/catalog/collection_catalog_entry.h"
#include "mongo/db/catalog/database.h"
#include "mongo/db/catalog/database_holder.h"
#include "mongo/db/concurrency/d_concurrency.h"
#include "mongo/db/pipeline/document_source.h"
#include "mongo/db/pipeline/lite_parsed_document_source.h"

namespace mongo {
namespace {

BSONObj buildCatalogEntry(OperationContext* opCtx, Collection* collection) {
    invariant(collection);

    const NamespaceString nss = collection->ns();
    CollectionCatalogEntry* catalogEntry = collection->getCatalogEntry();

    BSONObjBuilder builder;
    builder.append("db", nss.db());
    builder.append("name", nss.coll());
    builder.append("type", "collection");
    builder.append("ns", nss.ns());

    BSONObjBuilder mdBuilder(builder.subobjStart("md"));
    mdBuilder.append("options", catalogEntry->getCollectionOptions(opCtx).toBSON());

    std::vector<std::string> indexNames;
    catalogEntry->getAllIndexes(opCtx, &indexNames);
    BSONArrayBuilder indexesBuilder(mdBuilder.subarrayStart("indexes"));
    BSONObjBuilder idxIdentBuilder;
    for (const auto& indexName : indexNames) {
        indexesBuilder.append(catalogEntry->getIndexSpec(opCtx, indexName));
        const std::string ident = catalogEntry->getIndexIdent(opCtx, indexName);
        if (!ident.empty()) {
            idxIdentBuilder.append(indexName, ident);
        }
    }
    indexesBuilder.doneFast();
    mdBuilder.doneFast();
    builder.append("idxIdent", idxIdentBuilder.obj());

    return builder.obj();
}

class DocumentSourceListCatalog final : public DocumentSource {
public:
    static constexpr StringData kStageName = "$listCatalog"_sd;

    class LiteParsed final : public LiteParsedDocumentSource {
    public:
        static std::unique_ptr<LiteParsed> parse(const AggregationRequest& request,
                                                 const BSONElement& spec) {
            return stdx::make_unique<LiteParsed>(request.getNamespaceString());
        }

        explicit LiteParsed(NamespaceString nss) : _nss(std::move(nss)) {}

        stdx::unordered_set<NamespaceString> getInvolvedNamespaces() const final {
            return {};
        }

        PrivilegeVector requiredPrivileges(bool isMongos) const final {
            return {Privilege(ResourcePattern::forDatabaseName(_nss.db()),
                              ActionType::listCollections)};
        }

        bool isInitialSource() const final {
            return true;
        }

    private:
        NamespaceString _nss;
    };

    explicit DocumentSourceListCatalog(const boost::intrusive_ptr<ExpressionContext>& expCtx)
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
        return Value(Document{{kStageName, Document()}});
    }

    StageConstraints constraints(Pipeline::SplitState pipeState) const final {
        StageConstraints constraints(StreamType::kStreaming,
                                     PositionRequirement::kFirst,
                                     HostTypeRequirement::kNone,
                                     DiskUseRequirement::kNoDiskUse,
                                     FacetRequirement::kNotAllowed,
                                     TransactionRequirement::kAllowed);
        constraints.requiresInputDocSource = false;
        constraints.isIndependentOfAnyCollection = true;
        return constraints;
    }

    static boost::intrusive_ptr<DocumentSource> createFromBson(
        BSONElement elem, const boost::intrusive_ptr<ExpressionContext>& expCtx) {
        uassert(ErrorCodes::FailedToParse,
                "The $listCatalog stage specification must be an empty object",
                elem.type() == BSONType::Object && elem.Obj().isEmpty());
        return new DocumentSourceListCatalog(expCtx);
    }

private:
    void loadEntries() {
        Lock::DBLock dbLock(pExpCtx->opCtx, pExpCtx->ns.db(), MODE_IS);
        Database* db = DatabaseHolder::getDatabaseHolder().get(pExpCtx->opCtx, pExpCtx->ns.db());
        if (!db) {
            return;
        }

        if (!pExpCtx->ns.isCollectionlessAggregateNS()) {
            Collection* collection = db->getCollection(pExpCtx->opCtx, pExpCtx->ns);
            if (collection) {
                _entries.push_back(buildCatalogEntry(pExpCtx->opCtx, collection));
            }
            return;
        }

        for (const auto& [name, collection] : db->collections(pExpCtx->opCtx)) {
            if (collection->ns().coll() == "system.namespaces") {
                continue;
            }
            _entries.push_back(buildCatalogEntry(pExpCtx->opCtx, collection.get()));
        }
    }

    bool _loaded = false;
    std::vector<BSONObj> _entries;
    size_t _current = 0;
};

constexpr StringData DocumentSourceListCatalog::kStageName;

}  // namespace

REGISTER_DOCUMENT_SOURCE(listCatalog,
                         DocumentSourceListCatalog::LiteParsed::parse,
                         DocumentSourceListCatalog::createFromBson);

}  // namespace mongo
