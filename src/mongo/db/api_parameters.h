/**
 *    Copyright (C) 2018 MongoDB Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version. If you
 *    do not wish to do so, delete this exception statement from your version.
 *    If you delete this exception statement from all source files in the
 *    program, then also delete it in the license file.
 */

#pragma once

#include "mongo/base/error_codes.h"
#include "mongo/db/jsobj.h"
#include "mongo/util/assert_util.h"

namespace mongo {

struct APIParameters {
    bool versionPresent = false;
    bool strict = false;
    bool deprecationErrors = false;

    static APIParameters parse(const BSONObj& cmdObj) {
        APIParameters parsed;

        BSONElement apiVersion = cmdObj["apiVersion"];
        if (!apiVersion.eoo()) {
            uassert(ErrorCodes::APIVersionError,
                    "apiVersion must be the string \"1\"",
                    apiVersion.type() == BSONType::String && apiVersion.str() == "1");
            parsed.versionPresent = true;
        }

        BSONElement apiStrict = cmdObj["apiStrict"];
        if (!apiStrict.eoo()) {
            uassert(ErrorCodes::APIVersionError,
                    "apiStrict must be a boolean",
                    apiStrict.type() == BSONType::Bool);
            parsed.strict = apiStrict.boolean();
        }

        BSONElement apiDeprecationErrors = cmdObj["apiDeprecationErrors"];
        if (!apiDeprecationErrors.eoo()) {
            uassert(ErrorCodes::APIVersionError,
                    "apiDeprecationErrors must be a boolean",
                    apiDeprecationErrors.type() == BSONType::Bool);
            parsed.deprecationErrors = apiDeprecationErrors.boolean();
        }

        return parsed;
    }
};

}  // namespace mongo
