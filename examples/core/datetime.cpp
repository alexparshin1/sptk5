/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       datetime.cpp - description                             ║
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

#include <iostream>
#include <sptk5/DateTime.h>

using namespace std;
using namespace sptk;

int main()
{
    DateTime now = DateTime::Now();
    int printFlags = DateTime::PF_TIMEZONE;

    // Print current date and time, showing timezone and milliseconds
    cout << "Current time is " << now.dateString() + " " + now.timeString(printFlags, DateTime::PA_MILLISECONDS) << endl;

    // Print same date and time as GMT, showing timezone and milliseconds
    cout << "Current GMT time is " << now.dateString() + " " + now.timeString(printFlags|DateTime::PF_GMT, DateTime::PA_MILLISECONDS) << endl;
	cout << "UTC epoch is " << chrono::duration_cast<chrono::seconds>(now.sinceEpoch()).count() << endl;
	cout << endl;

    cout << "Decode date and time in PST timezone and print it in local timezone:" << endl;
    const char*  pstDateTimeStr = "2013-10-01 10:00:00-7:00";
    DateTime    pstDateTime(pstDateTimeStr);
    cout << "From PST(-7:00): " << pstDateTimeStr
         << " to local: " << pstDateTime.isoDateTimeString() << endl;

    const char*  utcDateTimeStr = "2013-10-01T10:00:00Z";
    DateTime     utcDateTime(utcDateTimeStr);
    cout << "From UTC: " << utcDateTimeStr
         << " to local: " << utcDateTime.isoDateTimeString() << endl;

    cout << endl << "Define the date as 2003/09/28, and print the date components:" << endl;

    DateTime   dt(2003, 9, 28);
    cout << "Year:  " << dt.year() << endl;
    cout << "Month:  " << dt.month() << ", " << dt.monthName() << endl;
    cout << "Day:    " << dt.day() << ", " << dt.dayOfWeekName() << endl;
    cout << "Date:   " << dt.dateString() << endl;
    cout << "Time:   " << dt.timeString(printFlags) << endl;

    cout << endl << "Get the date and time from the system, and print the date components:" << endl << endl;
    dt = DateTime::Now();

    /// Printing the date components:
    cout << "Year:   " << dt.year() << endl;
    cout << "Month:  " << dt.month() << ", " << dt.monthName() << endl;
    cout << "Day:    " << dt.day() << ", " << dt.dayOfWeekName() << endl;
    cout << "Date:   " << dt.dateString() << endl;
    cout << "Time:   " << dt.timeString(printFlags) << endl;

    cout << endl << "Get the date and time from the system for TZ=':US/Pacific', and print the date components:" << endl << endl;
    cout << "Local TZ offset is " << DateTime::timeZoneOffset << " seconds." << endl;

#ifndef _WIN32
    DateTime::setTimeZone(":US/Pacific");
    cout << "US/Pacific TZ offset is " << DateTime::timeZoneOffset << " seconds."  << endl;

    dt = DateTime::Now();

    /// Printing the date components:
    cout << "Year:   " << dt.year() << endl;
    cout << "Month:  " << dt.month() << ", " << dt.monthName() << endl;
    cout << "Day:    " << dt.day() << ", " << dt.dayOfWeekName() << endl;
    cout << "Date:   " << dt.dateString() << endl;
    cout << "Time:   " << dt.timeString() << endl;
    cout << "TZ offset is " << DateTime::timeZoneOffset << endl;
#endif

    return 0;
}
