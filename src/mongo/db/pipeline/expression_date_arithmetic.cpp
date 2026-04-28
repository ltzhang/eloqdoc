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

#include "mongo/db/pipeline/document.h"
#include "mongo/db/query/datetime/date_time_support.h"
#include "mongo/util/time_support.h"

namespace mongo {
namespace {

using boost::intrusive_ptr;

enum class DateUnit {
    kYear,
    kQuarter,
    kMonth,
    kWeek,
    kDay,
    kHour,
    kMinute,
    kSecond,
    kMillisecond,
};

StringData unitStr(DateUnit unit) {
    switch (unit) {
        case DateUnit::kYear:        return "year"_sd;
        case DateUnit::kQuarter:     return "quarter"_sd;
        case DateUnit::kMonth:       return "month"_sd;
        case DateUnit::kWeek:        return "week"_sd;
        case DateUnit::kDay:         return "day"_sd;
        case DateUnit::kHour:        return "hour"_sd;
        case DateUnit::kMinute:      return "minute"_sd;
        case DateUnit::kSecond:      return "second"_sd;
        case DateUnit::kMillisecond: return "millisecond"_sd;
    }
    MONGO_UNREACHABLE;
}

DateUnit parseUnit(StringData unit) {
    if (unit == "year") {
        return DateUnit::kYear;
    } else if (unit == "quarter") {
        return DateUnit::kQuarter;
    } else if (unit == "month") {
        return DateUnit::kMonth;
    } else if (unit == "week") {
        return DateUnit::kWeek;
    } else if (unit == "day") {
        return DateUnit::kDay;
    } else if (unit == "hour") {
        return DateUnit::kHour;
    } else if (unit == "minute") {
        return DateUnit::kMinute;
    } else if (unit == "second") {
        return DateUnit::kSecond;
    } else if (unit == "millisecond") {
        return DateUnit::kMillisecond;
    }
    uasserted(51240, str::stream() << "unknown date unit '" << unit << "'");
}

long long requirePositiveInt(StringData opName, StringData fieldName, const Value& value) {
    uassert(51241,
            str::stream() << opName << " requires '" << fieldName << "' to be an integer",
            value.numeric() && value.integral64Bit());
    auto out = value.coerceToLong();
    uassert(51242,
            str::stream() << opName << " requires '" << fieldName << "' to be positive",
            out > 0);
    return out;
}

long long requireInt(StringData opName, StringData fieldName, const Value& value) {
    uassert(51243,
            str::stream() << opName << " requires '" << fieldName << "' to be an integer",
            value.numeric() && value.integral64Bit());
    return value.coerceToLong();
}

TimeZone makeTimeZone(const intrusive_ptr<ExpressionContext>& expCtx,
                      const Document& root,
                      const intrusive_ptr<Expression>& timezone) {
    if (!timezone) {
        return TimeZoneDatabase::utcZone();
    }
    Value tzValue = timezone->evaluate(root);
    if (tzValue.nullish()) {
        return TimeZoneDatabase::utcZone();
    }
    uassert(51244,
            str::stream() << "timezone must evaluate to a string, found "
                          << typeName(tzValue.getType()),
            tzValue.getType() == BSONType::String);
    return expCtx->timeZoneDatabase->getTimeZone(tzValue.getString());
}

bool isLeapYear(long long year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(long long year, long long month) {
    static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return month == 2 && isLeapYear(year) ? 29 : kDays[month - 1];
}

Date_t addMonths(Date_t date, long long months, const TimeZone& tz) {
    auto parts = tz.dateParts(date);
    long long monthIndex = static_cast<long long>(parts.year) * 12 + (parts.month - 1) + months;
    long long year = monthIndex / 12;
    long long month = monthIndex % 12 + 1;
    if (month <= 0) {
        month += 12;
        --year;
    }
    long long day = std::min<long long>(parts.dayOfMonth, daysInMonth(year, month));
    return tz.createFromDateParts(
        year, month, day, parts.hour, parts.minute, parts.second, parts.millisecond);
}

Date_t addDate(Date_t date, DateUnit unit, long long amount, const TimeZone& tz) {
    auto parts = tz.dateParts(date);
    switch (unit) {
        case DateUnit::kYear:
            return addMonths(date, amount * 12, tz);
        case DateUnit::kQuarter:
            return addMonths(date, amount * 3, tz);
        case DateUnit::kMonth:
            return addMonths(date, amount, tz);
        case DateUnit::kWeek:
            return tz.createFromDateParts(parts.year,
                                          parts.month,
                                          parts.dayOfMonth + amount * 7,
                                          parts.hour,
                                          parts.minute,
                                          parts.second,
                                          parts.millisecond);
        case DateUnit::kDay:
            return tz.createFromDateParts(parts.year,
                                          parts.month,
                                          parts.dayOfMonth + amount,
                                          parts.hour,
                                          parts.minute,
                                          parts.second,
                                          parts.millisecond);
        case DateUnit::kHour:
            return date + Hours(amount);
        case DateUnit::kMinute:
            return date + Minutes(amount);
        case DateUnit::kSecond:
            return date + Seconds(amount);
        case DateUnit::kMillisecond:
            return date + Milliseconds(amount);
    }
    MONGO_UNREACHABLE;
}

long long floorDiv(long long value, long long divisor) {
    long long q = value / divisor;
    long long r = value % divisor;
    return (r != 0 && ((r < 0) != (divisor < 0))) ? q - 1 : q;
}

StringData weekdayName(int day) {
    switch (day) {
        case 0: return "sunday"_sd;
        case 1: return "monday"_sd;
        case 2: return "tuesday"_sd;
        case 3: return "wednesday"_sd;
        case 4: return "thursday"_sd;
        case 5: return "friday"_sd;
        case 6: return "saturday"_sd;
    }
    MONGO_UNREACHABLE;
}

int weekdayIndex(StringData day) {
    if (day == "sunday") {
        return 0;
    } else if (day == "monday") {
        return 1;
    } else if (day == "tuesday") {
        return 2;
    } else if (day == "wednesday") {
        return 3;
    } else if (day == "thursday") {
        return 4;
    } else if (day == "friday") {
        return 5;
    } else if (day == "saturday") {
        return 6;
    }
    uasserted(51245, str::stream() << "unknown startOfWeek '" << day << "'");
}

Date_t truncateDate(Date_t date, DateUnit unit, long long binSize, int startOfWeek, const TimeZone& tz) {
    auto parts = tz.dateParts(date);
    switch (unit) {
        case DateUnit::kYear: {
            long long year = floorDiv(parts.year - 2000, binSize) * binSize + 2000;
            return tz.createFromDateParts(year, 1, 1, 0, 0, 0, 0);
        }
        case DateUnit::kQuarter: {
            long long quarter = (parts.month - 1) / 3;
            long long newQuarter = floorDiv(quarter, binSize) * binSize;
            return tz.createFromDateParts(parts.year, newQuarter * 3 + 1, 1, 0, 0, 0, 0);
        }
        case DateUnit::kMonth: {
            long long month = floorDiv(parts.month - 1, binSize) * binSize + 1;
            return tz.createFromDateParts(parts.year, month, 1, 0, 0, 0, 0);
        }
        case DateUnit::kWeek: {
            Date_t day = tz.createFromDateParts(parts.year, parts.month, parts.dayOfMonth, 0, 0, 0, 0);
            int current = tz.dayOfWeek(day) - 1;
            int delta = (current - startOfWeek + 7) % 7;
            return addDate(day, DateUnit::kDay, -delta, tz);
        }
        case DateUnit::kDay:
            return tz.createFromDateParts(parts.year,
                                          parts.month,
                                          floorDiv(parts.dayOfMonth - 1, binSize) * binSize + 1,
                                          0,
                                          0,
                                          0,
                                          0);
        case DateUnit::kHour:
            return tz.createFromDateParts(parts.year,
                                          parts.month,
                                          parts.dayOfMonth,
                                          floorDiv(parts.hour, binSize) * binSize,
                                          0,
                                          0,
                                          0);
        case DateUnit::kMinute:
            return tz.createFromDateParts(parts.year,
                                          parts.month,
                                          parts.dayOfMonth,
                                          parts.hour,
                                          floorDiv(parts.minute, binSize) * binSize,
                                          0,
                                          0);
        case DateUnit::kSecond:
            return tz.createFromDateParts(parts.year,
                                          parts.month,
                                          parts.dayOfMonth,
                                          parts.hour,
                                          parts.minute,
                                          floorDiv(parts.second, binSize) * binSize,
                                          0);
        case DateUnit::kMillisecond:
            return tz.createFromDateParts(parts.year,
                                          parts.month,
                                          parts.dayOfMonth,
                                          parts.hour,
                                          parts.minute,
                                          parts.second,
                                          floorDiv(parts.millisecond, binSize) * binSize);
    }
    MONGO_UNREACHABLE;
}

class DateExpressionBase : public Expression {
public:
    explicit DateExpressionBase(const intrusive_ptr<ExpressionContext>& expCtx) : Expression(expCtx) {}

protected:
    void addCommonDependency(DepsTracker* deps) const {
        if (_timezone) {
            _timezone->addDependencies(deps);
        }
    }

    intrusive_ptr<Expression> _timezone;
};

template <typename SubClass>
class ExpressionDateAddSub final : public DateExpressionBase {
public:
    explicit ExpressionDateAddSub(const intrusive_ptr<ExpressionContext>& expCtx)
        : DateExpressionBase(expCtx) {}

    static intrusive_ptr<Expression> parse(const intrusive_ptr<ExpressionContext>& expCtx,
                                           BSONElement expr,
                                           const VariablesParseState& vps) {
        uassert(51246,
                str::stream() << SubClass::kName << " requires an object argument",
                expr.type() == BSONType::Object);
        intrusive_ptr<ExpressionDateAddSub<SubClass>> out(new ExpressionDateAddSub<SubClass>(expCtx));
        for (auto&& elem : expr.Obj()) {
            auto fieldName = elem.fieldNameStringData();
            if (fieldName == "startDate") {
                out->_startDate = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "amount") {
                out->_amount = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "unit") {
                uassert(51247, "unit must be a string constant", elem.type() == BSONType::String);
                out->_unit = parseUnit(elem.valueStringData());
                out->_unitSpecified = true;
            } else if (fieldName == "timezone") {
                out->_timezone = Expression::parseOperand(expCtx, elem, vps);
            } else {
                uasserted(51248, str::stream() << SubClass::kName << " unknown argument " << fieldName);
            }
        }
        uassert(51249, str::stream() << SubClass::kName << " requires 'startDate'", out->_startDate);
        uassert(51250, str::stream() << SubClass::kName << " requires 'amount'", out->_amount);
        uassert(51264, str::stream() << SubClass::kName << " requires 'unit'", out->_unitSpecified);
        return out;
    }

    Value evaluate(const Document& root) const final {
        auto start = _startDate->evaluate(root);
        auto amountValue = _amount->evaluate(root);
        if (start.nullish() || amountValue.nullish()) {
            return Value(BSONNULL);
        }
        uassert(51251,
                str::stream() << SubClass::kName << " requires 'startDate' to be a date",
                start.getType() == BSONType::Date);
        long long amount = requireInt(SubClass::kName, "amount"_sd, amountValue);
        if (SubClass::kSubtract) {
            amount = -amount;
        }
        return Value(addDate(start.getDate(), _unit, amount, makeTimeZone(getExpressionContext(), root, _timezone)));
    }

    Value serialize(bool explain) const final {
        MutableDocument spec;
        spec.addField("startDate", _startDate->serialize(explain));
        spec.addField("unit", Value(unitStr(_unit)));
        spec.addField("amount", _amount->serialize(explain));
        if (_timezone) {
            spec.addField("timezone", _timezone->serialize(explain));
        }
        return Value(Document{{SubClass::kName, spec.freezeToValue()}});
    }

    void _doAddDependencies(DepsTracker* deps) const final {
        _startDate->addDependencies(deps);
        _amount->addDependencies(deps);
        addCommonDependency(deps);
    }

    const char* getOpName() const {
        return SubClass::kName.rawData();
    }

private:
    intrusive_ptr<Expression> _startDate;
    intrusive_ptr<Expression> _amount;
    DateUnit _unit = DateUnit::kMillisecond;
    bool _unitSpecified = false;
};

class ExpressionDateDiff final : public DateExpressionBase {
public:
    explicit ExpressionDateDiff(const intrusive_ptr<ExpressionContext>& expCtx) : DateExpressionBase(expCtx) {}

    static intrusive_ptr<Expression> parse(const intrusive_ptr<ExpressionContext>& expCtx,
                                           BSONElement expr,
                                           const VariablesParseState& vps) {
        uassert(51252, "$dateDiff requires an object argument", expr.type() == BSONType::Object);
        intrusive_ptr<ExpressionDateDiff> out(new ExpressionDateDiff(expCtx));
        for (auto&& elem : expr.Obj()) {
            auto fieldName = elem.fieldNameStringData();
            if (fieldName == "startDate") {
                out->_startDate = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "endDate") {
                out->_endDate = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "unit") {
                uassert(51253, "unit must be a string constant", elem.type() == BSONType::String);
                out->_unit = parseUnit(elem.valueStringData());
                out->_unitSpecified = true;
            } else if (fieldName == "timezone") {
                out->_timezone = Expression::parseOperand(expCtx, elem, vps);
            } else {
                uasserted(51254, str::stream() << "$dateDiff unknown argument " << fieldName);
            }
        }
        uassert(51255, "$dateDiff requires 'startDate'", out->_startDate);
        uassert(51256, "$dateDiff requires 'endDate'", out->_endDate);
        uassert(51265, "$dateDiff requires 'unit'", out->_unitSpecified);
        return out;
    }

    Value evaluate(const Document& root) const final {
        auto start = _startDate->evaluate(root);
        auto end = _endDate->evaluate(root);
        if (start.nullish() || end.nullish()) {
            return Value(BSONNULL);
        }
        uassert(51257, "$dateDiff requires date inputs", start.getType() == Date && end.getType() == Date);
        long long millis = (end.getDate() - start.getDate()).count();
        switch (_unit) {
            case DateUnit::kMillisecond:
                return Value(millis);
            case DateUnit::kSecond:
                return Value(floorDiv(millis, 1000));
            case DateUnit::kMinute:
                return Value(floorDiv(millis, 60 * 1000));
            case DateUnit::kHour:
                return Value(floorDiv(millis, 60 * 60 * 1000));
            case DateUnit::kDay:
                return Value(floorDiv(millis, 24LL * 60 * 60 * 1000));
            case DateUnit::kWeek:
                return Value(floorDiv(millis, 7LL * 24 * 60 * 60 * 1000));
            case DateUnit::kMonth: {
                auto tz = makeTimeZone(getExpressionContext(), root, _timezone);
                auto s = tz.dateParts(start.getDate());
                auto e = tz.dateParts(end.getDate());
                return Value(static_cast<long long>((e.year - s.year) * 12 + (e.month - s.month)));
            }
            case DateUnit::kQuarter: {
                auto months = ExpressionDateDiff::monthDiff(root, start.getDate(), end.getDate());
                return Value(floorDiv(months, 3));
            }
            case DateUnit::kYear: {
                auto months = ExpressionDateDiff::monthDiff(root, start.getDate(), end.getDate());
                return Value(floorDiv(months, 12));
            }
        }
        MONGO_UNREACHABLE;
    }

    static long long monthDiff(const Document& root, Date_t start, Date_t end) {
        auto s = TimeZoneDatabase::utcZone().dateParts(start);
        auto e = TimeZoneDatabase::utcZone().dateParts(end);
        return (e.year - s.year) * 12 + (e.month - s.month);
    }

    Value serialize(bool explain) const final {
        MutableDocument spec;
        spec.addField("startDate", _startDate->serialize(explain));
        spec.addField("endDate", _endDate->serialize(explain));
        spec.addField("unit", Value(unitStr(_unit)));
        if (_timezone) {
            spec.addField("timezone", _timezone->serialize(explain));
        }
        return Value(Document{{"$dateDiff", spec.freezeToValue()}});
    }

    void _doAddDependencies(DepsTracker* deps) const final {
        _startDate->addDependencies(deps);
        _endDate->addDependencies(deps);
        addCommonDependency(deps);
    }

    const char* getOpName() const {
        return "$dateDiff";
    }

private:
    intrusive_ptr<Expression> _startDate;
    intrusive_ptr<Expression> _endDate;
    DateUnit _unit = DateUnit::kMillisecond;
    bool _unitSpecified = false;
};

class ExpressionDateTrunc final : public DateExpressionBase {
public:
    explicit ExpressionDateTrunc(const intrusive_ptr<ExpressionContext>& expCtx) : DateExpressionBase(expCtx) {}

    static intrusive_ptr<Expression> parse(const intrusive_ptr<ExpressionContext>& expCtx,
                                           BSONElement expr,
                                           const VariablesParseState& vps) {
        uassert(51258, "$dateTrunc requires an object argument", expr.type() == BSONType::Object);
        intrusive_ptr<ExpressionDateTrunc> out(new ExpressionDateTrunc(expCtx));
        out->_binSize = ExpressionConstant::create(expCtx, Value(1));
        for (auto&& elem : expr.Obj()) {
            auto fieldName = elem.fieldNameStringData();
            if (fieldName == "date") {
                out->_date = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "unit") {
                uassert(51259, "unit must be a string constant", elem.type() == BSONType::String);
                out->_unit = parseUnit(elem.valueStringData());
                out->_unitSpecified = true;
            } else if (fieldName == "binSize") {
                out->_binSize = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "timezone") {
                out->_timezone = Expression::parseOperand(expCtx, elem, vps);
            } else if (fieldName == "startOfWeek") {
                uassert(51260,
                        "startOfWeek must be a string constant",
                        elem.type() == BSONType::String);
                out->_startOfWeek = weekdayIndex(elem.valueStringData());
            } else {
                uasserted(51261, str::stream() << "$dateTrunc unknown argument " << fieldName);
            }
        }
        uassert(51262, "$dateTrunc requires 'date'", out->_date);
        uassert(51266, "$dateTrunc requires 'unit'", out->_unitSpecified);
        return out;
    }

    Value evaluate(const Document& root) const final {
        auto date = _date->evaluate(root);
        auto binSizeValue = _binSize->evaluate(root);
        if (date.nullish() || binSizeValue.nullish()) {
            return Value(BSONNULL);
        }
        uassert(51263, "$dateTrunc requires 'date' to be a date", date.getType() == Date);
        long long binSize = requirePositiveInt("$dateTrunc"_sd, "binSize"_sd, binSizeValue);
        return Value(truncateDate(date.getDate(),
                                  _unit,
                                  binSize,
                                  _startOfWeek,
                                  makeTimeZone(getExpressionContext(), root, _timezone)));
    }

    Value serialize(bool explain) const final {
        MutableDocument spec;
        spec.addField("date", _date->serialize(explain));
        spec.addField("unit", Value(unitStr(_unit)));
        spec.addField("binSize", _binSize->serialize(explain));
        if (_timezone) {
            spec.addField("timezone", _timezone->serialize(explain));
        }
        if (_unit == DateUnit::kWeek) {
            spec.addField("startOfWeek", Value(weekdayName(_startOfWeek)));
        }
        return Value(Document{{"$dateTrunc", spec.freezeToValue()}});
    }

    void _doAddDependencies(DepsTracker* deps) const final {
        _date->addDependencies(deps);
        _binSize->addDependencies(deps);
        addCommonDependency(deps);
    }

    const char* getOpName() const {
        return "$dateTrunc";
    }

private:
    intrusive_ptr<Expression> _date;
    intrusive_ptr<Expression> _binSize;
    DateUnit _unit = DateUnit::kMillisecond;
    int _startOfWeek = 0;
    bool _unitSpecified = false;
};

struct DateAdd {
    static constexpr StringData kName = "$dateAdd"_sd;
    static constexpr bool kSubtract = false;
};

struct DateSubtract {
    static constexpr StringData kName = "$dateSubtract"_sd;
    static constexpr bool kSubtract = true;
};

}  // namespace

REGISTER_EXPRESSION(dateAdd, ExpressionDateAddSub<DateAdd>::parse);
REGISTER_EXPRESSION(dateSubtract, ExpressionDateAddSub<DateSubtract>::parse);
REGISTER_EXPRESSION(dateDiff, ExpressionDateDiff::parse);
REGISTER_EXPRESSION(dateTrunc, ExpressionDateTrunc::parse);

}  // namespace mongo
