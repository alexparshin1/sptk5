/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       datetime.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cutils>

using namespace std;
using namespace sptk;

int main()
{
    DateTime now = DateTime::Now();
    int printFlags = DateTime::PF_TIMEZONE;

    // Print current date and time, showing timezone and milliseconds
    COUT("Current time is " << now.dateString() + " " + now.timeString(printFlags, DateTime::PrintAccuracy::MILLISECONDS) << endl)

    // Print same date and time as GMT, showing timezone and milliseconds
    COUT("Current GMT time is " << now.dateString() + " " + now.timeString(printFlags | DateTime::PF_GMT, DateTime::PrintAccuracy::MILLISECONDS) << endl)
    COUT("UTC epoch is " << chrono::duration_cast<chrono::seconds>(now.sinceEpoch()).count() << endl)
    COUT(endl)

    COUT("Decode date and time in PST timezone and print it in local timezone:" << endl)
    const char* pstDateTimeStr = "2013-10-01 10:00:00-7:00";
    DateTime pstDateTime(pstDateTimeStr);
    COUT("From PST(-7:00): " << pstDateTimeStr << " to local: " << pstDateTime.isoDateTimeString() << endl)

    const char* utcDateTimeStr = "2013-10-01T10:00:00Z";
    DateTime utcDateTime(utcDateTimeStr);
    COUT("From UTC: " << utcDateTimeStr << " to local: " << utcDateTime.isoDateTimeString() << endl)

    COUT(endl
         << "Define the date as 2003/09/28, and print the date components:" << endl)

    short year;
    short month;
    short day;
    short wday;
    short yday;
    DateTime dt(2003, 9, 28);
    dt.decodeDate(&year, &month, &day, &wday, &yday);
    COUT("Year:  " << year << endl)
    COUT("Month:  " << month << ", " << dt.monthName() << endl)
    COUT("Day:    " << day << ", " << dt.dayOfWeekName() << endl)
    COUT("Date:   " << dt.dateString() << endl)
    COUT("Time:   " << dt.timeString(printFlags) << endl)

    COUT(endl
         << "Get the date and time from the system, and print the date components:" << endl
         << endl)
    dt = DateTime::Now();
    dt.decodeDate(&year, &month, &day, &wday, &yday);

    /// Printing the date components:
    COUT("Year:   " << year << endl)
    COUT("Month:  " << month << ", " << dt.monthName() << endl)
    COUT("Day:    " << day << ", " << dt.dayOfWeekName() << endl)
    COUT("Date:   " << dt.dateString() << endl)
    COUT("Time:   " << dt.timeString(printFlags) << endl)

    COUT(endl
         << "Get the date and time from the system for TZ=':US/Pacific', and print the date components:" << endl
         << endl)
    COUT("Local TZ offset is " << TimeZone::offset().count() << " minutes." << endl)

#ifndef _WIN32
    TimeZone::set(":US/Pacific");
    COUT("US/Pacific TZ offset is " << TimeZone::offset().count() << " minutes." << endl)

    dt = DateTime::Now();
    dt.decodeDate(&year, &month, &day, &wday, &yday);

    /// Printing the date components:
    COUT("Year:   " << year << endl)
    COUT("Month:  " << month << ", " << dt.monthName() << endl)
    COUT("Day:    " << day << ", " << dt.dayOfWeekName() << endl)
    COUT("Date:   " << dt.dateString() << endl)
    COUT("Time:   " << dt.timeString() << endl)
    COUT("TZ offset is " << TimeZone::offset().count() << " minutes" << endl)
#endif

    return 0;
}
