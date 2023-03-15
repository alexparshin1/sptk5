/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include "IntervalTimerThread.h"

using namespace std;
using namespace sptk;
using namespace chrono;

IntervalTimerThread::IntervalTimerThread()
    : Thread("IntervalTimerThread")
{
}

IntervalTimerThread::~IntervalTimerThread()
{
    m_semaphore.post();
}

STimerEvent IntervalTimerThread::waitForEvent()
{
    STimerEvent event = nextEvent();
    if (!event)
    {
        m_semaphore.sleep_for(chrono::seconds(1));
        return nullptr;
    }

    if (m_semaphore.sleep_until(event->when()))
    {
        return nullptr; // Wait interrupted
    }

    popFrontEvent();
    return event;
}

void IntervalTimerThread::threadFunction()
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

void IntervalTimerThread::terminate()
{
    m_semaphore.post();
    Thread::terminate();
}

void IntervalTimerThread::wakeUp()
{
    m_semaphore.post();
}

void IntervalTimerThread::schedule(const STimerEvent& event)
{
    const scoped_lock lock(m_scheduledMutex);
    m_scheduledEvents.emplace(event);
    if (m_scheduledEvents.size() == 1)
    {
        wakeUp();
    }
}

STimerEvent IntervalTimerThread::nextEvent()
{
    const scoped_lock lock(m_scheduledMutex);

    while (!m_scheduledEvents.empty())
    {
        if (const auto& event = m_scheduledEvents.front();
            !event->cancelled())
        {
            return event;
        }
        m_scheduledEvents.pop();
    }

    return nullptr;
}

void IntervalTimerThread::popFrontEvent()
{
    STimerEvent event;

    const scoped_lock lock(m_scheduledMutex);
    if (m_scheduledEvents.empty())
    {
        return;
    }

    m_scheduledEvents.pop();
}
