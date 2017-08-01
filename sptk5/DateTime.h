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

#include <chrono>
#include <ctime>
#include <sptk5/sptk.h>
#include <string>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

class DateTimeFormat;

/**
 * @brief Date and Time value.
 *
 * Represents the date and time value. This value is stored as
 * a floating point number. Allows to synchronize the Now() time
 * with the external date/time, without affecting the local host
 * system time.
 */
class SP_EXPORT DateTime
{
    friend class DateTimeFormat;
    typedef std::chrono::system_clock               clock;

public:
    typedef std::chrono::system_clock::time_point   time_point;
    typedef std::chrono::system_clock::duration     duration;

protected:

    /**
     * @brief Internal decode date operation into year, month, and day
     * @param dt Clock::time_point, Date to decode
     * @param year int16_t&, Year (output)
     * @param month int16_t&, Month (output)
     * @param day int16_t&, Day (output)
     */
    static void decodeDate(const time_point& dt, short& year, short& month, short& day, short& dayOfWeek, short& dayOfYear,
                               bool gmt=false);

    /**
     * @brief Internal decode time operation into hour, minute, second, and millisecond
     * @param dt Clock::time_point, Date to decode
     * @param hour int16_t&, Hour (output)
     * @param minute int16_t&, Minute (output)
     * @param second int16_t&, Second (output)
     * @param millisecond int16_t&, Millisecond (output)
     */
    static void decodeTime(const time_point& dt, short& hour, short& minute, short& second, short& millisecond, bool gmt);

    /**
     * @brief Internal encode date operation from y,m,d
     */
    static void   encodeDate(time_point& dt,int16_t y=0,int16_t m=0,int16_t d=0);

    /**
     * @brief Internal encode date operation from string
     */
    static void   encodeDate(time_point& dt,const char *dat);

    /**
     * @brief Internal encode timee operation from h,m,s,ms
     */
    static void   encodeTime(time_point& dt,int16_t h=0,int16_t m=0,int16_t s=0,int16_t ms=0);

    /**
     * @brief Internal encode timee operation from string
     */
    static void   encodeTime(time_point& dt,const char *tim);

    /**
     * @brief Returns true for the leap year
     */
    static int    isLeapYear(const int16_t year)
    {
        return ((year&3) == 0 && year%100) || ((year%400) == 0);
    }

protected:

    /**
     * Actual date and time value
     */
    time_point            m_dateTime;


public:

    /**
     * System's date format
     */
    static char           dateFormat[32];

    /**
     * System's time format
     */
    static char           fullTimeFormat[32];

    /**
     * System's time format
     */
    static char           shortTimeFormat[32];

    /**
     * System's date parts order
     */
    static char           datePartsOrder[4];

    /**
     * System's date separator
     */
    static char           dateSeparator;

    /**
     * System's time separator
     */
    static char           timeSeparator;

    /**
     * The locale-defined weekday names
     */
    static std::string    weekDayNames[7];

    /**
     * The locale-defined weekday names
     */
    static std::string    monthNames[12];

    /**
     * Time zone abbbreviastion
     */
    static std::string    timeZoneName;

    /**
     * Time zone offset from GMT in hours
     */
    static int            timeZoneOffset;


    /**
     * Call that function if standard tzset() was called
     */
    static void tzset();


public:

    /**
     * @brief Default constructor
     */
    DateTime() noexcept {}

    /**
     * @brief Constructor
     * @param y int16_t, year
     * @param m int16_t, month
     * @param d int16_t, day
     * @param h int16_t, hour
     * @param mm int16_t, minute
     * @param s int16_t, second
     */
    DateTime (int16_t y,int16_t m,int16_t d,int16_t h=0,int16_t mm=0,int16_t s=0) noexcept;

    /**
     * @brief Constructor
     * @param dateStr const char *, date string
     */
    DateTime (const char * dateStr) noexcept;

    /**
     * @brief Copy constructor
     */
    DateTime (const DateTime& dt) noexcept = default;

    /**
     * @brief Constructor
     * @param dt const time_point&, Time point
     */
    DateTime(const time_point& dt) noexcept;

    /**
     * @brief Constructor
     * @param dt const duration&, Duration since epoch
     */
    DateTime(const duration& dt) noexcept;

    /**
     * @brief Constructor
     * @param dt int64_t, Time since epoch, milliseconds
     */
    DateTime(int64_t sinceEpochMS) noexcept;

    time_point& timePoint()
    {
        return m_dateTime;
    }

    const time_point& timePoint() const
    {
        return m_dateTime;
    }

    /**
     * @brief Assignment
     */
    DateTime& operator =(const DateTime& date) = default;

    /**
     * @brief Assignment
     */
    DateTime& operator =(const char* dat);

    /**
     * @brief Addition, another DateTime
     */
    DateTime  operator + (duration& dt);

    /**
     * @brief Substruction, another DateTime
     */
    DateTime  operator - (duration& dt);

    /**
     * @brief Increment by another DateTime
     */
    DateTime& operator += (duration& dt);

    /**
     * @brief Decrement by another DateTime
     */
    DateTime& operator -= (duration& dt);

    /**
     * @brief Increment by day, prefix
     */
    DateTime& operator ++ ();

    /**
     * @brief Increment by day, postfix
     */
    DateTime& operator ++ (int);

    /**
     * @brief Decrement by day, prefix
     */
    DateTime& operator -- ();

    /**
     * @brief Decrement by day, postfix
     */
    DateTime& operator -- (int);

    /**
     * @brief Print the date into str
     */
    void formatDate(char *str, bool universalDateFormat=false) const;

    /**
     * @brief Print the date into str
     */
    void formatTime(char* str, bool ampm, bool showSeconds, bool showTimezone, bool showMilliseconds) const;

    /**
     * Duration since epoch
     */
    duration sinceEpoch() const
    {
        return m_dateTime.time_since_epoch();
    }

    /**
     * @brief Set the current date and time for this program only.
     *
     * The system time is not affected. Useful for synchronization between
     * different hosts' programs.
     */
    static void Now(DateTime dt);

    /**
     * @brief Reports the system date and time.
     */
    static DateTime System();

    /**
     * @brief Reports the current date and time.
     */
    static DateTime Now();

    /**
     * @brief Reports the current date.
     */
    static DateTime Date();

    /**
     * @brief Reports the current time.
     */
    static DateTime Time();

    /**
     * @brief Converts C time into DateTime
     */
    static DateTime convertCTime(const time_t tt);

    /**
     * @brief Reports the number of days in the month in this date (1..31)
     */
    int16_t daysInMonth() const;

    /**
     * @brief Reports the day of the week in this date (1..7)
     */
    int16_t dayOfWeek(void) const;

    /**
     * @brief Reports the day since the beginning of the year in this date
     */
    int16_t dayOfYear()  const;

    /**
     * @brief Reports the day of the week name in this date ('Sunday'..'Saturday')
     */
    std::string dayOfWeekName(void) const;

    /**
     * @brief Reports the month name in this date ('Sunday'..'Saturday')
     */
    std::string monthName() const;

    /**
     * @brief Reports the date part only
     */
    DateTime date() const;

    /**
     * @brief Reports the day of month (1..31)
     */
    int16_t day() const;

    /**
     * @brief Reports the month number (1..12)
     */
    int16_t month() const;

    /**
     * @brief Reports the year
     */
    int16_t year() const;

    /**
     * @brief Returns date as a string
     * @param universalDateFormat bool, If false then use local date format. Otherwise use "YYYY-MM-DD" format
     */
    std::string dateString(bool universalDateFormat=false) const;

    /**
     * @brief Returns time as a string
     */
    std::string timeString(bool showSeconds=false, bool showTimezone=false, bool showMilliseconds=false) const;

    /**
     * @brief Returns time as a ISO date and time string
     */
    std::string isoDateTimeString(bool showMilliseconds=false) const;

    /**
     * @brief Returns date and time as a string
     */
    operator std::string () const
    {
        return dateString() + " " + timeString();
    }

    /**
     * @brief Decodes date into y,m,d
     */
    void decodeDate(short* year, short* month, short *day, short* wday, short* yday, bool gmt=false) const
    {
        decodeDate(m_dateTime, *year, *month, *day, *wday, *yday, gmt);
    }

    /**
     * @brief Decodes time into h,m,s,ms
     */
    void decodeTime(int16_t *h,int16_t *m,int16_t *s,int16_t *ms, bool gmt = false) const
    {
        decodeTime(m_dateTime,*h,*m,*s,*ms, gmt);
    }

    bool zero() const
    {
        return m_dateTime.time_since_epoch() == std::chrono::microseconds(0);
    }

    /**
     * @brief Returns system's time mode.
     */
    static bool time24Mode();

    /**
     * @brief Sets system's time mode
     */
    static void time24Mode(bool t24mode);
};
/**
 * @}
 */
}

/**
 * @brief Compares DateTime values
 */
bool operator <  (const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Compares DateTime values
 */
bool operator <= (const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Compares DateTime values
 */
bool operator >  (const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Compares DateTime values
 */
bool operator >= (const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Compares DateTime values
 */
bool operator == (const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Compares DateTime values
 */
bool operator != (const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Adds two DateTime values
 */
sptk::DateTime operator + (const sptk::DateTime& dt1, const sptk::DateTime::duration& duration);

/**
 * @brief Adds two DateTime values
 */
sptk::DateTime operator - (const sptk::DateTime& dt1, const sptk::DateTime::duration& duration);

/**
 * @brief Subtracts two DateTime values
 */
sptk::DateTime::duration operator - (const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * Convert duration into seconds, with 1 msec accuracy
 */
double duration2seconds(const sptk::DateTime::duration& duration);

#endif
