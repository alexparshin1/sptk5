/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDateTime.h  -  description
                             -------------------
    begin                : Tue Dec 14 1999
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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

#ifndef __CDATETIME_H__
#define __CDATETIME_H__

#include <sptk5/sptk.h>
#include <time.h>
#include <string>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

class CDateTime;
class CDateTimeFormat;

/// @brief Date and Time value.
///
/// Represents the date and time value. This value is stored as
/// a floating point number. Allows to synchronize the Now() time
/// with the external date/time, without affecting the local host
/// system time.
class SP_EXPORT CDateTime
{
    friend class CDateTimeFormat;
protected:

    /// Internal decode date operation into y,m,d
    static void   decodeDate(const double dt,int16_t& y,int16_t& m,int16_t& d);

    /// Internal decode time operation into h,m,s,ms
    static void   decodeTime(const double dt,int16_t& h,int16_t& m,int16_t& s,int16_t& ms);

    /// Internal encode date operation from y,m,d
    static void   encodeDate(double &dt,int16_t y=0,int16_t m=0,int16_t d=0);

    /// Internal encode date operation from string
    static void   encodeDate(double &dt,const char *dat);

    /// Internal encode timee operation from h,m,s,ms
    static void   encodeTime(double &dt,int16_t h=0,int16_t m=0,int16_t s=0,int16_t ms=0);

    /// Internal encode timee operation from string
    static void   encodeTime(double &dt,const char *tim);

    /// Returns true for the leap year
    static int    isLeapYear(const int16_t year) {
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

public:

    /// @brief Constructor
    /// @param y int16_t, year
    /// @param m int16_t, month
    /// @param d int16_t, day
    /// @param h int16_t, hour
    /// @param mm int16_t, minute
    /// @param s int16_t, second
    CDateTime (int16_t y,int16_t m,int16_t d,int16_t h=0,int16_t mm=0,int16_t s=0);

    /// @brief Constructor
    /// @param dateStr const char *, date string
    CDateTime (const char * dateStr);

    /// @brief Copy constructor
    CDateTime (const CDateTime &dt);

    /// @brief Constructor
    /// @param dt double, floating point date and time value
    CDateTime (const double dt=0);

    /// @brief Conversion to double
    operator double () const;

    /// @brief Assignment
    void operator = (const CDateTime& date);

    /// @brief Assignment
    void operator = (const char * dat);

    /// @brief Addition, another CDateTime
    CDateTime  operator + (CDateTime& dt);

    /// @brief Substruction, another CDateTime
    CDateTime  operator - (CDateTime& dt);

    /// @brief Increment by another CDateTime
    CDateTime& operator += (CDateTime& dt);

    /// @brief Decrement by another CDateTime
    CDateTime& operator -= (CDateTime& dt);

    /// @brief Increment by day, prefix
    CDateTime& operator ++ ();

    /// @brief Increment by day, postfix
    CDateTime& operator ++ (int);

    /// @brief Decrement by day, prefix
    CDateTime& operator -- ();

    /// @brief Decrement by day, postfix
    CDateTime& operator -- (int);

    /// @brief Print the date into str
    void formatDate(char *str) const;

    /// @brief Print the date into str
    void formatTime(char *str,bool ampm=true,bool showSeconds=false) const;

    /// @brief Set the current date and time for this program only.
    ///
    /// The system time is not affected. Useful for synchronization between
    /// different hosts' programs.
    static void Now(CDateTime dt);

    /// @brief Reports the system date and time.
    static CDateTime System();

    /// @brief Reports the current date and time.
    static CDateTime Now();

    /// @brief Reports the current date.
    static CDateTime Date();

    /// @brief Reports the current time.
    static CDateTime Time();

    /// @brief Reports the current time of day in milliseconds
    ///
    /// This is fast method to get a time of day without considering dateTimeOffset.
    /// It's useful for measuring short time intervals (shorter than one day, anyway).
    static uint32_t TimeOfDayMs();

    /// @brief Converts C time into CDateTime
    static CDateTime convertCTime(const time_t tt);

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
    std::string dateString() const;

    /// @brief Returns time as a string
    std::string timeString(bool showSeconds=false) const;

    /// @brief Returns date and time as a string
    operator std::string () const {
        return dateString() + " " + timeString();
    }

    /// @brief Returns interval between two dates in seconds
    double secondsTo(CDateTime toDate) const
    {
        return (toDate - m_dateTime) * 86400;
    }

    /// @brief Decodes date into y,m,d
    void decodeDate(int16_t *y,int16_t *m,int16_t *d) const {
        decodeDate(m_dateTime,*y,*m,*d);
    }

    /// @brief Decodes time into h,m,s,ms
    void decodeTime(int16_t *h,int16_t *m,int16_t *s,int16_t *ms) const {
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
bool operator <  (const sptk::CDateTime &dt1, const sptk::CDateTime &dt2);

/// @brief Compares CDateTime values
bool operator <= (const sptk::CDateTime &dt1, const sptk::CDateTime &dt2);

/// @brief Compares CDateTime values
bool operator >  (const sptk::CDateTime &dt1, const sptk::CDateTime &dt2);

/// @brief Compares CDatetime values
bool operator >= (const sptk::CDateTime &dt1, const sptk::CDateTime &dt2);

/// @brief Compares CDatetime values
bool operator == (const sptk::CDateTime &dt1, const sptk::CDateTime &dt2);

/// @brief Compares CDatetime values
bool operator != (const sptk::CDateTime &dt1, const sptk::CDateTime &dt2);

/// @brief Adds two CDatetime values
sptk::CDateTime operator + (const sptk::CDateTime &dt1, const sptk::CDateTime &dt2);

/// @brief Subtracts two CDatetime values
sptk::CDateTime operator - (const sptk::CDateTime &dt1, const sptk::CDateTime &dt2);

#endif
