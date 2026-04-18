/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/String.h>
#include <sptk5/VariantStorageClient.h>

#include <chrono>
#include <ctime>
#include <iostream>
#include <mutex>

namespace sptk {
/**
 * @addtogroup utility Utility Classes.
 * @{
 */

class DateTimeFormat;

class DateTime;

/**
 *  @brief Timezone-related information (global, static).
 */
class SP_EXPORT TimeZone
{
    friend class DateTime;
    friend class DateTimeFormat;

public:
    /**
     *  @brief Time zone abbbreviastion.
     */
    static String name();

    /**
    *  @brief Set the timezone for the application.
    * @param timeZoneName       Time zone name, such as "UTC", "US/Pacific", etc.
    */
    static void set(const String& timeZoneName);

    static bool time24Mode();

    /**
    *  @brief Get timezone offset.
    * @return timezone offset, minutes.
    */
    static std::chrono::minutes offset();

    /**
    *  @brief Get timezone offset.
    * @return timezone offset.
    */
    static int isDaylightSavingsTime();

protected:
    static void time24Mode(bool mode);
    static void timeZoneName(const String& name);
    static void timeZoneOffset(std::chrono::minutes offset);
    static void isDaylightSavingsTime(int savingsTime);

private:
    static bool                 _time24Mode;
    static String               _timeZoneName;
    static std::chrono::minutes _timeZoneOffset;
    static int                  _isDaylightSavingsTime;
};


/**
 *  @brief Date and Time value.
 *
 * Represents the date and time value. This value is stored as
 * a floating point number. Allows synchronizing the Now() time
 * with the external date/time, without affecting the local host
 * system time.
 */
class SP_EXPORT DateTime : public VariantStorageClient
{
    friend class DateTimeFormat;

public:
    /**
     *  @brief Clock used by DateTime.
     */
    using clock = std::chrono::system_clock;

    /**
     *  @brief DateTime::time_point type definition.
     */
    using time_point = clock::time_point;

    /**
     *  @brief DateTime::duration type definition.
     */
    using duration = clock::duration;

    /**
     *  @brief Time print accuracy.
     */
    enum class PrintAccuracy
    {
        MINUTES = 1,
        SECONDS = 2,
        MILLISECONDS = 3
    };

    /**
     *  @brief Date and time print flags.
     */
    static constexpr auto PF_RFC_DATE = 1;
    static constexpr auto PF_TIMEZONE = 2;
    static constexpr auto PF_12HOURS = 4;
    static constexpr auto PF_GMT = 16;

    enum class Format
    {
        DATE_FORMAT,
        DATE_PARTS_ORDER,
        FULL_TIME_FORMAT,
        SHORT_TIME_FORMAT,
        MONTH_NAME,
        WEEKDAY_NAME
    };

    /**
     *  @brief System's format info.
     * @param dtFormat          Format type.
     * @param arg               Optional format argument, for MONTH_NAME and WEEKDAY_NAME.
     */
    static String format(Format dtFormat, size_t arg = 0);

    /**
     *  @brief System's date separator.
     */
    static char dateSeparator();

    /**
     *  @brief Sets system's time mode.
     */
    static void time24Mode(bool t24mode);

    /**
    *  @brief Constructor.
    * @param y                  Year.
    * @param m                  Month.
    * @param d                  Day.
    * @param h                  Hour.
    * @param mm                 Minute.
    * @param s                  Second.
    * @param ms                 Millisecond.
    */
    DateTime(short y, short m, short d, short h = 0, short mm = 0, short s = 0, short ms = 0);

    /**
     *  @brief Constructor.
     * @param dateStr           Date string.
     */
    explicit DateTime(const char* dateStr = nullptr);

    /**
     *  @brief Copy constructor.
     */
    DateTime(const DateTime& dt);

    /**
     *  @brief Move constructor.
     */
    DateTime(DateTime&& dt) noexcept;

    /**
     *  @brief Constructor.
     * @param timePoint                Time point.
     */
    explicit DateTime(const time_point& timePoint);

    /**
     *  @brief Constructor.
     * @param interval                Duration since epoch.
     */
    explicit DateTime(const duration& interval);

    /**
     *  @brief Returns time_point presentation of the date and time.
     */
    [[nodiscard]] time_point timePoint() const;

    [[nodiscard]] size_t dataSize() const override;

    /**
     *  @brief Assignment.
     */
    DateTime& operator=(const DateTime& date);

    /**
     *  @brief Move assignment.
     */
    DateTime& operator=(DateTime&& date) noexcept;

    /**
     *  @brief Print the date into the stream.
     * @param str               Output stream.
     * @param printFlags        Print flags, recognized { PF_GMT, PF_RFC_DATE }.
     */
    void formatDate(std::ostream& str, int printFlags = 0) const;

    /**
     *  @brief Print the date into string.
     * @param str               Output stream.
     * @param printFlags        Print flags, recognized { PF_GMT, PF_TIMEZONE, PF_12HOURS }.
     * @param printAccuracy     Print accuracy: @see PrintAccuracy.
     */
    void formatTime(std::ostream& str, int printFlags = 0, PrintAccuracy printAccuracy = PrintAccuracy::SECONDS) const;

    /**
     *  @brief Duration since epoch.
     */
    [[nodiscard]] duration sinceEpoch() const;

    /**
     *  @brief Reports the current date and time.
     */
    static DateTime Now();

    /**
     *  @brief Converts C time into DateTime.
     * @param timestamp                C time to convert.
     */
    static DateTime convertCTime(time_t timestamp);

    /**
     *  @brief Reports the number of days in the month in this date (1..31).
     */
    [[nodiscard]] int16_t daysInMonth() const;

    /**
     *  @brief Reports the day of the week in this date (0..6).
     */
    [[nodiscard]] int16_t dayOfWeek() const;

    /**
     *  @brief Reports the day of the week name in this date ('Sunday' to 'Saturday').
     */
    [[nodiscard]] String dayOfWeekName() const;

    /**
     *  @brief Reports the month name in this date ('Sunday' to 'Saturday').
     */
    [[nodiscard]] String monthName() const;

    /**
     *  @brief Reports the date part only.
     */
    [[nodiscard]] DateTime date() const;

    /**
     *  @brief Returns date as a string.
     * @param printFlags        Print flags, recognized { PF_GMT, PF_RFC_DATE }.
     */
    [[nodiscard]] String dateString(int printFlags = 0) const;

    /**
     *  @brief Returns time as a string.
     * @param printFlags        Print flags, recognized { PF_GMT, PF_TIMEZONE, PF_12HOURS }.
     * @param printAccuracy     Print accuracy: @see PrintAccuracy.
     */
    [[nodiscard]] String timeString(int printFlags = 0, PrintAccuracy printAccuracy = PrintAccuracy::SECONDS) const;

    /**
     *  @brief Returns time as an ISO date and time string.
     * @param printAccuracy     Print accuracy: @see PrintAccuracy.
     * @param gmt               If true, print GMT time.
     */
    [[nodiscard]] String isoDateTimeString(PrintAccuracy printAccuracy = PrintAccuracy::SECONDS, bool gmt = false) const;

    /**
     *  @brief Returns date and time as a string.
     */
    [[nodiscard]] explicit operator String() const;

    /**
     *  @brief Returns time_t presentation.
     */
    [[nodiscard]] explicit operator time_t() const;

    /**
     *  @brief Decodes date into y,m,d.
     */
    void decodeDate(short* year, short* month, short* day, short* weekDay, short* yearDate, bool gmt = false) const;

    /**
     *  @brief Decodes time into hour, minute, second, millisecond.
     */
    void decodeTime(short* hour, short* minute, short* second, short* millisecond, bool gmt = false) const;

    /**
     *  @brief Return true if date and time are at epoch.
     */
    [[nodiscard]] bool zero() const;

    /**
     * @return Varian data type.
     */
    static VariantDataType variantDataType();

private:
    mutable std::mutex m_mutex;          ///< Mutex for thread safety.
    time_point         m_dateTime;       ///< Actual date and time value
    static String      _dateFormat;      ///< System's date format
    static String      _fullTimeFormat;  ///< System's time format
    static String      _shortTimeFormat; ///< System's time format
    static String      _datePartsOrder;  ///< System's date parts order
    static char        _dateSeparator;   ///< System's date separator
    static char        _timeSeparator;   ///< System's time separator
    static Strings     _weekDayNames;    ///< The locale-defined weekday names
    static Strings     _monthNames;      ///< The locale-defined weekday names
};

SP_EXPORT int  operator<=>(const DateTime& dt1, const DateTime& dt2);
SP_EXPORT bool operator==(const DateTime& dt1, const DateTime& dt2);

/**
 *  @brief Adds two DateTime values.
 */
SP_EXPORT DateTime operator+(const DateTime& dateTime, const DateTime::duration& duration);

/**
 *  @brief Adds two DateTime values.
 */
SP_EXPORT DateTime operator-(const DateTime& dateTime, const DateTime::duration& duration);

/**
 *  @brief Subtracts two DateTime values.
 */
SP_EXPORT DateTime::duration operator-(const DateTime& dateTime, const DateTime& dt2);

/**
 *  @brief Convert duration into seconds, with 1 msec accuracy.
 */
SP_EXPORT double duration2seconds(const DateTime::duration& duration);

/**
 * @}
 */
} // namespace sptk
