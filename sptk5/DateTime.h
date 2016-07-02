/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DateTime.h - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_DATETIME_H__
#define __SPTK_DATETIME_H__ 

#include <sptk5/sptk.h>
#include <time.h>
#include <string>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

class CDateTimeFormat;

/// @brief Date and Time value.
///
/// Represents the date and time value. This value is stored as
/// a floating point number. Allows to synchronize the Now() time
/// with the external date/time, without affecting the local host
/// system time.
class SP_EXPORT DateTime
{
    friend class CDateTimeFormat;
protected:

    /// @brief Internal decode date operation into year, month, and day
    /// @param dt double, Date to decode
    /// @param year int16_t&, Year (output)
    /// @param month int16_t&, Month (output)
    /// @param day int16_t&, Day (output)
    static void   decodeDate(double dt, int16_t& year, int16_t& month, int16_t& day);

    /// @brief Internal decode time operation into hour, minute, second, and millisecond
    /// @param dt double, Date to decode
    /// @param hour int16_t&, Hour (output)
    /// @param minute int16_t&, Minute (output)
    /// @param second int16_t&, Second (output)
    /// @param millisecond int16_t&, Millisecond (output)
    static void   decodeTime(double dt, int16_t& hour, int16_t& minute, int16_t& second, int16_t& millisecond);

    /// @brief Internal encode date operation from y,m,d
    static void   encodeDate(double &dt,int16_t y=0,int16_t m=0,int16_t d=0);

    /// @brief Internal encode date operation from string
    static void   encodeDate(double &dt,const char *dat);

    /// @brief Internal encode timee operation from h,m,s,ms
    static void   encodeTime(double &dt,int16_t h=0,int16_t m=0,int16_t s=0,int16_t ms=0);

    /// @brief Internal encode timee operation from string
    static void   encodeTime(double &dt,const char *tim);

    /// @brief Returns true for the leap year
    static int    isLeapYear(const int16_t year)
    {
        return ((year&3) == 0 && year%100) || ((year%400) == 0);
    }

protected:

    double                m_dateTime;             ///< Actual date and time value

public:

    static char           dateFormat[32];         ///< System's date format
    static char           fullTimeFormat[32];     ///< System's time format
    static char           shortTimeFormat[32];    ///< System's time format
    static char           datePartsOrder[4];      ///< System's date parts order
    static char           dateSeparator;          ///< System's date separator
    static char           timeSeparator;          ///< System's time separator
    static std::string    weekDayNames[7];        ///< The locale-defined weekday names
    static std::string    monthNames[12];         ///< The locale-defined weekday names
    static std::string    timeZoneName;           ///< Time zone abbbreviastion
    static int            timeZoneOffset;         ///< Time zone offset from GMT in hours

    static void tzset();                          ///< Call that function if standard tzset() was called

public:

    /// @brief Constructor
    /// @param y int16_t, year
    /// @param m int16_t, month
    /// @param d int16_t, day
    /// @param h int16_t, hour
    /// @param mm int16_t, minute
    /// @param s int16_t, second
    DateTime (int16_t y,int16_t m,int16_t d,int16_t h=0,int16_t mm=0,int16_t s=0);

    /// @brief Constructor
    /// @param dateStr const char *, date string
    DateTime (const char * dateStr);

    /// @brief Copy constructor
    DateTime (const DateTime &dt);

    /// @brief Constructor
    /// @param dt double, floating point date and time value
    DateTime (double dt=0);

    /// @brief Conversion to double
    operator double () const;

    /// @brief Assignment
    void operator = (const DateTime& date);

    /// @brief Assignment
    void operator = (const char * dat);

    /// @brief Addition, another CDateTime
    DateTime  operator + (DateTime& dt);

    /// @brief Substruction, another CDateTime
    DateTime  operator - (DateTime& dt);

    /// @brief Increment by another CDateTime
    DateTime& operator += (DateTime& dt);

    /// @brief Decrement by another CDateTime
    DateTime& operator -= (DateTime& dt);

    /// @brief Increment by day, prefix
    DateTime& operator ++ ();

    /// @brief Increment by day, postfix
    DateTime& operator ++ (int);

    /// @brief Decrement by day, prefix
    DateTime& operator -- ();

    /// @brief Decrement by day, postfix
    DateTime& operator -- (int);

    /// @brief Print the date into str
    void formatDate(char *str, bool universalDateFormat=false) const;

    /// @brief Print the date into str
    void formatTime(char *str, bool ampm=true, bool showSeconds=false, bool showTimezone=false) const;

    /// @brief Returns value as Unix epoch time.
    /// @return Unix epoch time
    time_t toEpoch() const;

    /// @brief Sets value as Unix epoch time.
    /// @param dt time_t, Unix epoch time
    void fromEpoch(time_t dt);

    /// @brief Set the current date and time for this program only.
    ///
    /// The system time is not affected. Useful for synchronization between
    /// different hosts' programs.
    static void Now(DateTime dt);

    /// @brief Reports the system date and time.
    static DateTime System();

    /// @brief Reports the current date and time.
    static DateTime Now();

    /// @brief Reports the current date.
    static DateTime Date();

    /// @brief Reports the current time.
    static DateTime Time();

    /// @brief Reports the current time of day in milliseconds
    ///
    /// This is fast method to get a time of day without considering dateTimeOffset.
    /// It's useful for measuring short time intervals (shorter than one day, anyway).
    static uint32_t TimeOfDayMs();

    /// @brief Converts C time into CDateTime
    static DateTime convertCTime(const time_t tt);

    /// @brief Reports the number of days in the month in this date (1..31)
    int16_t daysInMonth() const;

    /// @brief Reports the day of the week in this date (1..7)
    int16_t dayOfWeek(void) const;

    /// @brief Reports the day since the beginning of the year in this date
    int16_t dayOfYear()  const;

    /// @brief Reports the day of the week name in this date ('Sunday'..'Saturday')
    std::string dayOfWeekName(void) const;

    /// @brief Reports the month name in this date ('Sunday'..'Saturday')
    std::string monthName() const;

    /// @brief Reports the date as an integer
    uint32_t date() const;

    /// @brief Reports the day of month (1..31)
    int16_t day() const;

    /// @brief Reports the month number (1..12)
    int16_t month() const;

    /// @brief Reports the year
    int16_t year() const;

    /// @brief Returns date as a string
    /// @param universalDateFormat bool, If false then use local date format. Otherwise use "YYYY-MM-DD" format
    std::string dateString(bool universalDateFormat=false) const;

    /// @brief Returns time as a string
    std::string timeString(bool showSeconds=false, bool showTimezone=false) const;

    /// @brief Returns date and time as a string
    operator std::string () const
    {
        return dateString() + " " + timeString();
    }

    /// @brief Returns interval between two dates in seconds
    double secondsTo(DateTime toDate) const
    {
        return (toDate - m_dateTime) * 86400;
    }

    /// @brief Decodes date into y,m,d
    void decodeDate(int16_t *y,int16_t *m,int16_t *d) const
    {
        decodeDate(m_dateTime,*y,*m,*d);
    }

    /// @brief Decodes time into h,m,s,ms
    void decodeTime(int16_t *h,int16_t *m,int16_t *s,int16_t *ms) const
    {
        decodeTime(m_dateTime,*h,*m,*s,*ms);
    }

    /// @brief Returns system's time mode.
    static bool time24Mode();

    /// @brief Sets system's time mode
    static void time24Mode(bool t24mode);
};
/// @}
}

/// @brief Compares CDatetime values
bool operator <  (const sptk::DateTime &dt1, const sptk::DateTime &dt2);

/// @brief Compares CDateTime values
bool operator <= (const sptk::DateTime &dt1, const sptk::DateTime &dt2);

/// @brief Compares CDateTime values
bool operator >  (const sptk::DateTime &dt1, const sptk::DateTime &dt2);

/// @brief Compares CDatetime values
bool operator >= (const sptk::DateTime &dt1, const sptk::DateTime &dt2);

/// @brief Compares CDatetime values
bool operator == (const sptk::DateTime &dt1, const sptk::DateTime &dt2);

/// @brief Compares CDatetime values
bool operator != (const sptk::DateTime &dt1, const sptk::DateTime &dt2);

/// @brief Adds two CDatetime values
sptk::DateTime operator + (const sptk::DateTime &dt1, const sptk::DateTime &dt2);

/// @brief Subtracts two CDatetime values
sptk::DateTime operator - (const sptk::DateTime &dt1, const sptk::DateTime &dt2);

#endif
