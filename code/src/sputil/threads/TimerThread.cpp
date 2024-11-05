/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/threads/TimerThread.h"

using namespace std;
using namespace sptk;
using namespace chrono;

TimerThread::TimerThread()
    : Thread("TimerThread")
{
}

TimerThread::~TimerThread()
{
    TimerThread::terminate();
    Thread::join();
}

void TimerThread::threadFunction()
{
    while (!terminated())
    {
        auto event = m_scheduledEvents.next();
        if (event && event->fire())
        {
            schedule(event);
        }
    }
}

void TimerThread::terminate()
{
    Thread::terminate();
    m_scheduledEvents.wakeUp();
}

void TimerThread::schedule(const STimerEvent& event)
{
    m_scheduledEvents.add(event->when().timePoint(), event);
}

void TimerThread::clear()
{
    m_scheduledEvents.clear();
}
