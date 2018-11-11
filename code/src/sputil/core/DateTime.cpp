/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DateTime.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <ctime>
#include <cmath>
#include <cstring>
#include <sptk5/DateTime.h>
#include <sptk5/Exception.h>
#include <iomanip>

using namespace std;
using namespace std::chrono;
using namespace sptk;

namespace sptk
{

class SP_EXPORT DateTimeFormat
{
public:
    DateTimeFormat() noexcept;

    void init() noexcept;

    static char parseDateOrTime(char* format, const char* dateOrTime);
};

} // namespace sptk

static const short _monthDays[2][13] =
{
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static bool _time24Mode;
static DateTime::duration dateTimeOffset;

char     DateTime::dateFormat[32];
char     DateTime::datePartsOrder[4];
char     DateTime::fullTimeFormat[32];
char     DateTime::shortTimeFormat[32];
char     DateTime::dateSeparator;
char     DateTime::timeSeparator;
String   DateTime::weekDayNames[7];
String   DateTime::monthNames[12];
String   DateTime::timeZoneName;
int      DateTime::timeZoneOffset;
int      DateTime::isDaylightSavingsTime;

static void upperCase(char* dest, const char* src)
{
    int i = 0;
    auto len = (int) strlen(src);
    for (; i < len; i++)
        dest[i] = (char) toupper(src[i]);
    dest[i] = 0;
}

char DateTimeFormat::parseDateOrTime(char* format, const char* dateOrTime)
{
    char separator[] = " ";
    char dt[32];

    strncpy(dt, dateOrTime, sizeof(dt));
	dt[31] = 0;

    // Cut-off trailing non-digit characters
    auto len = (int) strlen(dt);
    for (int index = len - 1; index >= 0; index--) {
        if (isdigit(dt[index]) != 0) {
            dt[index + 1] = 0;
            break;
        }
    }
    char* ptr = dt;
    // find a separator char
    while (isalnum(*ptr) != 0 || *ptr == ' ')
        ptr++;
    separator[0] = *ptr;
    ptr = strtok(dt, separator);
    strcpy(format, "");

    //bool processingTime = false;
    const char* pattern;
    while (ptr != nullptr) {
        int number = string2int(ptr);
        switch (number) {
            case 10:
                pattern = "19";   // hour (12-hour mode)
                _time24Mode = false;
                break;
            case 22:
                pattern = "29";    // hour (24-hour mode)
                _time24Mode = true;
                break;
            case 48:
                pattern = "59";    // minute
                break;
            case 59:
                pattern = "59";    // second
                break;
            case 17:
                pattern = "39";   // day
                strncat(DateTime::datePartsOrder, "D", sizeof(DateTime::datePartsOrder) - 1);
                break;
            case 6:
                pattern = "19";   // month
                strncat(DateTime::datePartsOrder, "M", sizeof(DateTime::datePartsOrder) - 1);
                break;
            case 2000:
            case 0:
                pattern = "2999"; // year
                strncat(DateTime::datePartsOrder, "Y", sizeof(DateTime::datePartsOrder) - 1);
                break;
            default:
                pattern = nullptr;
                break;
        }
        if (pattern != nullptr) {
            strcat(format, pattern);
            strcat(format, separator);
        }
        ptr = strtok(nullptr, separator);
    }
    len = (int) strlen(format);
    if (len != 0)
        format[len - 1] = 0;
    //if (processingTime && _time24Mode)
    //    strcat(format, "TM");

    return separator[0];
}

DateTimeFormat::DateTimeFormat() noexcept
{
    init();
}

void DateTimeFormat::init() noexcept
{
    char dateBuffer[32];
    char timeBuffer[32];
    // make a special date and time - today :)
    struct tm t = {};
    t.tm_year = 100;    // since 1900, -> 2000
    t.tm_mon = 5;      // June (January=0)
    t.tm_mday = 17;
    t.tm_hour = 22;
    t.tm_min = 48;
    t.tm_sec = 59;
    t.tm_wday = 0;      // Sunday

#ifdef __linux__
    // For unknown reason this call of setlocale() under Windows makes
    // calls of sprintf to produce access violations. If you know why please
    // tell me..
    setlocale(LC_TIME, "");
#endif

    // Build local data and time
    strftime(timeBuffer, 32, "%X", &t);
    strftime(dateBuffer, 32, "%x", &t);

    // Build local date and time formats
    DateTime::datePartsOrder[0] = 0;
    DateTime::dateSeparator = parseDateOrTime(DateTime::dateFormat, dateBuffer);
    DateTime::time24Mode(timeBuffer[0] == '2');

    // Filling up the week day names, as defined in locale.
    // This date should be Monday:
    t.tm_year = 103;    // since 1900, -> 2003
    t.tm_mon = 9;
    t.tm_mday = 21;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    for (int wday = 0; wday < 7; wday++) {
        t.tm_wday = wday;
        strftime(dateBuffer, 32, "%A", &t);
        DateTime::weekDayNames[wday] = string(dateBuffer);
    }

    // Filling up the month names, as defined in locale.
    // This date should be January 1st:
    t.tm_year = 103;    // since 1900, -> 2003
    t.tm_mon = 1;
    t.tm_mday = 1;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    t.tm_wday = 3;
    for (int month = 0; month < 12; month++) {
        t.tm_mon = month;
        strftime(dateBuffer, 32, "%B", &t);
        DateTime::monthNames[month] = string(dateBuffer);
    }
    ::tzset();
#if defined(__BORLANDC__) || _MSC_VER > 1800
    const char *ptr = _tzname[0];
#else
    const char* ptr = tzname[0];
#endif
    auto len = (int) strlen(ptr);
    const char* p1 = strchr(ptr, ' ');
    if (p1 != nullptr)
        len = int(p1 - ptr);

    DateTime::timeZoneName = string(ptr, (unsigned) len);

    time_t ts = time(nullptr);
    char buf[16];
    struct tm *ltime = localtime(&ts);
    strftime(buf, sizeof(buf), "%z", ltime);
    int offset = string2int(buf);
    int minutes = offset % 100;
    int hours = offset / 100;
    DateTime::isDaylightSavingsTime = ltime->tm_isdst == -1? 0 : ltime->tm_isdst;
    DateTime::timeZoneOffset = hours * 60 + minutes;
}

static DateTimeFormat dateTimeFormatInitializer;

void sptk::DateTime::setTimeZone(const String& tzname)
{
#ifdef _WIN32
	_putenv_s("TZ", tzname.c_str());
#else
	setenv("TZ", tzname.c_str(), 1);
#endif
    ::tzset();
    dateTimeFormatInitializer.init();
}

void DateTime::time24Mode(bool t24mode)
{
    const char* timeBuffer = "10:48:59AM";

    if (t24mode)
        timeBuffer = "22:48:59";

    _time24Mode = t24mode;
    DateTime::timeSeparator = DateTimeFormat::parseDateOrTime(DateTime::fullTimeFormat, timeBuffer);
    strncpy(DateTime::shortTimeFormat, DateTime::fullTimeFormat, sizeof(DateTime::shortTimeFormat) - 1);
    char* p = strchr(DateTime::shortTimeFormat, DateTime::timeSeparator);
    if (p != nullptr) {
        p = strchr(p + 1, DateTime::timeSeparator);
        if (p != nullptr)
            *p = 0;
    }
    if (!_time24Mode) {
        strncat(DateTime::fullTimeFormat, "TM", sizeof(DateTime::fullTimeFormat) - 1);
        strncat(DateTime::shortTimeFormat, "TM", sizeof(DateTime::shortTimeFormat) - 1);
    }
}

void DateTime::encodeDate(time_point& dt, short year, short month, short day)
{
    if (year == 0 && month == 0 && day == 0) {
        time_point tp;
        dt = tp;
        return;
    }

    tm time = {};
    time.tm_year = year - 1900;
    time.tm_mon = month - 1;
    time.tm_mday = day;
    time.tm_isdst = isDaylightSavingsTime;

    time_t t = mktime(&time);
    dt = clock::from_time_t(t);
}

void DateTime::encodeDate(time_point& dt, const char* dat)
{
    char bdat[64];
    short datePart[7];

    memset(datePart, 0, sizeof(datePart));
    ::upperCase(bdat, dat);

    if (strcmp(bdat, "TODAY") == 0) {
        dt = Date().timePoint();
        return;
    }

    char actualDateSeparator = 0;
    short partNumber = 0;
    char* ptr = nullptr;
    auto len = (uint32_t) strlen(bdat);
    for (uint32_t i = 0; i <= len && partNumber < 7; i++) {
        char c = bdat[i];
        if (actualDateSeparator == char(0) && c == '-') {
            actualDateSeparator = c;
            c = dateSeparator;
        }
        if (c == dateSeparator || c == timeSeparator || c == ' ' || c == '-' || c == 0) {
            if (actualDateSeparator == char(0) && c == dateSeparator)
                actualDateSeparator = dateSeparator;
            if (c == timeSeparator && partNumber < 3)
                partNumber = 3;
            if (ptr != nullptr) { // end of token
                bdat[i] = 0;
                datePart[partNumber] = (short) string2int(ptr);
                partNumber++;
                ptr = nullptr;
            }
        } else {
            if (isdigit(c) == 0) {
                dt = time_point();
                return;
            }
            if (ptr == nullptr)
                ptr = bdat + i;
        }
    }
    if (partNumber < 3) { // Not enough date parts
        dt = time_point();
        return;
    }
    short month = 0, day = 0, year = 0;
    if (actualDateSeparator != dateSeparator && datePart[0] > 31) {
        // YYYY-MM-DD format
        year = datePart[0];
        month = datePart[1];
        day = datePart[2];
    } else {
        for (int ii = 0; ii < 3; ii++) {
            switch (datePartsOrder[ii]) {
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

    if (year < 100) {
        if (year < 35)
            year = short(year + 2000);
        else
            year = short(year + 1900);
    }
    time_point dd;
    encodeDate(dd, year, month, day);
    if (partNumber > 3) // Time part included into string
        encodeTime(dd, datePart[3], datePart[4], datePart[5], datePart[6]);
    dt = dd;
}

void DateTime::encodeTime(time_point& dt, short h, short m, short s, short ms)
{
    dt += hours(h) + minutes(m) + seconds(s) + milliseconds(ms);
}

static int trimRight(char* s)
{
    auto len = (int) strlen(s);

    while (len > 0) {
        len--;
        if ((unsigned char) s[len] > 32) {
            len++;
            s[len] = 0;
            break;
        }
    }
    return len;
}

// Returns timezone offset in minutes from formats:
// "Z" - UTC
// "[+-]HH24:MM - TZ offset
int decodeTZOffset(const char* tzOffset)
{
    char tzo[10];
    strncpy(tzo, tzOffset, sizeof(tzo) - 1);
    tzo[9] = 0;

    char* p = tzo;
    int sign = 1;
    switch (*p) {
        case 'Z':
            return 0;
        case '+':
            p++;
            break;
        case '-':
            p++;
            sign = -1;
            break;
        default:
            break;
    }
    char* p1 = strchr(p, ':');
    int hours, minutes = 0;
    if (p1 != nullptr) {
        *p1 = 0;
        minutes = string2int(p1 + 1);
    }
    hours = string2int(p);
    return sign * (hours * 60 + minutes);
}

void DateTime::encodeTime(time_point& dt, const char* tim)
{
    char bdat[32];

    ::upperCase(bdat, tim);

    if (trimRight(bdat) == 0) {
        dt = time_point();
        return;
    }

    if (strcmp(bdat, "TIME") == 0) {
        dt = Time().m_dateTime;        // Sets the current date
        return;
    }

    char* ptr = nullptr;
    bool afternoon = false;
    short timePart[4] = {0, 0, 0, 0};
    short partNumber = 0;
    int tzOffsetMin = 0;
    char* p = strpbrk(bdat, "APZ+-"); // Looking for AM, PM, or timezone
    if (p != nullptr) {
        char* p1;
        switch (*p) {
            case 'P':
                afternoon = true;
                break;
            case 'A':
                p1 = strpbrk(bdat, "Z+-");
                if (p1 != nullptr)
                    tzOffsetMin = -decodeTZOffset(p1);
                break;
            default:
                tzOffsetMin = -decodeTZOffset(p);
                break;
        }
        *p = 0;
        tzOffsetMin += DateTime::timeZoneOffset;
    }
    trimRight(bdat);
    auto len = (uint32_t) strlen(bdat);
    for (uint32_t i = 0; i <= len && partNumber < 4; i++) {
        char c = bdat[i];
        if (c == timeSeparator || c == ' ' || c == '.' || c == 0) {
            if (ptr != nullptr) { // end of token
                bdat[i] = 0;
                timePart[partNumber] = (short) string2int(ptr);

                partNumber++;
                ptr = nullptr;
            }
        } else {
            if (isdigit(c) == 0) {
                dt = time_point();
                return;
            }
            if (ptr == nullptr)
                ptr = bdat + i;
        }
    }
    if (afternoon && timePart[0] != 12)
        timePart[0] = short(timePart[0] + 12);
    encodeTime(dt, timePart[0], timePart[1], timePart[2], timePart[3]);
    if (tzOffsetMin != 0)
        dt += minutes(tzOffsetMin);
}

void DateTime::decodeTime(const time_point& dt, short &h, short &m, short &s, short &ms, bool gmt)
{
    time_t tt = clock::to_time_t(dt);

    tm time = {};

#ifdef _WIN32
	if (gmt)
		gmtime_s(&time, &tt);
	else
		localtime_s(&time, &tt);
#else
	if (gmt) {
        gmtime_r(&tt, &time);
    } else {
        localtime_r(&tt, &time);
    }
#endif

    h = (short) time.tm_hour;
    m = (short) time.tm_min;
    s = (short) time.tm_sec;

    duration dur = dt.time_since_epoch();
    seconds sec = duration_cast<seconds>(dt.time_since_epoch());
    milliseconds msec = duration_cast<milliseconds>(dur - sec);
    ms = (short) msec.count();
}

void DateTime::decodeDate(const time_point &dt, short &year, short &month, short &day, short &dayOfWeek, short &dayOfYear,
                          bool gmt)
{
    time_t tt = clock::to_time_t(dt);

    tm time = {};
#ifdef _WIN32
	if (gmt)
		gmtime_s(&time, &tt);
	else
		localtime_s(&time, &tt);
#else
	if (gmt)
		gmtime_r(&tt, &time);
	else
		localtime_r(&tt, &time);
#endif

    year = (short) (time.tm_year + 1900);
    month = (short) (time.tm_mon + 1);
    day = (short) time.tm_mday;
    dayOfWeek = (short) time.tm_wday;
    dayOfYear = (short) time.tm_yday;
}

//----------------------------------------------------------------
// Constructors
//----------------------------------------------------------------
DateTime::DateTime(short year, short month, short day, short hour, short minute, short second, short millisecond) noexcept
{
    try {
        encodeDate(m_dateTime, year, month, day);
        encodeTime(m_dateTime, hour, minute, second, millisecond);
    }
    catch (...) {
        m_dateTime = time_point();
    }
}

DateTime::DateTime(const char* dat) noexcept
{
    while (*dat == ' ') dat++;
    if (*dat == char(0)) {
        m_dateTime = time_point();
        return;
    }

    char* s1 = strdup(dat);
    char* s2 = strpbrk(s1, " T");
    if (s2 != nullptr) {
        *s2 = 0;
        s2++;
    }

    try {
        if (strchr(s1, dateSeparator) != nullptr || strchr(s1, '-') != nullptr) {
            encodeDate(m_dateTime, s1);
            if (s2 != nullptr && strchr(s2, timeSeparator) != nullptr)
                encodeTime(m_dateTime, s2);
        } else
            encodeTime(m_dateTime, s1);
        free(s1);
    } catch (...) {
        free(s1);
        m_dateTime = time_point();
    }
}

DateTime::DateTime(const time_point& dt) noexcept
: m_dateTime(dt)
{
}

DateTime::DateTime(const duration& dt) noexcept
: m_dateTime(dt)
{
}

DateTime::DateTime(int64_t sinceEpochMS) noexcept
: m_dateTime(milliseconds(sinceEpochMS))
{
}

//----------------------------------------------------------------
// Assignments
//----------------------------------------------------------------

DateTime& DateTime::operator=(const char* dat)
{
    if (dat != nullptr)
        encodeDate(m_dateTime, dat);
    else
        m_dateTime = time_point();
    return *this;
}

//----------------------------------------------------------------
// Date Arithmetic
//----------------------------------------------------------------
DateTime DateTime::operator+(duration& dt)
{
    return DateTime(m_dateTime + dt);
}

DateTime DateTime::operator-(duration& dt)
{
    return DateTime(m_dateTime - dt);
}

DateTime& DateTime::operator+=(duration& dt)
{
    m_dateTime += dt;
    return *this;
}

DateTime& DateTime::operator-=(duration& dt)
{
    m_dateTime -= dt;
    return *this;
}

DateTime& DateTime::operator++()
{
    m_dateTime += hours(24);
    return *this;
}

const DateTime DateTime::operator++(int)
{
    m_dateTime += hours(24);
    return *this;
}

DateTime& DateTime::operator--()
{
    m_dateTime -= hours(24);
    return *this;
}

const DateTime DateTime::operator--(int)
{
    m_dateTime -= hours(24);
    return *this;
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

DateTime operator+(const DateTime &dt, const sptk::DateTime::duration &duration)
{
    return DateTime(dt.timePoint() + duration);
}

DateTime operator-(const DateTime &dt, const sptk::DateTime::duration &duration)
{
    return DateTime(dt.timePoint() - duration);
}

DateTime::duration operator-(const DateTime &dt, const sptk::DateTime &dt2)
{
    return dt.timePoint() - dt2.timePoint();
}


}

#if _WIN32
#define gmtime_r(a,b)		gmtime_s(b,a)
#define localtime_r(a,b)	localtime_s(b,a)
#endif

//----------------------------------------------------------------
// Format routine
//----------------------------------------------------------------
void DateTime::formatDate(ostream& str, int printFlags) const
{
    if (zero())
        return;

    time_t t = clock::to_time_t(m_dateTime);
    tm     tt {};
    if ((printFlags & PF_GMT) != 0)
        gmtime_r(&t, &tt);
    else
        localtime_r(&t, &tt);

    char buffer[128];
    size_t len;
    if ((printFlags & PF_RFC_DATE) != 0)
        len = strftime(buffer, sizeof(buffer) - 1, "%F", &tt);
    else
        len = strftime(buffer, sizeof(buffer) - 1, "%x", &tt);
    str << string(buffer, len);
}

void DateTime::formatTime(ostream& str, int printFlags, PrintAccuracy printAccuracy) const
{
    short h = 0, m = 0, s = 0, ms = 0;

    decodeTime(m_dateTime, h, m, s, ms, (printFlags & PF_GMT) != 0);
    const char* appendix = nullptr;
    bool ampm = (printFlags & PF_12HOURS) != 0;
    if ((printFlags & PF_TIMEZONE) != 0)
        ampm = false;
    if (ampm) {
        if (h > 11)
            appendix = "PM";
        else
            appendix = "AM";
        if (h > 12)
            h = short(h % 12);
    }

    char savedFill = str.fill('0');
    str << setw(2) << h << timeSeparator << setw(2) << m;
    switch (printAccuracy) {
        case PA_MINUTES:
            break;
        case PA_SECONDS:
            str << timeSeparator << setw(2) << s;
            break;
        case PA_MILLISECONDS:
            str << timeSeparator << setw(2) << s << "." << setw(3) << ms;
            break;
    }

    if (ampm)
        str << appendix;

    if ((printFlags & PF_TIMEZONE) != 0) {
        if (timeZoneOffset == 0 || (printFlags & PF_GMT) != 0)
            str << "Z";
        else {
            int minutes;
            if (timeZoneOffset > 0) {
                str << '+';
                minutes = timeZoneOffset;
            } else {
                str << '-';
                minutes = -timeZoneOffset;
            }
            str << setw(2) << minutes / 60 << ":" << setw(2) << minutes % 60;
        }
    }

    str.fill(savedFill);
}

//----------------------------------------------------------------
//  Miscellaneous Routines
//----------------------------------------------------------------
short DateTime::dayOfYear() const
{
    short y = 0, m = 0, d = 0, wd = 0, yd = 0;

    decodeDate(m_dateTime, y, m, d, wd, yd, false);

    return d;
}

// Get the current system time
DateTime DateTime::System()
{
    return DateTime(clock::now());
}

// Get the current system time with optional synchronization offset
DateTime DateTime::Now()
{
    return DateTime(clock::now() + ::dateTimeOffset);
}

// Set the synchronization offset
void DateTime::Now(DateTime dt)
{
    ::dateTimeOffset = dt.m_dateTime - DateTime::System().m_dateTime;
}

DateTime DateTime::Date()
{
    return Now().date();
}

DateTime DateTime::Time()
{
    duration sinceEpoch = clock::now().time_since_epoch();
    duration thisDay = sinceEpoch - hours(duration_cast<hours>(sinceEpoch).count() / 24 * 24);
    return DateTime(time_point() + thisDay);
}

short DateTime::daysInMonth() const
{
    short y = 0, m = 0, d = 0, wd = 0, yd = 0;
    decodeDate(m_dateTime, y, m, d, wd, yd, false);
    return _monthDays[isLeapYear(y)][m - 1];
}

DateTime DateTime::date() const
{
    duration sinceEpoch = m_dateTime.time_since_epoch();
    long days = duration_cast<hours>(sinceEpoch).count() / 24;
    DateTime dt(time_point() + hours(days * 24));  // Sets the current date
    return dt;
}

short DateTime::day() const
{
    short y = 0, m = 0, d = 0, wd = 0, yd = 0;
    decodeDate(m_dateTime, y, m, d, wd, yd, false);
    return d;
}

short DateTime::month() const
{
    short y = 0, m = 0, d = 0, wd = 0, yd = 0;
    decodeDate(m_dateTime, y, m, d, wd, yd, false);
    return m;
}

short DateTime::year() const
{
    short y = 0, m = 0, d = 0, wd = 0, yd = 0;
    decodeDate(m_dateTime, y, m, d, wd, yd, false);
    return y;
}

short DateTime::dayOfWeek() const
{
    short y = 0, m = 0, d = 0, wd = 0, yd = 0;
    decodeDate(m_dateTime, y, m, d, wd, yd, false);
    return short(wd + 1);
}

String DateTime::dayOfWeekName() const
{
    return DateTime::weekDayNames[dayOfWeek() - 1];
}

String DateTime::monthName() const
{
    return DateTime::monthNames[month() - 1];
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
        printFlags |= PF_GMT;

    return dateString(printFlags) + "T" + timeString(printFlags, printAccuracy);
}

DateTime DateTime::convertCTime(const time_t tt)
{
    return DateTime(clock::from_time_t(tt));
}

bool DateTime::time24Mode()
{
    return _time24Mode;
}

double sptk::duration2seconds(const DateTime::duration& duration)
{
    return chrono::duration_cast<milliseconds>(duration).count() / 1000.0;
}

#if USE_GTEST
#include <gtest/gtest.h>

TEST(SPTK_DateTime, ctor1)
{
    DateTime dateTime("2018-01-01 11:22:33.444+10");
    chrono::milliseconds msSinceEpoch = duration_cast<chrono::milliseconds>(dateTime.sinceEpoch());
    EXPECT_EQ(1514769753444, msSinceEpoch.count());
}

TEST(SPTK_DateTime, ctor2)
{
    DateTime dateTime1("2018-01-01 11:22:33");
    DateTime dateTime2(2018, 1, 1, 11, 22, 33);
    chrono::milliseconds msSinceEpoch1 = duration_cast<chrono::milliseconds>(dateTime1.sinceEpoch());
    chrono::milliseconds msSinceEpoch2 = duration_cast<chrono::milliseconds>(dateTime2.sinceEpoch());
    EXPECT_EQ(msSinceEpoch1.count(), msSinceEpoch2.count());
}

TEST(SPTK_DateTime, timeZones)
{
    DateTime dateTime1("2018-01-01 11:22:33.444+10");
    DateTime dateTime2("2018-01-01 10:22:33.444+09");
    chrono::milliseconds msSinceEpoch1 = duration_cast<chrono::milliseconds>(dateTime1.sinceEpoch());
    chrono::milliseconds msSinceEpoch2 = duration_cast<chrono::milliseconds>(dateTime2.sinceEpoch());
    EXPECT_EQ(1514769753444, msSinceEpoch1.count());
    EXPECT_EQ(1514769753444, msSinceEpoch2.count());
}

TEST(SPTK_DateTime, add)
{
    DateTime dateTime1("2018-01-01 11:22:33.444+10");
    DateTime dateTime2 = dateTime1 + chrono::milliseconds(111);
    chrono::milliseconds msSinceEpoch2 = duration_cast<chrono::milliseconds>(dateTime2.sinceEpoch());
    EXPECT_EQ(1514769753555, msSinceEpoch2.count());
}

TEST(SPTK_DateTime, extractDate)
{
    short year, month, day, wday, yday;
    DateTime dateTime("2018-08-07 11:22:33.444Z");
    dateTime.decodeDate(&year, &month, &day, &wday, &yday, true);
    EXPECT_EQ(2018, year);
    EXPECT_EQ(8, month);
    EXPECT_EQ(7, day);
    EXPECT_EQ(2, wday);
    EXPECT_EQ(218, yday);
}

TEST(SPTK_DateTime, extractTime)
{
    short hour, minute, second, ms;
    DateTime dateTime("2018-08-07 11:22:33.444Z");
    dateTime.decodeTime(&hour, &minute, &second, &ms, true);
    EXPECT_EQ(11, hour);
    EXPECT_EQ(22, minute);
    EXPECT_EQ(33, second);
    EXPECT_EQ(444, ms);
}

TEST(SPTK_DateTime, formatDate)
{
    DateTime dateTime("2018-08-07 11:22:33.444Z");

    time_t t = (time_t) dateTime;
    tm     tt {};
    localtime_r(&t, &tt);

    char buffer[128];
    strftime(buffer, sizeof(buffer) - 1, "%x", &tt);

    EXPECT_STREQ("2018-08-07", dateTime.dateString(DateTime::PF_GMT|DateTime::PF_RFC_DATE).c_str());
    EXPECT_STREQ(buffer, dateTime.dateString(DateTime::PF_GMT).c_str());
}

TEST(SPTK_DateTime, formatTime)
{
    DateTime dateTime("2018-08-07 11:22:33.444Z");

    auto   t = (time_t) dateTime;
    tm     tt {};
    gmtime_r(&t, &tt);

    char buffer[128];
    strftime(buffer, sizeof(buffer) - 1, "%X", &tt);

    EXPECT_STREQ("11:22:33.444Z", dateTime.timeString(DateTime::PF_GMT|DateTime::PF_TIMEZONE, DateTime::PA_MILLISECONDS).c_str());
    EXPECT_STREQ("11:22:33", dateTime.timeString(DateTime::PF_GMT).c_str());
}

#endif
