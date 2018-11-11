/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DateTime.h - description                               ║
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

#ifndef __SPTK_DATETIME_H__
#define __SPTK_DATETIME_H__

#include <chrono>
#include <ctime>
#include <iostream>
#include <sptk5/sptk.h>
#include <sptk5/String.h>

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

public:

    /**
     * Clock used by DateTime
     */
    typedef std::chrono::system_clock clock;

    /**
     * DateTime::time_point type definition
     */
    typedef std::chrono::system_clock::time_point time_point;

    /**
     * DateTime::duration type definition
     */
    typedef std::chrono::system_clock::duration duration;

    /**
     * Time print accuracy
     */
    enum PrintAccuracy
    {
        PA_MINUTES = 1,
        PA_SECONDS = 2,
        PA_MILLISECONDS = 3
    };

    /**
     * Date and time print flags
     */
    enum PrintFlags
    {
        PF_RFC_DATE = 1,
        PF_TIMEZONE = 2,
        PF_12HOURS = 4,
        PF_GMT = 16
    };

    protected:

    /**
     * @brief Internal decode date operation into year, month, and day
     * @param dt                Date to decode
     * @param year              Year (output)
     * @param month             Month (output)
     * @param day               Day (output)
     * @param dayOfWeek         Day of week, 0..6 (output)
     * @param dayOfYear         Day of year (output)
     * @param gmt               Use GMT timezone for output
    */
    static void decodeDate(const time_point& dt, short& year, short& month, short& day, short& dayOfWeek, short& dayOfYear,
                           bool gmt = false);

    /**
     * @brief Internal decode time operation into hour, minute, second, and millisecond
     * @param dt                Date to decode
     * @param hour              Hour (output)
     * @param minute            Minute (output)
     * @param second            Second (output)
     * @param millisecond       Millisecond (output)
     * @param gmt               Use GMT timezone for output
    */
    static void decodeTime(const time_point& dt, short& hour, short& minute, short& second, short& millisecond,
                           bool gmt = false);

    /**
     * @brief Internal encode date operation from y,m,d
     */
    static void encodeDate(time_point& dt, short y = 0, short m = 0, short d = 0);

    /**
     * @brief Internal encode date operation from string
     */
    static void encodeDate(time_point& dt, const char* dat);

    /**
     * @brief Internal encode timee operation from h,m,s,ms
     */
    static void encodeTime(time_point& dt, short h = 0, short m = 0, short s = 0, short ms = 0);

    /**
     * @brief Internal encode timee operation from string
     */
    static void encodeTime(time_point& dt, const char* tim);

    /**
     * @brief Returns true for the leap year
     */
    static int isLeapYear(const int16_t year)
    {
        return ((year&3) == 0 && year%100) || ((year%400) == 0);
    }

protected:

    /**
     * Actual date and time value
     */
    time_point m_dateTime;

public:

    /**
     * System's date format
     */
    static char dateFormat[32];

    /**
     * System's time format
     */
    static char fullTimeFormat[32];

    /**
     * System's time format
     */
    static char shortTimeFormat[32];

    /**
     * System's date parts order
     */
    static char datePartsOrder[4];

    /**
     * System's date separator
     */
    static char dateSeparator;

    /**
     * System's time separator
     */
    static char timeSeparator;

    /**
     * The locale-defined weekday names
     */
    static String weekDayNames[7];

    /**
     * The locale-defined weekday names
     */
    static String monthNames[12];

    /**
     * Time zone abbbreviastion
     */
    static String timeZoneName;

    /**
    * Time zone offset from GMT in minutes
    */
    static int timeZoneOffset;

    /**
    * Daylight savings time 0 or 1
    */
    static int isDaylightSavingsTime;

    /**
    * Set timezone for the application
    * @param tzname             Time zone name, such as "UTC", ":US/Pacific", etc
    */
    static void setTimeZone(const sptk::String& tzname);


public:

    /**
    * @brief Default constructor
    */
    DateTime() noexcept
    {}

    /**
    * @brief Constructor
    * @param y                  Year
    * @param m                  Month
    * @param d                  Day
    * @param h                  Hour
    * @param mm                 Minute
    * @param s                  Second
    * @param ms                 Millisecond
    */
    DateTime(short y, short m, short d, short h = 0, short mm = 0, short s = 0, short ms = 0) noexcept;

    /**
     * @brief Constructor
     * @param dateStr           Date string
     */
    explicit DateTime(const char* dateStr) noexcept;

    /**
     * @brief Copy constructor
     */
    DateTime(const DateTime& dt) noexcept = default;

    /**
     * @brief Constructor
     * @param dt                Time point
     */
    explicit DateTime(const time_point& dt) noexcept;

    /**
     * @brief Constructor
     * @param dt                Duration since epoch
     */
    explicit DateTime(const duration& dt) noexcept;

    /**
     * @brief Constructor
     * @param sinceEpochMS      Time since epoch, milliseconds
     */
    explicit DateTime(int64_t sinceEpochMS) noexcept;

    /**
     * Returns time_point presentation of the date and time
     */
    time_point& timePoint()
    {
        return m_dateTime;
    }

    /**
     * Returns time_point presentation of the date and time
     */
    const time_point& timePoint() const
    {
        return m_dateTime;
    }

    /**
     * @brief Assignment
     */
    DateTime& operator=(const DateTime& date) = default;

    /**
     * @brief Assignment
     */
    DateTime& operator=(const char* dat);

    /**
     * @brief Addition, another DateTime
     */
    DateTime operator+(duration& dt);

    /**
     * @brief Substruction, another DateTime
     */
    DateTime operator-(duration& dt);

    /**
     * @brief Increment by another DateTime
     */
    DateTime& operator+=(duration& dt);

    /**
     * @brief Decrement by another DateTime
     */
    DateTime& operator-=(duration& dt);

    /**
     * @brief Increment by day, prefix
     */
    DateTime& operator++();

    /**
     * @brief Increment by day, postfix
     */
    const DateTime operator++(int);

    /**
     * @brief Decrement by day, prefix
     */
    DateTime& operator--();

    /**
     * @brief Decrement by day, postfix
     */
    const DateTime operator--(int);

    /**
     * @brief Print the date into stream
     * @param str               Output stream
     * @param printFlags        Print flags, recognised { PF_GMT, PF_RFC_DATE }
     */
    void formatDate(std::ostream& str, int printFlags=0) const;

    /**
     * Print date into string
     * @param str               Output stream
     * @param printFlags        Print flags, recognised { PF_GMT, PF_TIMEZONE, PF_12HOURS }
     * @param printAccuracy     Print accuracy, @see PrintAccuracy
     */
    void formatTime(std::ostream& str, int printFlags=0, PrintAccuracy printAccuracy=PA_SECONDS) const;

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
     * @param tt                C time to convert
     */
    static DateTime convertCTime(const time_t tt);

    /**
     * @brief Reports the number of days in the month in this date (1..31)
     */
    int16_t daysInMonth() const;

    /**
     * @brief Reports the day of the week in this date (1..7)
     */
    int16_t dayOfWeek() const;

    /**
     * @brief Reports the day since the beginning of the year in this date
     */
    short dayOfYear() const;

    /**
     * @brief Reports the day of the week name in this date ('Sunday'..'Saturday')
     */
    String dayOfWeekName() const;

    /**
     * @brief Reports the month name in this date ('Sunday'..'Saturday')
     */
    String monthName() const;

    /**
     * @brief Reports the date part only
     */
    DateTime date() const;

    /**
     * @brief Reports the day of month (1..31)
     */
    short day() const;

    /**
     * @brief Reports the month number (1..12)
     */
    short month() const;

    /**
     * @brief Reports the year
     */
    short year() const;

    /**
     * @brief Returns date as a string
     * @param printFlags        Print flags, recognised { PF_GMT, PF_RFC_DATE }
     */
    String dateString(int printFlags = 0) const;

    /**
     * @brief Returns time as a string
     * @param printFlags        Print flags, recognised { PF_GMT, PF_TIMEZONE, PF_12HOURS }
     * @param printAccuracy     Print accuracy, @see PrintAccuracy
     */
    String timeString(int printFlags = 0, PrintAccuracy printAccuracy = PA_SECONDS) const;

    /**
     * @brief Returns time as a ISO date and time string
     * @param printAccuracy     Print accuracy, @see PrintAccuracy
     * @param gmt               If true print GMT time
     */
    String isoDateTimeString(PrintAccuracy printAccuracy = PA_SECONDS, bool gmt = false) const;

    /**
     * @brief Returns date and time as a string
     */
    operator String() const
    {
        return dateString() + " " + timeString();
    }

    /**
     * @brief Returns time_t presentation
     */
    operator time_t() const
    {
        return clock::to_time_t(m_dateTime);
    }

    /**
     * @brief Decodes date into y,m,d
     */
    void decodeDate(short* year, short* month, short* day, short* wday, short* yday, bool gmt = false) const
    {
        decodeDate(m_dateTime, *year, *month, *day, *wday, *yday, gmt);
    }

    /**
     * @brief Decodes time into h,m,s,ms
     */
    void decodeTime(short* h, short* m, short* s, short* ms, bool gmt = false) const
    {
        decodeTime(m_dateTime, *h, *m, *s, *ms, gmt);
    }

    /**
     * @brief Return true if date and time are at epoch
     */
    bool zero() const
    {
        return m_dateTime.time_since_epoch().count() == 0;
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
 * @brief Compares DateTime values
 */
bool operator<(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Compares DateTime values
 */
bool operator<=(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Compares DateTime values
 */
bool operator>(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Compares DateTime values
 */
bool operator>=(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Compares DateTime values
 */
bool operator==(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Compares DateTime values
 */
bool operator!=(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * @brief Adds two DateTime values
 */
sptk::DateTime operator+(const sptk::DateTime& dt1, const sptk::DateTime::duration& duration);

/**
 * @brief Adds two DateTime values
 */
sptk::DateTime operator-(const sptk::DateTime& dt1, const sptk::DateTime::duration& duration);

/**
 * @brief Subtracts two DateTime values
 */
sptk::DateTime::duration operator-(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * Convert duration into seconds, with 1 msec accuracy
 */
double duration2seconds(const sptk::DateTime::duration& duration);

/**
 * @}
 */
}

#endif
