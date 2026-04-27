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

#include "mongo/db/pipeline/expression.h"

#include <algorithm>
#include <memory>
#include <pcre.h>

#include "mongo/db/pipeline/document.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {
namespace {

using boost::intrusive_ptr;

constexpr int kMaxRegexCaptures = 64;

int optionBits(StringData options) {
    int bits = PCRE_UTF8;
    for (char option : options) {
        switch (option) {
            case 'i':
                bits |= PCRE_CASELESS;
                break;
            case 'm':
                bits |= PCRE_MULTILINE;
                break;
            case 's':
                bits |= PCRE_DOTALL;
                break;
            case 'x':
                bits |= PCRE_EXTENDED;
                break;
            default:
                uasserted(51110, str::stream() << "invalid regex option '" << option << "'");
        }
    }
    return bits;
}

struct PcreDeleter {
    void operator()(pcre* compiled) const {
        if (compiled) {
            pcre_free(compiled);
        }
    }
};

class RegexPattern {
public:
    RegexPattern(std::string pattern, std::string options)
        : _pattern(std::move(pattern)), _options(std::move(options)) {
        const char* error = nullptr;
        int errorOffset = 0;
        _compiled.reset(
            pcre_compile(_pattern.c_str(), optionBits(_options), &error, &errorOffset, nullptr));
        uassert(51111,
                str::stream() << "invalid regex pattern at offset " << errorOffset << ": "
                              << (error ? error : "unknown error"),
                _compiled);
        pcre_fullinfo(_compiled.get(), nullptr, PCRE_INFO_CAPTURECOUNT, &_captureCount);
    }

    int captureCount() const {
        return _captureCount;
    }

    bool match(StringData input, int startByte, std::vector<int>* offsets) const {
        offsets->assign((std::min(_captureCount, kMaxRegexCaptures) + 1) * 3, -1);
        int rc = pcre_exec(_compiled.get(),
                           nullptr,
                           input.rawData(),
                           input.size(),
                           startByte,
                           0,
                           offsets->data(),
                           offsets->size());
        if (rc == PCRE_ERROR_NOMATCH) {
            return false;
        }
        uassert(51112, str::stream() << "regex execution failed with code " << rc, rc >= 0);
        return true;
    }

private:
    std::string _pattern;
    std::string _options;
    std::unique_ptr<pcre, PcreDeleter> _compiled;
    int _captureCount = 0;
};

struct RegexArgs {
    Value input;
    Value regex;
    Value options;
    bool optionsSpecified = false;
};

std::string requireString(StringData opName, StringData fieldName, const Value& value) {
    uassert(51113,
            str::stream() << opName << " requires '" << fieldName << "' to evaluate to a string",
            value.getType() == BSONType::String);
    return value.getString();
}

RegexPattern makeRegex(StringData opName, const Value& regex, const Value& options) {
    std::string optionString;
    if (!options.nullish()) {
        optionString = requireString(opName, "options"_sd, options);
    }

    if (regex.getType() == BSONType::RegEx) {
        uassert(51114,
                str::stream() << opName
                              << " cannot specify options in both 'regex' and 'options'",
                optionString.empty() || StringData(regex.getRegexFlags()).empty());
        return RegexPattern(regex.getRegex(), optionString.empty() ? regex.getRegexFlags()
                                                                   : optionString);
    }

    return RegexPattern(requireString(opName, "regex"_sd, regex), optionString);
}

int codePointIndex(StringData input, int byteOffset) {
    return static_cast<int>(mongoutils::str::lengthInUTF8CodePoints(input.substr(0, byteOffset)));
}

Value regexMatchDocument(StringData input, const std::vector<int>& offsets, int captureCount) {
    std::vector<Value> captures;
    int returnedCaptures = std::min(captureCount, kMaxRegexCaptures);
    captures.reserve(returnedCaptures);
    for (int i = 1; i <= returnedCaptures; ++i) {
        int start = offsets[i * 2];
        int end = offsets[i * 2 + 1];
        captures.push_back(start < 0 ? Value(BSONNULL)
                                     : Value(input.substr(start, end - start).toString()));
    }

    int matchStart = offsets[0];
    int matchEnd = offsets[1];
    return Value(Document{{"match", input.substr(matchStart, matchEnd - matchStart).toString()},
                          {"idx", codePointIndex(input, matchStart)},
                          {"captures", std::move(captures)}});
}

template <typename SubClass>
class RegexExpressionBase : public Expression {
public:
    explicit RegexExpressionBase(const intrusive_ptr<ExpressionContext>& expCtx) : Expression(expCtx) {}

    static intrusive_ptr<Expression> parse(const intrusive_ptr<ExpressionContext>& expCtx,
                                           BSONElement expr,
                                           const VariablesParseState& vps) {
        uassert(51115,
                str::stream() << SubClass::kName << " requires an object argument",
                expr.type() == BSONType::Object);

        intrusive_ptr<SubClass> out(new SubClass(expCtx));
        for (auto&& elem : expr.Obj()) {
            auto fieldName = elem.fieldNameStringData();
            if (fieldName == "input") {
                out->_input = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "regex") {
                out->_regex = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "options") {
                out->_options = Expression::parseOperand(expCtx, elem, vps);
            } else {
                uasserted(51116,
                          str::stream() << SubClass::kName << " found unknown argument '"
                                        << fieldName << "'");
            }
        }

        uassert(51117, str::stream() << SubClass::kName << " requires 'input'", out->_input);
        uassert(51118, str::stream() << SubClass::kName << " requires 'regex'", out->_regex);
        return out;
    }

    Value serialize(bool explain) const final {
        MutableDocument spec;
        spec.addField("input", _input->serialize(explain));
        spec.addField("regex", _regex->serialize(explain));
        if (_options) {
            spec.addField("options", _options->serialize(explain));
        }
        return Value(Document{{SubClass::kName, spec.freezeToValue()}});
    }

protected:
    RegexArgs evaluateArgs(const Document& root) const {
        return {_input->evaluate(root),
                _regex->evaluate(root),
                _options ? _options->evaluate(root) : Value(),
                static_cast<bool>(_options)};
    }

    void _doAddDependencies(DepsTracker* deps) const final {
        _input->addDependencies(deps);
        _regex->addDependencies(deps);
        if (_options) {
            _options->addDependencies(deps);
        }
    }

    intrusive_ptr<Expression> _input;
    intrusive_ptr<Expression> _regex;
    intrusive_ptr<Expression> _options;
};

class ExpressionRegexMatch final : public RegexExpressionBase<ExpressionRegexMatch> {
public:
    static constexpr StringData kName = "$regexMatch"_sd;

    explicit ExpressionRegexMatch(const intrusive_ptr<ExpressionContext>& expCtx)
        : RegexExpressionBase<ExpressionRegexMatch>(expCtx) {}

    Value evaluate(const Document& root) const final {
        auto args = evaluateArgs(root);
        if (args.input.nullish() || args.regex.nullish() ||
            (args.optionsSpecified && args.options.nullish())) {
            return Value(BSONNULL);
        }

        auto input = requireString(kName, "input"_sd, args.input);
        auto regex = makeRegex(kName, args.regex, args.options);
        std::vector<int> offsets;
        return Value(regex.match(input, 0, &offsets));
    }

    const char* getOpName() const {
        return "$regexMatch";
    }
};

class ExpressionRegexFind final : public RegexExpressionBase<ExpressionRegexFind> {
public:
    static constexpr StringData kName = "$regexFind"_sd;

    explicit ExpressionRegexFind(const intrusive_ptr<ExpressionContext>& expCtx)
        : RegexExpressionBase<ExpressionRegexFind>(expCtx) {}

    Value evaluate(const Document& root) const final {
        auto args = evaluateArgs(root);
        if (args.input.nullish() || args.regex.nullish() ||
            (args.optionsSpecified && args.options.nullish())) {
            return Value(BSONNULL);
        }

        auto input = requireString(kName, "input"_sd, args.input);
        auto regex = makeRegex(kName, args.regex, args.options);
        std::vector<int> offsets;
        if (!regex.match(input, 0, &offsets)) {
            return Value(BSONNULL);
        }
        return regexMatchDocument(input, offsets, regex.captureCount());
    }

    const char* getOpName() const {
        return "$regexFind";
    }
};

class ExpressionRegexFindAll final : public RegexExpressionBase<ExpressionRegexFindAll> {
public:
    static constexpr StringData kName = "$regexFindAll"_sd;

    explicit ExpressionRegexFindAll(const intrusive_ptr<ExpressionContext>& expCtx)
        : RegexExpressionBase<ExpressionRegexFindAll>(expCtx) {}

    Value evaluate(const Document& root) const final {
        auto args = evaluateArgs(root);
        if (args.input.nullish() || args.regex.nullish() ||
            (args.optionsSpecified && args.options.nullish())) {
            return Value(BSONNULL);
        }

        auto input = requireString(kName, "input"_sd, args.input);
        auto regex = makeRegex(kName, args.regex, args.options);
        std::vector<Value> matches;
        std::vector<int> offsets;
        int start = 0;
        while (start <= static_cast<int>(input.size()) && regex.match(input, start, &offsets)) {
            matches.push_back(regexMatchDocument(input, offsets, regex.captureCount()));
            int matchStart = offsets[0];
            int matchEnd = offsets[1];
            if (matchEnd > matchStart) {
                start = matchEnd;
            } else if (matchEnd < static_cast<int>(input.size())) {
                start = matchEnd + 1;
            } else {
                break;
            }
        }
        return Value(std::move(matches));
    }

    const char* getOpName() const {
        return "$regexFindAll";
    }
};

template <typename SubClass>
class ReplaceExpressionBase : public Expression {
public:
    explicit ReplaceExpressionBase(const intrusive_ptr<ExpressionContext>& expCtx) : Expression(expCtx) {}

    static intrusive_ptr<Expression> parse(const intrusive_ptr<ExpressionContext>& expCtx,
                                           BSONElement expr,
                                           const VariablesParseState& vps) {
        uassert(51119,
                str::stream() << SubClass::kName << " requires an object argument",
                expr.type() == BSONType::Object);

        intrusive_ptr<SubClass> out(new SubClass(expCtx));
        for (auto&& elem : expr.Obj()) {
            auto fieldName = elem.fieldNameStringData();
            if (fieldName == "input") {
                out->_input = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "find") {
                out->_find = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "replacement") {
                out->_replacement = Expression::parseOperand(expCtx, elem, vps);
            } else {
                uasserted(51120,
                          str::stream() << SubClass::kName << " found unknown argument '"
                                        << fieldName << "'");
            }
        }

        uassert(51121, str::stream() << SubClass::kName << " requires 'input'", out->_input);
        uassert(51122, str::stream() << SubClass::kName << " requires 'find'", out->_find);
        uassert(51123,
                str::stream() << SubClass::kName << " requires 'replacement'",
                out->_replacement);
        return out;
    }

    Value evaluate(const Document& root) const final {
        auto input = _input->evaluate(root);
        auto find = _find->evaluate(root);
        auto replacement = _replacement->evaluate(root);
        if (input.nullish() || find.nullish() || replacement.nullish()) {
            return Value(BSONNULL);
        }

        std::string result = requireString(SubClass::kName, "input"_sd, input);
        std::string needle = requireString(SubClass::kName, "find"_sd, find);
        std::string replacementString = requireString(SubClass::kName, "replacement"_sd, replacement);
        uassert(51124,
                str::stream() << SubClass::kName << " requires 'find' to be a non-empty string",
                !needle.empty());

        size_t pos = result.find(needle);
        while (pos != std::string::npos) {
            result.replace(pos, needle.size(), replacementString);
            if (!SubClass::kReplaceAll) {
                break;
            }
            pos = result.find(needle, pos + replacementString.size());
        }
        return Value(result);
    }

    Value serialize(bool explain) const final {
        MutableDocument spec;
        spec.addField("input", _input->serialize(explain));
        spec.addField("find", _find->serialize(explain));
        spec.addField("replacement", _replacement->serialize(explain));
        return Value(Document{{SubClass::kName, spec.freezeToValue()}});
    }

protected:
    void _doAddDependencies(DepsTracker* deps) const final {
        _input->addDependencies(deps);
        _find->addDependencies(deps);
        _replacement->addDependencies(deps);
    }

    intrusive_ptr<Expression> _input;
    intrusive_ptr<Expression> _find;
    intrusive_ptr<Expression> _replacement;
};

class ExpressionReplaceOne final : public ReplaceExpressionBase<ExpressionReplaceOne> {
public:
    static constexpr StringData kName = "$replaceOne"_sd;
    static constexpr bool kReplaceAll = false;

    explicit ExpressionReplaceOne(const intrusive_ptr<ExpressionContext>& expCtx)
        : ReplaceExpressionBase<ExpressionReplaceOne>(expCtx) {}

    const char* getOpName() const {
        return "$replaceOne";
    }
};

class ExpressionReplaceAll final : public ReplaceExpressionBase<ExpressionReplaceAll> {
public:
    static constexpr StringData kName = "$replaceAll"_sd;
    static constexpr bool kReplaceAll = true;

    explicit ExpressionReplaceAll(const intrusive_ptr<ExpressionContext>& expCtx)
        : ReplaceExpressionBase<ExpressionReplaceAll>(expCtx) {}

    const char* getOpName() const {
        return "$replaceAll";
    }
};

}  // namespace

REGISTER_EXPRESSION(regexMatch, ExpressionRegexMatch::parse);
REGISTER_EXPRESSION(regexFind, ExpressionRegexFind::parse);
REGISTER_EXPRESSION(regexFindAll, ExpressionRegexFindAll::parse);
REGISTER_EXPRESSION(replaceOne, ExpressionReplaceOne::parse);
REGISTER_EXPRESSION(replaceAll, ExpressionReplaceAll::parse);

}  // namespace mongo
