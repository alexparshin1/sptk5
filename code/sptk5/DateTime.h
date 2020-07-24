/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
class DateTime;

/**
 * Timezone-related information (global, static)
 */
class SP_EXPORT TimeZone
{
public:
    /**
     * Time zone abbbreviastion
     */
    static String name();

    /**
    * Get timezone offset
    * @return timezone offset, seconds
    */
    static int offset();

    /**
    * Get timezone offset
    * @return timezone offset
    */
    static int isDaylightSavingsTime();

    /**
    * Set timezone for the application
    * @param timeZoneName       Time zone name, such as "UTC", ":US/Pacific", etc
    */
    static void set(const sptk::String& timeZoneName);
};


/**
 * Date and Time value.
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

    enum Format {
        DATE_FORMAT,
        DATE_PARTS_ORDER,
        FULL_TIME_FORMAT,
        SHORT_TIME_FORMAT,
        MONTH_NAME,
        WEEKDAY_NAME
    };

	/**
	 * System's format info
	 * @param dtFormat          Format type
	 * @param arg               Optional format argument, for MONTH_NAME and WEEKDAY_NAME
	 */
	static String format(Format dtFormat, size_t arg=0);

	/**
	 * System's date separator
	 */
	static char dateSeparator();

	/**
	 * System's time separator
	 */
	static char timeSeparator();

	/**
	 * Returns system's time mode.
	 */
	static bool time24Mode();

	/**
	 * Sets system's time mode
	 */
	static void time24Mode(bool t24mode);

    /**
    * Constructor
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
     * Constructor
     * @param dateStr           Date string
     */
    explicit DateTime(const char* dateStr=nullptr) noexcept;

    /**
     * Copy constructor
     */
    DateTime(const DateTime& dt) noexcept = default;

    /**
     * Constructor
     * @param dt                Time point
     */
    explicit DateTime(const time_point& dt) noexcept;

    /**
     * Constructor
     * @param dt                Duration since epoch
     */
    explicit DateTime(const duration& dt) noexcept;

    /**
     * Returns time_point presentation of the date and time
     */
    const time_point& timePoint() const
    {
        return m_dateTime;
    }

    /**
     * Assignment
     */
    DateTime& operator=(const DateTime& date) = default;

    /**
     * Print the date into stream
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
     * Reports the current date and time.
     */
    static DateTime Now();

    /**
     * Converts C time into DateTime
     * @param tt                C time to convert
     */
    static DateTime convertCTime(const time_t tt);

    /**
     * Reports the number of days in the month in this date (1..31)
     */
    int16_t daysInMonth() const;

    /**
     * Reports the day of the week in this date (1..7)
     */
    int16_t dayOfWeek() const;

    /**
     * Reports the day of the week name in this date ('Sunday'..'Saturday')
     */
    String dayOfWeekName() const;

    /**
     * Reports the month name in this date ('Sunday'..'Saturday')
     */
    String monthName() const;

    /**
     * Reports the date part only
     */
    DateTime date() const;

    /**
     * Returns date as a string
     * @param printFlags        Print flags, recognised { PF_GMT, PF_RFC_DATE }
     */
    String dateString(int printFlags = 0) const;

    /**
     * Returns time as a string
     * @param printFlags        Print flags, recognised { PF_GMT, PF_TIMEZONE, PF_12HOURS }
     * @param printAccuracy     Print accuracy, @see PrintAccuracy
     */
    String timeString(int printFlags = 0, PrintAccuracy printAccuracy = PA_SECONDS) const;

    /**
     * Returns time as a ISO date and time string
     * @param printAccuracy     Print accuracy, @see PrintAccuracy
     * @param gmt               If true print GMT time
     */
    String isoDateTimeString(PrintAccuracy printAccuracy = PA_SECONDS, bool gmt = false) const;

    /**
     * Returns timezone offset in minutes
     * @return timezone offset in minutes
     */
    static int timeZoneOffset();

    /**
     * Returns timezone name
     * @return timezone name
     */
    static String timeZoneName();

    /**
     * Returns true if daylight savings time
     * @return true if daylight savings time
     */
    static bool isDaylightSavingsTime();

    /**
     * Returns date and time as a string
     */
    explicit operator String() const
    {
        return dateString() + " " + timeString();
    }

    /**
     * Returns time_t presentation
     */
    explicit operator time_t() const
    {
        return clock::to_time_t(m_dateTime);
    }

    /**
     * Decodes date into y,m,d
     */
    void decodeDate(short* year, short* month, short* day, short* wday, short* yday, bool gmt = false) const;

    /**
     * Decodes time into h,m,s,ms
     */
    void decodeTime(short* h, short* m, short* s, short* ms, bool gmt = false) const;

    /**
     * Return true if date and time are at epoch
     */
    bool zero() const
    {
        return m_dateTime.time_since_epoch().count() == 0;
    }

private:

    time_point      m_dateTime;             ///< Actual date and time value

    static String   _dateFormat;            ///< System's date format
    static String   _fullTimeFormat;        ///< System's time format
    static String   _shortTimeFormat;       ///< System's time format
    static String   _datePartsOrder;        ///< System's date parts order
    static char     _dateSeparator;         ///< System's date separator
    static char     _timeSeparator;         ///< System's time separator
    static Strings  _weekDayNames;          ///< The locale-defined weekday names
    static Strings  _monthNames;            ///< The locale-defined weekday names

    static bool     _time24Mode;
    static String   _timeZoneName;
    static int      _timeZoneOffset;
    static int      _isDaylightSavingsTime;
};


/**
 * Compares DateTime values
 */
SP_EXPORT bool operator<(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * Compares DateTime values
 */
SP_EXPORT bool operator<=(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * Compares DateTime values
 */
SP_EXPORT bool operator>(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * Compares DateTime values
 */
SP_EXPORT bool operator>=(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * Compares DateTime values
 */
SP_EXPORT bool operator==(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * Compares DateTime values
 */
SP_EXPORT bool operator!=(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * Adds two DateTime values
 */
SP_EXPORT sptk::DateTime operator+(const sptk::DateTime& dt1, const sptk::DateTime::duration& duration);

/**
 * Adds two DateTime values
 */
SP_EXPORT sptk::DateTime operator-(const sptk::DateTime& dt1, const sptk::DateTime::duration& duration);

/**
 * Subtracts two DateTime values
 */
SP_EXPORT sptk::DateTime::duration operator-(const sptk::DateTime& dt1, const sptk::DateTime& dt2);

/**
 * Convert duration into seconds, with 1 msec accuracy
 */
SP_EXPORT double duration2seconds(const sptk::DateTime::duration& duration);

/**
 * @}
 */
}

#endif
