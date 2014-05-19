/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDateTime.cpp  -  description
                             -------------------
    begin                : Tue Dec 14 1999
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#ifndef _WIN32
#ifdef __BORLANDC__
#include <time.h>
#else
#include <sys/time.h>
#endif
#endif

#include <time.h>
#include <ctype.h>

#include <sptk5/CDateTime.h>
#include <sptk5/CException.h>
#include <sptk5/CStrings.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

using namespace std;
using namespace sptk;

namespace sptk {

    class SP_EXPORT CDateTimeFormat
    {
        public:
            CDateTimeFormat();
            string dateFormat();
            string timeFormat();
            void    weekDayNames(string wdn[7]);
            static char parseDateOrTime(char *format,const char *dt);
    };

}

static const short _monthDays[2][13] = 
{
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static const short _monthDaySums[2][13] =
{
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

#define DateDelta 693594

static bool     _time24Mode;
static double   dateTimeOffset;

char     CDateTime::dateFormat[32];
char     CDateTime::datePartsOrder[4];
char     CDateTime::fullTimeFormat[32];
char     CDateTime::shortTimeFormat[32];
char     CDateTime::dateSeparator;
char     CDateTime::timeSeparator;
string   CDateTime::weekDayNames[7];
string   CDateTime::monthNames[12];
string   CDateTime::timeZoneName;
int      CDateTime::timeZoneOffset;

static void upperCase(char *dest,const char *src)
{
    int i = 0;
    int len = (int) strlen(src);
    for (; i < len; i++)
        dest[i] = (char) toupper(src[i]);
    dest[i] = 0;
}

char CDateTimeFormat::parseDateOrTime(char *format,const char *dateOrTime)
{
    char  separator[] = " ";
    char  dt[32];

    strcpy(dt,dateOrTime);

    // Cut-off trailing non-digit characters
    int len = (int) strlen(dt);
    for (int index = len - 1; index >= 0; index--) {
        if (isdigit(dt[index])) {
            dt[index+1] = 0;
            break;
        }
    }
    char *ptr = dt;
    // find a separator char
    ptr = dt;
    while (isalnum(*ptr) || *ptr == ' ')
        ptr++;
    separator[0] = *ptr;
    ptr = strtok(dt,separator);
    strcpy(format,"");

    bool processingTime = false;
    const char *pattern;
    while (ptr) {
        int number = atoi(ptr);
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
            strcat(CDateTime::datePartsOrder,"D");
            break;
        case 6:
            pattern = "19";   // month
            strcat(CDateTime::datePartsOrder,"M");
            break;
        case 2000:
        case 0:
            pattern = "2999"; // year
            strcat(CDateTime::datePartsOrder,"Y");
            break;
        default:
            pattern = NULL;
            break;
        }
        if (pattern) {
            strcat(format,pattern);
            strcat(format,separator);
        }
        ptr = strtok(NULL,separator);
    }
    len = (int) strlen(format);
    if (len)
        format[len-1] = 0;
    if (processingTime && _time24Mode)
        strcat(format,"TM");

    return separator[0];
}

CDateTimeFormat::CDateTimeFormat()
{
    char  dateBuffer[32];
    char  timeBuffer[32];
    // make a special date and time - today :)
    struct tm t;
    t.tm_year = 100;    // since 1900, -> 2000
    t.tm_mon  = 5;      // June (January=0)
    t.tm_mday = 17;
    t.tm_hour = 22;
    t.tm_min  = 48;
    t.tm_sec  = 59;
    t.tm_wday = 0;      // Sunday

#ifndef _WIN32
    // For unknown reason this call of setlocale() under Windows makes
    // calls of sprintf to produce access violations. If you know why please
    // tell me..
    setlocale(LC_TIME,"");
#endif

    // Build local data and time
    strftime(timeBuffer,32,"%X",&t);
    strftime(dateBuffer,32,"%x",&t);

    // Build local date and time formats
    CDateTime::datePartsOrder[0] = 0;
    CDateTime::dateSeparator = parseDateOrTime(CDateTime::dateFormat,dateBuffer);
    CDateTime::time24Mode(timeBuffer[0] == '2');

    // Filling up the week day names, as defined in locale.
    // This date should be Monday:
    t.tm_year = 103;    // since 1900, -> 2003
    t.tm_mon  = 9;
    t.tm_mday = 21;
    t.tm_hour = 0;
    t.tm_min  = 0;
    t.tm_sec  = 0;
    for (int wday = 0; wday < 7; wday++) {
        t.tm_wday = wday;
        strftime(dateBuffer,32,"%A",&t);
        CDateTime::weekDayNames[wday] = dateBuffer;
    }

    // Filling up the month names, as defined in locale.
    // This date should be January 1st:
    t.tm_year = 103;    // since 1900, -> 2003
    t.tm_mon  = 1;
    t.tm_mday = 1;
    t.tm_hour = 0;
    t.tm_min  = 0;
    t.tm_sec  = 0;
    t.tm_wday = 3;
    for (int month = 0; month < 12; month++) {
        t.tm_mon = month;
        strftime(dateBuffer,32,"%B",&t);
        CDateTime::monthNames[month] = dateBuffer;
    }
    tzset();
#ifdef __BORLANDC__
    const char *ptr = _tzname[0];
#else
    const char *ptr = tzname[0];
#endif

    char zname[4];
    int cnt = 0;
    if (strlen(ptr) < 4)
        strcpy(zname,ptr);
    else
        while (ptr && cnt < 3) {
            zname[cnt++] = *ptr;
            ptr = strchr(ptr,' ');
            if (ptr)
                ptr++;
        }
    zname[cnt] = 0;
    CDateTime::timeZoneName = zname;
#if defined(__BORLANDC__)
    CDateTime::timeZoneOffset = -_timezone / 60 + _daylight * 60;
#else
    #ifdef __FreeBSD__
        time_t at = time(NULL);
        struct tm* tt = gmtime(&at);
        CDateTime::timeZoneOffset = (int) tt->tm_gmtoff  / 60;
    #else
        CDateTime::timeZoneOffset = -(int) timezone / 60 + daylight * 60;
    #endif
#endif
}

static CDateTimeFormat dateTimeFormatInitializer;

void CDateTime::time24Mode(bool t24mode)
{
    const char *timeBuffer = "10:48:59AM";

    if (t24mode)
        timeBuffer = "22:48:59";

    _time24Mode = t24mode;
    CDateTime::timeSeparator = CDateTimeFormat::parseDateOrTime(CDateTime::fullTimeFormat,timeBuffer);
    strcpy(CDateTime::shortTimeFormat,CDateTime::fullTimeFormat);
    char *p = strchr(CDateTime::shortTimeFormat,CDateTime::timeSeparator);
    if (p) {
        p = strchr(p+1,CDateTime::timeSeparator);
        if (p)
            *p = 0;
    }
    if (!_time24Mode) {
        strcat(CDateTime::fullTimeFormat,"TM");
        strcat(CDateTime::shortTimeFormat,"TM");
    }
}

void CDateTime::encodeDate(double &dt,short year,short month,short day)
{
    if (year == 0 && month == 0 && day == 0) {
        dt = 0;
        return;
    }
    if (month < 1 || month > 12)
        throw CException("Invalid month in the date");
    int yearKind = isLeapYear(year);
    month--;
    if (day < 1 || day > _monthDays[yearKind][month])
        throw CException("Invalid day in the date");
    if (year <= 0 || year > 9999)
        throw CException("Invalid year in the date");

    day += _monthDaySums[yearKind][month];
    int i = year - 1;
    dt = i * 365 + i / 4 - i / 100 + i / 400 + day - DateDelta;
}

void CDateTime::encodeDate(double &dt,const char *dat)
{
    char     bdat[64];
    short    datePart[7], partNumber = 0;
    char     *ptr = NULL;
    char     actualDateSeparator = 0;

    memset(datePart,0,sizeof(datePart));
    ::upperCase(bdat,dat);

    if (strcmp(bdat,"TODAY") == 0) {
        dt = Date();        // Sets the current date
        return;
    } else {
        uint32_t len = (uint32_t) strlen(bdat);
        for (uint32_t i = 0; i <= len && partNumber < 7; i++) {
            char c = bdat[i];
            if (!actualDateSeparator && c == '-') {
                actualDateSeparator = c;
                c = dateSeparator;
            }
            if (c == dateSeparator || c == timeSeparator || c == ' ' || c == '-' || c == 0) {
                if (!actualDateSeparator && c == dateSeparator)
                    actualDateSeparator = dateSeparator;
                if (c == timeSeparator && partNumber < 3)
                    partNumber = 3;
                if (ptr) { // end of token
                    bdat[i] = 0;
                    datePart[partNumber] = (short)atoi(ptr);
                    partNumber++;
                    ptr = NULL;
                }
            } else {
                if (!isdigit(c)) {
                    dt = 0;
                    return;
                }
                if (!ptr)
                    ptr = bdat + i;
            }
        }
        if (partNumber < 3) { // Not enough date parts
            dt = 0;
            return;
        }
        short month=0, day=0, year=0;
        if (actualDateSeparator != dateSeparator && datePart[0] > 31) {
            // YYYY-MM-DD format
            year  = datePart[0];
            month = datePart[1];
            day   = datePart[2];
        } else {
            for (int ii = 0; ii < 3; ii++) {
                switch (datePartsOrder[ii]) {
                case 'M':
                    month = datePart[ii];
                    break;
                case 'D':
                    day   = datePart[ii];
                    break;
                case 'Y':
                    year  = datePart[ii];
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
        double dd;
        encodeDate(dd,year,month,day);
        if (partNumber > 3) { // Time part included into string
            double d;
            encodeTime(d,datePart[3],datePart[4],datePart[5],datePart[6]);
            dd += d;
        }
        dt = dd;

    }
}

void CDateTime::encodeTime(double& dt,short h,short m,short s,short ms)
{
    double seconds = s + ms / 1000.0;
    double minutes = m + seconds / 60.0;
    double hours = h + (minutes / 60.0);
    dt = hours / 24.0;
}

static int trimRight(char *s)
{
    int len = (int) strlen(s);

    while ((len--) >= 0) {
        if ((unsigned char)s[len] > 32) {
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
    strncpy(tzo,tzOffset,sizeof(tzo) - 1);
    
    char* p = tzo;
    int   sign = 1;
    switch (*p) {
        case 'Z': return 0;
        case '+': p++; break;
        case '-': p++; sign = -1; break;
    }
    char* p1 = strchr(p,':');
    int   hours, minutes = 0;
    if (p1) {
        *p1 = 0;
        minutes = atoi(p1+1);
    }
    hours = atoi(p);
    return sign * (hours * 60 + minutes);
}

void CDateTime::encodeTime(double& dt,const char *tim)
{
    char  bdat[32];
    short timePart[4] = { 0, 0, 0, 0};
    short partNumber = 0;
    char  *ptr = NULL;
    bool  afternoon = false;

    ::upperCase(bdat,tim);

    if (!trimRight(bdat)) {
        dt = 0;
        return;
    }

    if (strcmp(bdat,"TIME") == 0) {
        dt = Time();        // Sets the current date
        return;
    } else {
        int tzOffsetMin = 0;
        char *p = strpbrk(bdat,"APZ+-"); // Looking for AM, PM, or timezone
        if (p) {
            if (*p == 'P') {
                afternoon = true;
                p = strpbrk(bdat,"Z+-");
                if (p)
                    tzOffsetMin = -decodeTZOffset(p);
            } else
                tzOffsetMin = -decodeTZOffset(p);
            *p = 0;
            tzOffsetMin += CDateTime::timeZoneOffset;
        }
        trimRight(bdat);
        uint32_t len = (uint32_t) strlen(bdat);
        for (uint32_t i = 0; i <= len && partNumber < 4; i++) {
            char c = bdat[i];
            if (c == timeSeparator || c == ' ' || c == '.' || c == 0) {
                if (ptr) { // end of token
                    bdat[i] = 0;
                    timePart[partNumber] = (short) atoi(ptr);

                    partNumber++;
                    ptr = NULL;
                }
            } else {
                if (!isdigit(c)) {
                    dt = 0;
                    return;
                }
                if (!ptr)
                    ptr = bdat + i;
            }
        }
        if (afternoon && timePart[0] != 12)
            timePart[0] = short(timePart[0] + 12);
        encodeTime(dt,timePart[0],timePart[1],timePart[2],timePart[3]);
        if (tzOffsetMin)
            dt += tzOffsetMin / 1440.0;
    }
}

const int S1 = 24 * 3600; // seconds in 1 day

void CDateTime::decodeTime(const double dt,short& h,short& m,short& s,short& ms)
{
    double t = dt - (int) dt;

    if (t < 0)
        t++;

    double floatSecs = t * S1;
    double msecs = int(floatSecs * 1000 + 0.5);
    int secs = int(msecs/1000);
    ms = short((msecs / 1000 - secs) * 1000);
    h = short(secs / 3600);
    secs = secs % 3600;
    m = short(secs / 60);
    secs = secs % 60;
    s = short(secs);
}

const int D1 = 365;              // Days in 1 year
const int D4 = D1 * 4 + 1;       // Days in 4 years
const int D100 = D4 * 25 - 1;    // Days in 100 years
const int D400 = D100 * 4 + 1;   // Days in 400 years

inline void DivMod(int op1, int op2, int& div, int& mod)
{
    div = op1 / op2;
    mod = op1 % op2;
}

void CDateTime::decodeDate(const double dat,short& year,short& month,short& day)
{
    int M, D, I;
    int T = (int) dat + DateDelta - 1;

    int fourCenturiesCount = T / D400;
    T %= D400;
    int Y = fourCenturiesCount * 400 + 1;

    DivMod(T, D100, I, D);
    if (I == 4) {
        I--;
        D += D100;
    }

    Y += I * 100;
    DivMod(D, D4, I, D);
    Y += I * 4;
    DivMod(D, D1, I, D);
    if (I == 4) {
        I--;
        D += D1;
    }
    Y += I;
    year = (short) Y;
    //year  = short (Y + 1900);

    int leapYear = isLeapYear(short(year));
    M = D / 31;
    int mds = _monthDaySums[leapYear][M+1];
    if (mds <= D)
        M++;
    else
        mds = _monthDaySums[leapYear][M];

    D -= mds;

    month = short(M + 1);
    day   = short(D + 1);
}

//----------------------------------------------------------------
// Constructors
//----------------------------------------------------------------
CDateTime::CDateTime(short year,short month,short day,short hour,short minute,short second)
{
    double t;
    encodeDate(m_dateTime,year,month,day);
    encodeTime(t,hour,minute,second);
    m_dateTime += t;
}

CDateTime::CDateTime(const char * dat)
{
    while (*dat && *dat == ' ') dat++;
    if (!*dat) {
        m_dateTime = 0;
        return;
    }

    char* s1 = strdup(dat);
    char* s2 = strpbrk(s1," T");
    if (s2) {
        *s2 = 0;
        s2++;
    }
    
    try {
        if (strchr(s1,dateSeparator) || strchr(s1,'-')) {
            encodeDate(m_dateTime, s1);
            if (s2 && strchr(s2,timeSeparator)) {
                double dt;
                encodeTime(dt, s2);
                m_dateTime += dt;
            }
        } else
            encodeTime(m_dateTime, s1);
    } catch (...) {
        free(s1);
        throw;
    }
}

CDateTime::CDateTime(const CDateTime &dt)
{
    m_dateTime = dt.m_dateTime;
}

CDateTime::CDateTime(const double dt)
{
    m_dateTime = dt;
}
//----------------------------------------------------------------
// Assignments
//----------------------------------------------------------------
void CDateTime::operator = (const CDateTime &dt)
{
    m_dateTime = dt.m_dateTime;
}

void CDateTime::operator = (const char * dat)
{
    if (dat)
        encodeDate(m_dateTime, dat);
    else
        m_dateTime = 0;
}

//----------------------------------------------------------------
// Conversion operations
//----------------------------------------------------------------
// CDateTime::operator int (void) { return (int) dateTime; }

CDateTime::operator double(void) const
{
    return m_dateTime;
}

//----------------------------------------------------------------
// Date Arithmetic
//----------------------------------------------------------------
CDateTime CDateTime::operator + (CDateTime& dt)
{
    return CDateTime(m_dateTime + dt.m_dateTime);
}

CDateTime CDateTime::operator - (CDateTime& dt)
{
    return CDateTime(m_dateTime - dt.m_dateTime);
}

CDateTime& CDateTime::operator += (CDateTime& dt)
{
    m_dateTime += dt.m_dateTime;
    return *this;
}

CDateTime& CDateTime::operator -= (CDateTime& dt)
{
    m_dateTime -= dt.m_dateTime;
    return *this;
}

CDateTime& CDateTime::operator ++()
{
    m_dateTime += 1;
    return *this;
}

CDateTime &CDateTime::operator ++(int)
{
    m_dateTime += 1;
    return *this;
}

CDateTime &CDateTime::operator --()
{
    m_dateTime -= 1;
    return *this;
}

CDateTime &CDateTime::operator --(int)
{
    m_dateTime -= 1;
    return *this;
}

//----------------------------------------------------------------
// Date comparison
//----------------------------------------------------------------

bool operator < (const CDateTime &dt1, const CDateTime &dt2)
{
    return ((double) dt1 < (double) dt2);
}

bool operator <= (const CDateTime &dt1, const CDateTime &dt2)
{
    return ((double) dt1 <= (double) dt2);
}

bool operator > (const CDateTime &dt1, const CDateTime &dt2)
{
    return ((double) dt1 > (double) dt2);
}

bool operator >= (const CDateTime &dt1, const CDateTime &dt2)
{
    return ((double) dt1 >= (double) dt2);
}

bool operator == (const CDateTime &dt1, const CDateTime &dt2)
{
    return ((double) dt1 == (double) dt2);
}

bool operator != (const CDateTime &dt1, const CDateTime &dt2)
{
    return ((double) dt1 != (double) dt2);
}

CDateTime operator + (const CDateTime &dt1, const CDateTime &dt2)
{
    return double(dt1) + double(dt2);
}

CDateTime operator - (const CDateTime &dt1, const CDateTime &dt2)
{
    return double(dt1) - double(dt2);
}

//----------------------------------------------------------------
// Format routine
//----------------------------------------------------------------
void CDateTime::formatDate(char *str, bool universalDateFormat) const
{
    char *ptr = str;
    short month, day, year;

    if (m_dateTime == 0) {
        *str = 0;
        return;
    }
    decodeDate(m_dateTime,year,month,day);
    if (universalDateFormat) {
        int bytes = sprintf(str, "%04d-%02d-%02d", year, month, day);
        ptr += bytes;
        *ptr = 0;
    } else {
        for (int i = 0; i < 3; i++) {
            switch (datePartsOrder[i]) {
            case 'M':
                sprintf(ptr,"%02i%c",month,dateSeparator);
                break;
            case 'D':
                sprintf(ptr,"%02i%c",day,dateSeparator);
                break;
            case 'Y':
                sprintf(ptr,"%04i%c",year,dateSeparator);
                break;
            }
            ptr += strlen(ptr);
        }
        *(ptr-1) = 0;
    }
}

void CDateTime::formatTime(char *str,bool ampm,bool showSeconds,bool showTimezone) const {
    short h,m,s,ms;

    decodeTime(m_dateTime,h,m,s,ms);
    const char *appendix = 0L;
    if (ampm) {
        if (h > 11)
            appendix = "PM";
        else
            appendix = "AM";
        if (h > 12)
            h = h%12;
    }
    int length;
    if (!showSeconds)
        length = sprintf(str,"%02i%c%02i",h,timeSeparator,m);
    else
        length = sprintf(str,"%02i%c%02i%c%02i",h,timeSeparator,m,timeSeparator,s);
    if (ampm)
        strcat(str,appendix);
    if (showTimezone) {
        int minutes;
        if (timeZoneOffset > 0) {
            str[length] = '+';
            minutes = timeZoneOffset;
        } else {
            str[length] = '-';
            minutes = -timeZoneOffset;
        }
        sprintf(str + length + 1, "%02d:%02d", minutes / 60, minutes % 60);
    }
}
//----------------------------------------------------------------
//  Miscellaneous Routines
//----------------------------------------------------------------
short CDateTime::dayOfYear(void) const {
    CDateTime temp(1, 1, year());

    return (short)(m_dateTime - temp.m_dateTime);
}

// Get the current system time
CDateTime CDateTime::System() {
    double dat,tim;
#ifndef _WIN32
    timeval tp;
    time_t  tt;
    time(&tt);
    struct tm *t = localtime(&tt);
    encodeDate(dat,short(t->tm_year+1900),short(t->tm_mon+1),short(t->tm_mday));
    gettimeofday(&tp,0L);
    short msec = short(tp.tv_usec / 1000);
    encodeTime(tim,short(t->tm_hour),short(t->tm_min),short(t->tm_sec),short(msec));
#else
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);
    encodeDate(dat,short(systemTime.wYear),short(systemTime.wMonth),short(systemTime.wDay));
    encodeTime(tim,short(systemTime.wHour),short(systemTime.wMinute),short(systemTime.wSecond),short(systemTime.wMilliseconds));
#endif

    return dat + tim;
}

// Get the current system time with optional synchronization offset
CDateTime CDateTime::Now() {
    return (double)CDateTime::System() + ::dateTimeOffset;
}

// Set the synchronization offset
void CDateTime::Now(CDateTime dt) {
    ::dateTimeOffset = (double)dt - (double)CDateTime::System();
}

CDateTime CDateTime::Date() {
    double dat = Now();
    return double(int(dat));
}

CDateTime CDateTime::Time() {
    double dat = Now();
    return dat - int(dat);
}

uint32_t CDateTime::TimeOfDayMs() {
#ifndef _WIN32
    timeval tp;
    gettimeofday(&tp,0L);
    return uint32_t(tp.tv_sec * 1000 + tp.tv_usec / 1000);
#else
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);
    return ((systemTime.wHour * 60 + systemTime.wMinute) *60 + systemTime.wSecond) * 1000 + systemTime.wMilliseconds;
#endif
}

short CDateTime::daysInMonth() const {
    short y, m, d;
    decodeDate(m_dateTime,y,m,d);
    return _monthDays[isLeapYear(y)][m-1];
}

uint32_t CDateTime::date() const {
    return uint32_t(m_dateTime);
}

short CDateTime::day() const {
    short y, m, d;
    decodeDate(m_dateTime,y,m,d);
    return d;
}

short CDateTime::month() const {
    short y, m, d;
    decodeDate(m_dateTime,y,m,d);
    return m;
}

short CDateTime::year() const {
    short y, m, d;
    decodeDate(m_dateTime,y,m,d);
    return y;
}

short CDateTime::dayOfWeek(void) const {
    return short((int(m_dateTime) - 1) % 7 + 1);
}

string CDateTime::dayOfWeekName(void) const {
    return CDateTime::weekDayNames[dayOfWeek() - 1];
}

string CDateTime::monthName() const {
    return CDateTime::monthNames[month()-1];
}

string CDateTime::dateString(bool universalDateFormat) const {
    char  buffer[32];
    formatDate(buffer, universalDateFormat);
    return string(buffer);
}

string CDateTime::timeString(bool showSeconds, bool showTimezone) const {
    char  buffer[32];
    formatTime(buffer,!_time24Mode,showSeconds,showTimezone);
    return string(buffer);
}

CDateTime CDateTime::convertCTime(const time_t tt) {
    struct tm *t = localtime((time_t*)&tt);
    double dat,tim;
    encodeDate(dat,short(t->tm_year+1900),short(t->tm_mon+1),short(t->tm_mday));
    encodeTime(tim,short(t->tm_hour),short(t->tm_min),short(t->tm_sec),short(0));
    return dat + tim;
}

bool CDateTime::time24Mode()
{
    return _time24Mode;
}
