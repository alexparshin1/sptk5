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

#include <sptk5/cutils>
#include <sptk5/threads/Timer.h>

using namespace std;
using namespace sptk;
using namespace chrono;

class EventIdComparator
{
public:
    bool operator()(const Timer::EventId& a, const Timer::EventId& b) const
    {
        if (a.when < b.when)
        {
            return true;
        }
        if (a.when > b.when)
        {
            return false;
        }
        return a.serial < b.serial;
    }
};

class sptk::TimerThread
    : public Thread
{
public:
    void terminate() override;

    TimerThread()
        : Thread("Timer thread")
    {
    }

    ~TimerThread() override
    {
        m_semaphore.post();
    }

    void schedule(Timer::Event& event)
    {
        scoped_lock lock(m_scheduledMutex);
        m_scheduledEvents.emplace(event->getId(), event);
        m_semaphore.post();
    }

    Timer::Event waitForEvent()
    {
        auto event = nextWakeUp();

        if (!event)
        {
            m_semaphore.sleep_for(chrono::milliseconds(500));
            return nullptr;
        }

        if (m_semaphore.sleep_until(event->when()))
        {
            return nullptr; // Wait interrupted
        }

        popFrontEvent();

        return event;
    }

    void clear()
    {
        scoped_lock lock(m_scheduledMutex);
        m_scheduledEvents.clear();
    }

    void forget(const Timer::Event& event)
    {
        scoped_lock lock(m_scheduledMutex);
        m_scheduledEvents.erase(event->getId());
    }

    void forget(const set<Timer::Event>& events)
    {
        scoped_lock lock(m_scheduledMutex);
        for (const auto& event: events)
        {
            m_scheduledEvents.erase(event->getId());
        }
    }

protected:
    void threadFunction() override;

private:
    using EventMap = multimap<Timer::EventId, Timer::Event, EventIdComparator>;

    mutex m_scheduledMutex;
    EventMap m_scheduledEvents;
    Semaphore m_semaphore;

    Timer::Event nextWakeUp()
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

    void popFrontEvent()
    {
        scoped_lock lock(m_scheduledMutex);
        if (!m_scheduledEvents.empty())
        {
            auto itor = m_scheduledEvents.begin();
            m_scheduledEvents.erase(itor);
        }
    }
};

atomic<uint64_t> Timer::nextSerial;

Timer::EventId::EventId(const DateTime& when)
    : serial(++nextSerial)
    , when(when)
{
}

Timer::EventData::EventData(const DateTime& timestamp, const Callback& eventCallback, milliseconds repeatEvery,
                            int repeatCount)
    : m_id(timestamp)
    , m_callback(eventCallback)
    , m_repeatInterval(repeatEvery)
    , m_repeatCount(repeatCount)
{
}

const Timer::EventId& Timer::EventData::getId() const
{
    return m_id;
}

bool Timer::EventData::fire()
{
    try
    {
        m_callback();
    }
    catch (const Exception& e)
    {
        CERR(e.what())
    }

    if (m_repeatCount == 0)
    {
        return false;
    }

    if (m_repeatCount > 0)
    {
        --m_repeatCount;
        if (m_repeatCount == 0)
        {
            return false;
        }
    }

    m_id.when = m_id.when + m_repeatInterval;

    return true;
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
    clear();
}

void TimerThread::terminate()
{
    m_semaphore.post();
    Thread::terminate();
}

Timer::Timer()
    : m_timerThread(make_shared<TimerThread>())
{
    m_timerThread->run();
}

Timer::~Timer()
{
    cancel();
}

void Timer::checkTimerThreadRunning()
{
}

Timer::Event Timer::fireAt(const DateTime& timestamp, const EventData::Callback& eventCallback)
{
    checkTimerThreadRunning();

    auto event = make_shared<EventData>(timestamp, eventCallback, milliseconds(), 0);
    m_timerThread->schedule(event);

    return event;
}

Timer::Event Timer::repeat(milliseconds interval, const EventData::Callback& eventCallback, int repeatCount)
{
    checkTimerThreadRunning();

    auto event = make_shared<EventData>(DateTime::Now() + interval, eventCallback, interval, repeatCount);
    m_timerThread->schedule(event);

    return event;
}

void Timer::cancel()
{
    m_timerThread->clear();
}
