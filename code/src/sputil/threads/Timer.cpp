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
        m_scheduledEvents.try_emplace(event->getId(), event);
        m_semaphore.post();
    }

    bool waitForEvent(Timer::Event& event)
    {
        if (DateTime when = nextWakeUp();
            m_semaphore.sleep_until(when))
        {
            return false; // Wait interrupted
        }

        return popFrontEvent(event);
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
    using EventMap = map<Timer::EventId, Timer::Event, EventIdComparator>;

    mutex m_scheduledMutex;
    EventMap m_scheduledEvents;
    Semaphore m_semaphore;

    DateTime nextWakeUp()
    {
        scoped_lock lock(m_scheduledMutex);
        if (m_scheduledEvents.empty())
        {
            return DateTime::Now() + seconds(1);
        }

        auto itor = m_scheduledEvents.begin();
        return itor->first.when;
    }

    bool popFrontEvent(Timer::Event& event)
    {
        scoped_lock lock(m_scheduledMutex);
        if (m_scheduledEvents.empty())
        {
            return false;
        }

        auto itor = m_scheduledEvents.begin();
        event = itor->second;
        m_scheduledEvents.erase(itor);
        return true;
    }
};

mutex Timer::timerThreadMutex;
shared_ptr<TimerThread> Timer::timerThread;

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
        Timer::Event event;
        if (waitForEvent(event) && event->fire())
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

Timer::~Timer()
{
    cancel();
}

void Timer::unlink(const Event& event)
{
    scoped_lock lock(m_mutex);
    m_events.erase(event);
}

void Timer::checkTimerThreadRunning()
{
    scoped_lock lock(timerThreadMutex);
    if (timerThread == nullptr)
    {
        timerThread = make_shared<TimerThread>();
        timerThread->run();
    }
}

Timer::Event Timer::fireAt(const DateTime& timestamp, const EventData::Callback& eventCallback)
{
    checkTimerThreadRunning();

    auto event = make_shared<EventData>(timestamp, eventCallback, milliseconds(), 0);
    timerThread->schedule(event);

    scoped_lock lock(m_mutex);
    m_events.insert(event);

    return event;
}

Timer::Event Timer::repeat(milliseconds interval, const EventData::Callback& eventCallback, int repeatCount)
{
    checkTimerThreadRunning();

    auto event = make_shared<EventData>(DateTime::Now() + interval, eventCallback, interval, repeatCount);
    timerThread->schedule(event);

    scoped_lock lock(m_mutex);
    m_events.insert(event);

    return event;
}

void Timer::cancel(const Event& event)
{
    scoped_lock lock(m_mutex);
    if (event)
    {
        timerThread->forget(event);
        m_events.erase(event);
    }
}

set<Timer::Event> Timer::moveOutEvents()
{
    set<Timer::Event> events;

    // Cancel all events in this timer
    scoped_lock lock(m_mutex);
    events = move(m_events);

    return events;
}

void Timer::cancel()
{
    set<Timer::Event> events = moveOutEvents();

    // Unregister and destroy events
    for (const auto& event: events)
    {
        timerThread->forget(event);
    }
}

