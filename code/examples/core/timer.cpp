/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       datetime.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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
#include "sptk5/threads/Timer.h"

using namespace std;
using namespace sptk;

void timerCallback(const char* eventData)
{
    COUT(DateTime::Now().timeString(0, DateTime::PA_MILLISECONDS) << " " << eventData << "." << endl)
}

int main()
{
    try {
        Timer timer;

        COUT(DateTime::Now().timeString(0, DateTime::PA_MILLISECONDS) << " scheduled." << endl)

        // Schedule single event
        timer.fireAt(DateTime::Now() + chrono::milliseconds(2500), bind(timerCallback, "single"));

        // Schedule repeatable event
        timer.repeat(chrono::seconds(1), bind(timerCallback, "every second"));

        // Schedule repeatable event, using event handle to cancel it later
        Timer::Event every3seconds = timer.repeat(chrono::seconds(3), bind(timerCallback, "every 3 seconds"));

        this_thread::sleep_for(chrono::seconds(5));

        // Cancelling event
        timer.cancel(every3seconds);

        this_thread::sleep_for(chrono::seconds(5));

        COUT(DateTime::Now().timeString(0, DateTime::PA_MILLISECONDS) << " Done" << endl)
    } catch (const Exception& e) {
        CERR(e.what() << endl)
        return 1;
    }

    return 0;
}
