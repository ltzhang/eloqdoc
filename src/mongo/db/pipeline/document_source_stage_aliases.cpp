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

#include "mongo/db/jsobj.h"
#include "mongo/db/pipeline/document_source_add_fields.h"
#include "mongo/db/pipeline/document_source_project.h"
#include "mongo/db/pipeline/document_source_replace_root.h"
#include "mongo/db/pipeline/lite_parsed_document_source.h"

namespace mongo {
namespace {

using boost::intrusive_ptr;
using std::list;

intrusive_ptr<DocumentSource> createSetFromBson(BSONElement elem,
                                                const intrusive_ptr<ExpressionContext>& expCtx) {
    return DocumentSourceAddFields::createFromBson(elem, expCtx);
}

list<intrusive_ptr<DocumentSource>> createUnsetFromBson(
    BSONElement elem, const intrusive_ptr<ExpressionContext>& expCtx) {
    BSONObjBuilder projectBuilder;

    if (elem.type() == String) {
        projectBuilder.append(elem.valueStringData(), false);
    } else if (elem.type() == Array) {
        for (auto&& field : elem.Obj()) {
            uassert(ErrorCodes::TypeMismatch,
                    str::stream() << "$unset entries must be strings, got "
                                  << typeName(field.type()),
                    field.type() == String);
            projectBuilder.append(field.valueStringData(), false);
        }
    } else {
        uasserted(ErrorCodes::TypeMismatch,
                  str::stream() << "$unset specification must be a string or an array of strings, "
                                << "got " << typeName(elem.type()));
    }

    BSONObj projectObj = BSON("$project" << projectBuilder.obj());
    return {DocumentSourceProject::createFromBson(projectObj.firstElement(), expCtx)};
}

list<intrusive_ptr<DocumentSource>> createReplaceWithFromBson(
    BSONElement elem, const intrusive_ptr<ExpressionContext>& expCtx) {
    BSONObj replaceRootObj = BSON("$replaceRoot" << BSON("newRoot" << elem));
    return {DocumentSourceReplaceRoot::createFromBson(replaceRootObj.firstElement(), expCtx)};
}

}  // namespace

REGISTER_DOCUMENT_SOURCE(set, LiteParsedDocumentSourceDefault::parse, createSetFromBson);
REGISTER_MULTI_STAGE_ALIAS(unset, LiteParsedDocumentSourceDefault::parse, createUnsetFromBson);
REGISTER_MULTI_STAGE_ALIAS(replaceWith,
                           LiteParsedDocumentSourceDefault::parse,
                           createReplaceWithFromBson);

}  // namespace mongo
