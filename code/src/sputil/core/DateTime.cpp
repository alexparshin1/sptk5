/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <cmath>
#include <cstring>
#include <ctime>
#include <sptk5/cutils>

using namespace std;
using namespace chrono;
using namespace sptk;

namespace sptk {

class SP_EXPORT DateTimeFormat
{
public:
    DateTimeFormat() noexcept;

    static void init() noexcept;

    static char parseDateOrTime(String& format, const String& dateOrTime);
};

} // namespace sptk

static const array<short, 12> gRegularYear = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const array<short, 12> gLeapYear = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

String DateTime::_dateFormat;
String DateTime::_datePartsOrder;
String DateTime::_fullTimeFormat;
String DateTime::_shortTimeFormat;
char DateTime::_dateSeparator;
char DateTime::_timeSeparator;
Strings DateTime::_weekDayNames;
Strings DateTime::_monthNames;

bool DateTime::_time24Mode;
String DateTime::_timeZoneName;
minutes DateTime::_timeZoneOffset;
int DateTime::_isDaylightSavingsTime;

constexpr int minutesInHour = 60;
constexpr int secondsInMinute = 60;
constexpr int tzMultiplier = 100;
constexpr double millisecondsInSecond = 1000.0;
constexpr size_t maxDateTimeStringLength = 128;

// Returns timezone offset in minutes from formats:
// "Z" - UTC
// "[+-]HH24:MM - TZ offset
static int decodeTZOffset(const char* tzOffset)
{
    const char* ptr = tzOffset;
    int sign = 1;
    switch (*ptr)
    {
        case 'Z':
        case 'z':
            return 0;
        case '+':
            ++ptr;
            break;
        case '-':
            ++ptr;
            sign = -1;
            break;
        default:
            break;
    }

    int minutes = 0;
    int hours = 0;
    if (strlen(ptr) > 2)
    {
        const char* ptr1 = strchr(ptr, ':');
        if (ptr1 != nullptr)
        {
            minutes = string2int(ptr1 + 1);
            hours = string2int(ptr);
        }
        else
        {
            auto hoursAndMinutes = string2int(ptr);
            hours = hoursAndMinutes / 100;
            minutes = hoursAndMinutes % 100;
        }
    }
    else
    {
        hours = string2int(ptr);
    }

    return sign * (hours * minutesInHour + minutes);
}

char DateTimeFormat::parseDateOrTime(String& format, const String& dateOrTime)
{
    // find a separator char
    size_t separatorPos = dateOrTime.find_first_not_of("0123456789 ");
    char separator = dateOrTime[separatorPos];

    const auto* ptr = dateOrTime.c_str();

    format.clear();

    const char* pattern = nullptr;
    while (ptr != nullptr)
    {
        switch (string2int(ptr))
        {
            case 10:
                pattern = "19"; // hour (12-hour mode)
                DateTime::_time24Mode = false;
                break;
            case 22:
                pattern = "29"; // hour (24-hour mode)
                DateTime::_time24Mode = true;
                break;
            case 48:
            case 59:
                pattern = "59"; // second
                break;
            case 17:
                pattern = "39"; // day
                DateTime::_datePartsOrder += "D";
                break;
            case 6:
                pattern = "19"; // month
                DateTime::_datePartsOrder += "M";
                break;
            case 2000:
            case 0:
                pattern = "2999"; // year
                DateTime::_datePartsOrder += "Y";
                break;
            default:
                pattern = nullptr;
                break;
        }
        if (pattern != nullptr)
        {
            format += pattern;
            format += separator;
        }
        if (separatorPos == string::npos)
        {
            break;
        }
        ptr = dateOrTime.c_str() + separatorPos + 1;
        separatorPos = dateOrTime.find(separator, separatorPos + 1);
    }
    format.resize(format.length() - 1);

    return separator;
}

DateTimeFormat::DateTimeFormat() noexcept
{
    init();
}

void DateTimeFormat::init() noexcept
{
    // make a special date and time - today :)
    struct tm atime = {};
    atime.tm_year = 100; // since 1900, -> 2000
    atime.tm_mon = 5;    // June (January=0)
    atime.tm_mday = 17;
    atime.tm_hour = 22;
    atime.tm_min = 48;
    atime.tm_sec = 59;
    atime.tm_wday = 0; // Sunday

#ifdef __linux__
    // For unknown reason this call of setlocale() under Windows makes
    // calls of sprintf to produce access violations. If you know why please
    // tell me..
    setlocale(LC_TIME, "");
#endif
    ::tzset();

    // Build local data and time
    array<char, 32> dateBuffer = {};
    array<char, 32> timeBuffer = {};
    strftime(timeBuffer.data(), 31, "%X", &atime);
    strftime(dateBuffer.data(), 31, "%x", &atime);

    // Build local date and time formats
    DateTime::_datePartsOrder[0] = 0;
    DateTime::_dateSeparator = parseDateOrTime(DateTime::_dateFormat, dateBuffer.data());
    DateTime::time24Mode(timeBuffer[0] == '2');

    // Filling up the week day names, as defined in locale.
    // This date should be Monday:
    atime.tm_year = 103; // since 1900, -> 2003
    atime.tm_mon = 9;
    atime.tm_mday = 21;
    atime.tm_hour = 0;
    atime.tm_min = 0;
    atime.tm_sec = 0;
    DateTime::_weekDayNames.clear();
    const int daysInWeek = 7;
    for (int wday = 0; wday < daysInWeek; ++wday)
    {
        atime.tm_wday = wday;
        strftime(dateBuffer.data(), 32, "%A", &atime);
        DateTime::_weekDayNames.push_back(dateBuffer.data());
    }

    // Filling up the month names, as defined in locale.
    // This date should be January 1st:
    atime.tm_year = 103; // since 1900, -> 2003
    atime.tm_mon = 1;
    atime.tm_mday = 1;
    atime.tm_hour = 0;
    atime.tm_min = 0;
    atime.tm_sec = 0;
    atime.tm_wday = 3;
    DateTime::_monthNames.clear();
    for (int month = 0; month < 12; ++month)
    {
        atime.tm_mon = month;
        strftime(dateBuffer.data(), 32, "%B", &atime);
        DateTime::_monthNames.push_back(dateBuffer.data());
    }
#if defined(__BORLANDC__) || _MSC_VER > 1800
    const char* ptr = _tzname[0];
#else
    const char* ptr = tzname[0];
#endif
    auto len = (int) strlen(ptr);

    if (const char* ptr1 = strchr(ptr, ' '); ptr1 != nullptr)
    {
        len = int(ptr1 - ptr);
    }

    DateTime::_timeZoneName = String(ptr, (unsigned) len);

    time_t ts = time(nullptr);
    array<char, 16> buf {};
    struct tm ltime {
    };
#ifdef _WIN32
    localtime_s(&ltime, &ts);
#else
    localtime_r(&ts, &ltime);
#endif
    strftime(buf.data(), sizeof(buf), "%z", &ltime);
    int offset = string2int(buf.data());
    auto offsetMinutes = minutes(offset % tzMultiplier);
    auto offsetHours = hours(offset / tzMultiplier);
    DateTime::_isDaylightSavingsTime = ltime.tm_isdst == -1 ? 0 : ltime.tm_isdst;
    DateTime::_timeZoneOffset = offsetHours + offsetMinutes;
}

static const DateTimeFormat dateTimeFormatInitializer;

namespace sptk {

#if _WIN32
#define gmtime_r(a, b) gmtime_s(b, a)
#define localtime_r(a, b) localtime_s(b, a)
#endif

static void decodeDate(const DateTime::time_point& timePoint, short& year, short& month, short& day, short& dayOfWeek,
                       short& dayOfYear,
                       bool gmt)
{
    time_t atime = DateTime::clock::to_time_t(timePoint);

    tm time = {};
    if (!gmt)
    {
        atime += DateTime::timeZoneOffset().count() * secondsInMinute;
    }
    gmtime_r(&atime, &time);

    year = (short) (time.tm_year + 1900);
    month = (short) (time.tm_mon + 1);
    day = (short) time.tm_mday;
    dayOfWeek = (short) time.tm_wday;
    dayOfYear = (short) time.tm_yday;
}


static void decodeTime(const DateTime::time_point& timePoint, short& h, short& m, short& s, short& ms, bool gmt)
{
    time_t tt = DateTime::clock::to_time_t(timePoint);

    tm time = {};
    if (!gmt)
    {
        tt += DateTime::timeZoneOffset().count() * secondsInMinute;
    }
    gmtime_r(&tt, &time);

    h = (short) time.tm_hour;
    m = (short) time.tm_min;
    s = (short) time.tm_sec;

    auto dur = duration_cast<milliseconds>(timePoint.time_since_epoch());
    auto sec = duration_cast<seconds>(timePoint.time_since_epoch());
    auto msec = duration_cast<milliseconds>(dur - sec);
    ms = (short) msec.count();
}


static void encodeDate(DateTime::time_point& timePoint, short year, short month, short day)
{
    tm time = {};
    time.tm_year = year - 1900;
    time.tm_mon = month - 1;
    time.tm_mday = day;
    time.tm_isdst = TimeZone::isDaylightSavingsTime();

    time_t atime = mktime(&time);
    timePoint = DateTime::clock::from_time_t(atime);
}

static short splitDateString(const char* bdat, short* datePart, char& actualDateSeparator)
{
    actualDateSeparator = 0;

    const char* ptr = bdat;
    char* end = nullptr;
    size_t partNumber = 0;
    for (; partNumber < 3; ++partNumber)
    {
        errno = 0;
        datePart[partNumber] = (short) strtol(ptr, &end, 10);
        if (errno)
        {
            throw Exception("Invalid date string");
        }

        if (*end == char(0))
        {
            break;
        }

        if (actualDateSeparator == char(0))
        {
            actualDateSeparator = *end;
        }
        else if (actualDateSeparator != *end)
        {
            throw Exception("Invalid date string");
        }

        ptr = end + 1;
    }

    return (short) partNumber;
}


static short splitTimeString(const char* bdat, short* timePart)
{
    static const RegularExpression matchTime("^([0-2]?\\d):([0-5]\\d):([0-5]\\d)(\\.\\d+)?");
    auto matches = matchTime.m(bdat);
    if (!matches)
    {
        throw Exception("Invalid time string");
    }

    int partNumber = 0;
    for (; partNumber < 4; ++partNumber)
    {
        const auto& part = matches[partNumber].value;
        if (part.empty())
        {
            break;
        }
        const auto* value = part.c_str();
        if (partNumber == 3)
        {
            ++value;
        } // Skip dot character
        timePart[partNumber] = (short) strtol(value, nullptr, 10);
    }

    return (short) partNumber;
}

static short correctTwoDigitYear(short year)
{
    if (year < 100)
    {
        if (year < 35)
        {
            year = short(year + 2000);
        }
        else
        {
            year = short(year + 1900);
        }
    }
    return year;
}


static void encodeTime(DateTime::time_point& timePoint, short h, short m, short s, short ms)
{
    timePoint += hours(h) + minutes(m) + seconds(s) + milliseconds(ms);
}


static void encodeTime(DateTime::time_point& dt, const char* tim)
{
    bool afternoon = false;
    array<short, 4> timePart = {0, 0, 0, 0};
    int tzOffsetMin = 0;

    if (const char* p = strpbrk(tim, "apAPZ+-"); p != nullptr)
    {
        // Looking for AM, PM, or timezone
        while (p != nullptr)
        {
            switch (*p)
            {
                case 'P':
                case 'p':
                    afternoon = true;
                    break;
                case 'A':
                case 'a':
                    break;
                case '+':
                case '-':
                    tzOffsetMin = -decodeTZOffset(p);
                    break;
                default:
                    break;
            }
            p = strpbrk(p + 1, "Z+-");
        }
        tzOffsetMin += (int) TimeZone::offset().count();
    }

    if (short partNumber = splitTimeString(tim, timePart.data()); partNumber == 0)
    {
        dt = DateTime::time_point();
        return;
    }

    if (afternoon && timePart[0] != 12)
    {
        timePart[0] = short(timePart[0] + 12);
    }

    encodeTime(dt, timePart[0], timePart[1], timePart[2], timePart[3]);
    dt += minutes(tzOffsetMin);
}


void parseDate(const short* datePart, short& month, short& day, short& year);

static void encodeDate(DateTime::time_point& dt, const char* dat)
{
    array<short, 7> datePart {};

    const char* timePtr = strpbrk(dat, " T");

    char actualDateSeparator = 0;

    if (short partNumber = splitDateString(dat, datePart.data(), actualDateSeparator); partNumber != 0)
    {
        short month = 0;
        short day = 0;
        short year = 0;
        if (actualDateSeparator != DateTime::dateSeparator() && datePart[0] > 31)
        {
            // YYYY-MM-DD format
            year = datePart[0];
            month = datePart[1];
            day = datePart[2];
        }
        else
        {
            parseDate(datePart.data(), month, day, year);
        }

        year = correctTwoDigitYear(year);

        encodeDate(dt, year, month, day);
    }
    else
    {
        dt = DateTime::time_point();
        timePtr = dat;
    }

    if (timePtr != nullptr)
    { // Time part included into string
        DateTime::time_point dtime;
        encodeTime(dtime, timePtr);
        dt += dtime.time_since_epoch();
    }
}

void parseDate(const short* datePart, short& month, short& day, short& year)
{
    auto datePartsOrder(DateTime::format(DateTime::Format::DATE_PARTS_ORDER, 0));
    for (int ii = 0; ii < 3; ++ii)
    {
        switch (datePartsOrder[ii])
        {
            case 'M':
                month = datePart[ii];
                break;
            case 'D':
                day = datePart[ii];
                break;
            case 'Y':
                year = datePart[ii];
                break;
            default:
                break;
        }
    }
}


static int isLeapYear(const int16_t year)
{
    return ((year & 3) == 0 && year % 100) || ((year % 400) == 0);
}

} // namespace sptk

void TimeZone::set(const String& timeZoneName)
{
#ifdef _WIN32
    _putenv_s("TZ", timeZoneName.c_str());
#else
    setenv("TZ", timeZoneName.c_str(), 1);
#endif
    ::tzset();
    dateTimeFormatInitializer.init();
}

void DateTime::time24Mode(bool t24mode)
{
    const char* timeBuffer = "10:48:59AM";

    if (t24mode)
    {
        timeBuffer = "22:48:59";
    }

    _time24Mode = t24mode;
    DateTime::_timeSeparator = DateTimeFormat::parseDateOrTime(DateTime::_fullTimeFormat, timeBuffer);
    DateTime::_shortTimeFormat = DateTime::_fullTimeFormat;

    if (auto pos = DateTime::_fullTimeFormat.find_last_of(DateTime::_timeSeparator); pos != string::npos)
    {
        DateTime::_shortTimeFormat = DateTime::_fullTimeFormat.substr(0, pos);
    }
    if (!_time24Mode)
    {
        DateTime::_fullTimeFormat += "TM";
        DateTime::_shortTimeFormat += "TM";
    }
}

//----------------------------------------------------------------
// Constructors
//----------------------------------------------------------------
DateTime::DateTime(short year, short month, short day, short hour, short minute, short second,
                   short millisecond)
{
    try
    {
        encodeDate(m_dateTime, year, month, day);
        encodeTime(m_dateTime, hour, minute, second, millisecond);
    }
    catch (const Exception&)
    {
        m_dateTime = time_point();
    }
}

DateTime::DateTime(const char* dat)
{
    if (dat == nullptr || *dat == char(0))
    {
        return;
    }

    if (*dat == 'n' && strcmp(dat, "now") == 0)
    {
        m_dateTime = clock::now();
        return;
    }

    while (*dat == ' ')
    {
        ++dat;
    }
    if (*dat == char(0))
    {
        m_dateTime = time_point();
        return;
    }

    Buffer s1(dat);
    char* s2 = strpbrk((char*) s1.data(), " T");
    if (s2 != nullptr)
    {
        *s2 = 0;
        ++s2;
    }

    try
    {
        if (strchr(s1.c_str(), _dateSeparator) != nullptr || strchr(s1.c_str(), '-') != nullptr)
        {
            encodeDate(m_dateTime, s1.c_str());
            if (s2 != nullptr && strchr(s2, _timeSeparator) != nullptr)
            {
                encodeTime(m_dateTime, s2);
            }
        }
        else
        {
            encodeTime(m_dateTime, s1.c_str());
        }
    }
    catch (const Exception&)
    {
        m_dateTime = time_point();
    }
}

DateTime::DateTime(const time_point& dt)
    : m_dateTime(dt)
{
}

DateTime::DateTime(const duration& dt)
    : m_dateTime(dt)
{
}

namespace sptk {

//----------------------------------------------------------------
// Date comparison
//----------------------------------------------------------------
bool operator<(const DateTime& dt1, const DateTime& dt2)
{
    return (dt1.timePoint() < dt2.timePoint());
}

bool operator<=(const DateTime& dt1, const DateTime& dt2)
{
    return (dt1.timePoint() <= dt2.timePoint());
}

bool operator>(const DateTime& dt1, const DateTime& dt2)
{
    return (dt1.timePoint() > dt2.timePoint());
}

bool operator>=(const DateTime& dt1, const DateTime& dt2)
{
    return (dt1.timePoint() >= dt2.timePoint());
}

bool operator==(const DateTime& dt1, const DateTime& dt2)
{
    return (dt1.timePoint() == dt2.timePoint());
}

bool operator!=(const DateTime& dt1, const DateTime& dt2)
{
    return (dt1.timePoint() != dt2.timePoint());
}

DateTime operator+(const DateTime& dt, const sptk::DateTime::duration& duration)
{
    return DateTime(dt.timePoint() + duration);
}

DateTime operator-(const DateTime& dt, const sptk::DateTime::duration& duration)
{
    return DateTime(dt.timePoint() - duration);
}

DateTime::duration operator-(const DateTime& dt, const sptk::DateTime& dt2)
{
    return dt.timePoint() - dt2.timePoint();
}


} // namespace sptk

//----------------------------------------------------------------
// Format routine
//----------------------------------------------------------------
void DateTime::formatDate(ostream& str, int printFlags) const
{
    if (zero())
    {
        return;
    }

    time_t t = clock::to_time_t(m_dateTime);

    if ((printFlags & PF_GMT) == 0)
    {
        t += DateTime::timeZoneOffset().count() * secondsInMinute;
    }

    tm tt {};
    gmtime_r(&t, &tt);

    array<char, maxDateTimeStringLength> buffer {};
    size_t len = 0;
    if ((printFlags & PF_RFC_DATE) != 0)
    {
        len = strftime(buffer.data(), sizeof(buffer) - 1, "%F", &tt);
    }
    else
    {
        len = strftime(buffer.data(), sizeof(buffer) - 1, "%x", &tt);
    }
    str << string(buffer.data(), len);
}

void DateTime::formatTime(ostream& str, int printFlags, PrintAccuracy printAccuracy) const
{
    short h = 0;
    short m = 0;
    short s = 0;
    short ms = 0;

    sptk::decodeTime(m_dateTime, h, m, s, ms, (printFlags & PF_GMT) != 0);
    const char* appendix = nullptr;
    bool ampm = (printFlags & PF_12HOURS) != 0;
    if ((printFlags & PF_TIMEZONE) != 0)
    {
        ampm = false;
    }
    if (ampm)
    {
        if (h > 11)
        {
            appendix = "PM";
        }
        else
        {
            appendix = "AM";
        }
        if (h > 12)
        {
            h = short(h % 12);
        }
    }

    char savedFill = str.fill('0');
    str << setw(2) << h << _timeSeparator << setw(2) << m;
    switch (printAccuracy)
    {
        case PrintAccuracy::MINUTES:
            break;
        case PrintAccuracy::SECONDS:
            str << _timeSeparator << setw(2) << s;
            break;
        default:
            str << _timeSeparator << setw(2) << s << "." << setw(3) << ms;
            break;
    }

    if (ampm)
    {
        str << appendix;
    }

    if ((printFlags & PF_TIMEZONE) != 0)
    {
        if (_timeZoneOffset.count() == 0 || (printFlags & PF_GMT) != 0)
        {
            str << "Z";
        }
        else
        {
            minutes offsetMinutes {};
            if (_timeZoneOffset.count() > 0)
            {
                str << '+';
                offsetMinutes = _timeZoneOffset;
            }
            else
            {
                str << '-';
                offsetMinutes = -_timeZoneOffset;
            }
            str << setw(2) << offsetMinutes.count() / secondsInMinute << ":" << setw(2) << offsetMinutes.count() % secondsInMinute;
        }
    }

    str.fill(savedFill);
}


void DateTime::decodeDate(short* year, short* month, short* day, short* wday, short* yday, bool gmt) const
{
    sptk::decodeDate(m_dateTime, *year, *month, *day, *wday, *yday, gmt);
}

void DateTime::decodeTime(short* h, short* m, short* s, short* ms, bool gmt) const
{
    sptk::decodeTime(m_dateTime, *h, *m, *s, *ms, gmt);
}


// Get the current system time with optional synchronization offset
DateTime DateTime::Now()
{
    return DateTime(clock::now());
}

short DateTime::daysInMonth() const
{
    short y = 0;
    short m = 0;
    short d = 0;
    short wd = 0;
    short yd = 0;
    sptk::decodeDate(m_dateTime, y, m, d, wd, yd, false);
    return isLeapYear(y) ? gLeapYear[m - 1] : gRegularYear[m - 1];
}

DateTime DateTime::date() const
{
    constexpr int hoursInDay = 24;
    duration sinceEpoch = m_dateTime.time_since_epoch();
    long days = duration_cast<hours>(sinceEpoch + seconds(TimeZone::offset())).count() / hoursInDay;
    time_point tp = time_point() + hours(days * hoursInDay);
    DateTime dt(tp); // Sets the current date
    return dt;
}

short DateTime::dayOfWeek() const
{
    short y = 0;
    short m = 0;
    short d = 0;
    short wd = 0;
    short yd = 0;

    sptk::decodeDate(m_dateTime, y, m, d, wd, yd, false);

    return short(wd);
}

String DateTime::dayOfWeekName() const
{
    return DateTime::_weekDayNames[size_t(dayOfWeek())];
}

String DateTime::monthName() const
{
    short y = 0;
    short m = 0;
    short d = 0;
    short wd = 0;
    short yd = 0;

    sptk::decodeDate(m_dateTime, y, m, d, wd, yd, false);

    return DateTime::_monthNames[size_t(m) - 1];
}

String DateTime::dateString(int printFlags) const
{
    stringstream str;
    formatDate(str, printFlags);
    return str.str();
}

String DateTime::timeString(int printFlags, PrintAccuracy printAccuracy) const
{
    stringstream str;
    formatTime(str, printFlags, printAccuracy);
    return str.str();
}

String DateTime::isoDateTimeString(PrintAccuracy printAccuracy, bool gmt) const
{
    int printFlags = PF_TIMEZONE | PF_RFC_DATE;
    if (gmt)
    {
        printFlags |= PF_GMT;
    }

    return dateString(printFlags) + "T" + timeString(printFlags, printAccuracy);
}

DateTime DateTime::convertCTime(const time_t tt)
{
    return DateTime(clock::from_time_t(tt));
}

String DateTime::format(Format dtFormat, size_t arg)
{
    switch (dtFormat)
    {
        case Format::DATE_PARTS_ORDER:
            return _datePartsOrder;
        case Format::FULL_TIME_FORMAT:
            return _fullTimeFormat;
        case Format::SHORT_TIME_FORMAT:
            return _shortTimeFormat;
        case Format::MONTH_NAME:
            return _monthNames[arg];
        case Format::WEEKDAY_NAME:
            return _weekDayNames[arg];
        default:
            return _dateFormat;
    }
}

char DateTime::dateSeparator()
{
    return _dateSeparator;
}

char DateTime::timeSeparator()
{
    return _timeSeparator;
}

String TimeZone::name()
{
    return DateTime::timeZoneName();
}

minutes TimeZone::offset()
{
    return DateTime::timeZoneOffset();
}

int TimeZone::isDaylightSavingsTime()
{
    return DateTime::isDaylightSavingsTime();
}

bool DateTime::time24Mode()
{
    return _time24Mode;
}

minutes DateTime::timeZoneOffset()
{
    return _timeZoneOffset;
}

/**
 * Returns timezone name
 * @return timezone name
 */
String DateTime::timeZoneName()
{
    return _timeZoneName;
}

/**
 * Returns true if daylight savings time
 * @return true if daylight savings time
 */
bool DateTime::isDaylightSavingsTime()
{
    return _isDaylightSavingsTime > 0;
}

double sptk::duration2seconds(const DateTime::duration& duration)
{
    auto ms = (double) chrono::duration_cast<microseconds>(duration).count() / 1000.0;
    return ms / millisecondsInSecond;
}
