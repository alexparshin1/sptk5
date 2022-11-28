/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include "BaseTimerThread.h"
#include <sptk5/cutils>
#include <sptk5/threads/Timer.h>

using namespace std;
using namespace sptk;
using namespace chrono;

class sptk::TimerThread
    : public BaseTimerThread
{
public:
    TimerThread()
        : BaseTimerThread("Timer thread")
    {
    }

    ~TimerThread() override = default;

    void schedule(const STimerEvent& event) override
    {
        scoped_lock lock(m_scheduledMutex);
        auto ticks = event->when().timePoint().time_since_epoch().count();
        auto itor = m_scheduledEvents.emplace(ticks, event);
        if (itor == m_scheduledEvents.begin())
        {
            wakeUp();
        }
    }

    void clear()
    {
        scoped_lock lock(m_scheduledMutex);
        m_scheduledEvents.clear();
    }

private:
    using EventMap = multimap<long, STimerEvent>;

    mutex m_scheduledMutex;
    EventMap m_scheduledEvents;

    STimerEvent nextEvent() override
    {
        scoped_lock lock(m_scheduledMutex);
        while (!m_scheduledEvents.empty())
        {
            auto itor = m_scheduledEvents.begin();
            if (!itor->second->cancelled())
            {
                return itor->second;
            }
            m_scheduledEvents.erase(itor);
        }

        return nullptr;
    }

    void popFrontEvent() override
    {
        scoped_lock lock(m_scheduledMutex);
        if (!m_scheduledEvents.empty())
        {
            auto itor = m_scheduledEvents.begin();
            m_scheduledEvents.erase(itor);
        }
    }
};

Timer::Timer()
    : m_timerThread(make_shared<TimerThread>())
{
    m_timerThread->run();
}

Timer::~Timer()
{
    cancel();
}

STimerEvent Timer::fireAt(const DateTime& timestamp, const TimerEvent::Callback& eventCallback) const
{
    auto event = make_shared<TimerEvent>(timestamp, eventCallback, milliseconds(), 0);
    m_timerThread->schedule(event);

    return event;
}

STimerEvent Timer::repeat(milliseconds interval, const TimerEvent::Callback& eventCallback, int repeatCount) const
{
    auto event = make_shared<TimerEvent>(DateTime::Now() + interval, eventCallback, interval, repeatCount);
    m_timerThread->schedule(event);

    return event;
}

void Timer::cancel() const
{
    m_timerThread->clear();
}
