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
#include "sptk5/threads/Timer.h"

using namespace std;
using namespace sptk;

void timerCallback(void* eventData)
{
    cout << DateTime::Now().timeString(0, DateTime::PA_MILLISECONDS) << " " << (const char*) eventData << "." << endl;
}

int main()
{
    try {
        {
            // Timer in local scope
            Timer timer(timerCallback);

            cout << DateTime::Now().timeString(0, DateTime::PA_MILLISECONDS) << " scheduled." << endl;

            // Schedule single event
            timer.fireAt(DateTime::Now() + chrono::milliseconds(2500), (void*) "single");

            // Schedule repeatable event
            timer.repeat(chrono::seconds(1), (void*) "every second");

            // Schedule repeatable event, using event handle to cancel it later
            Timer::Event every3seconds = timer.repeat(chrono::seconds(3), (void*) "every 3 seconds");

            this_thread::sleep_for(chrono::seconds(5));

            // Cancelling event
            timer.cancel(every3seconds);

            this_thread::sleep_for(chrono::seconds(5));

            // All events, scheduled by timer should stop here
        }
        cout << DateTime::Now().timeString(0, DateTime::PA_MILLISECONDS) << " Done" << endl;
        this_thread::sleep_for(chrono::seconds(5));
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}
