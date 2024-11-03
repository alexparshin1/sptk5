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

STimerEvent TimerThread::waitForEvent()
{
    STimerEvent event = nextEvent();
    if (!event)
    {
        m_semaphore.wait_for(100ms);
        return nullptr;
    }

    if (m_semaphore.wait_until(event->when()))
    {
        return nullptr; // Wait interrupted
    }

    popFrontEvent();
    return event;
}

void TimerThread::threadFunction()
{
    while (!terminated())
    {
        auto event = waitForEvent();
        if (event && event->fire())
        {
            schedule(event);
        }
    }
}

void TimerThread::terminate()
{
    Thread::terminate();
    m_semaphore.post();
}

void TimerThread::wakeUp()
{
    m_semaphore.post();
}

void TimerThread::schedule(const STimerEvent& event)
{
    const auto        ticks = event->when().timePoint();
    const scoped_lock lock(m_scheduledMutex);
    bool isFront {false};
    m_scheduledEvents.add(ticks, event, isFront);
    if (isFront)
    {
        wakeUp();
    }
}

void TimerThread::clear()
{
    m_scheduledEvents.clear();
}

STimerEvent TimerThread::nextEvent()
{
    return m_scheduledEvents.front();
}

void TimerThread::popFrontEvent()
{
    m_scheduledEvents.popFront();
}
