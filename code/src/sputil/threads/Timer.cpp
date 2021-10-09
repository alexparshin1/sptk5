/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
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
        else
        {
            auto itor = m_scheduledEvents.begin();
            return itor->first.when;
        }
    }

    bool popFrontEvent(Timer::Event& event)
    {
        scoped_lock lock(m_scheduledMutex);
        if (m_scheduledEvents.empty())
        {
            return false;
        }
        else
        {
            auto itor = m_scheduledEvents.begin();
            event = itor->second;
            m_scheduledEvents.erase(itor);
            return true;
        }
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
    for (auto event: events)
    {
        timerThread->forget(event);
    }
}

#ifdef USE_GTEST

TEST(SPTK_Timer, repeat)
{
    if (DateTime::Now() > DateTime()) // always true
    {
        Timer timer;

        int eventSet(0);

        Timer::Event handle = timer.repeat(milliseconds(20),
                                           [&eventSet]() {
                                               ++eventSet;
                                           });

        this_thread::sleep_for(milliseconds(105));
        timer.cancel(handle);

        EXPECT_NEAR(5, eventSet, 2);
    }
}

const int MAX_EVENT_COUNTER = 10;
const int MAX_TIMERS = 10;

class TimerTestData
{
public:
    static mutex eventCounterMutex;
    static vector<size_t> eventCounter;
    static vector<size_t> eventData;
};

mutex TimerTestData::eventCounterMutex;
vector<size_t> TimerTestData::eventCounter(MAX_EVENT_COUNTER);
vector<size_t> TimerTestData::eventData(MAX_EVENT_COUNTER);

static void gtestTimerCallback2(const uint8_t* theEventData)
{
    scoped_lock lock(TimerTestData::eventCounterMutex);
    auto eventIndex = size_t(theEventData);
    ++TimerTestData::eventCounter[eventIndex];
}

TEST(SPTK_Timer, fireOnce)
{
    mutex counterMutex;
    size_t counter = 1;
    Timer timer;

    timer.fireAt(
        DateTime::Now() + milliseconds(10),
        [&counter, &counterMutex]() {
            scoped_lock lock(counterMutex);
            ++counter;
        });

    this_thread::sleep_for(milliseconds(20));

    scoped_lock lock(counterMutex);
    EXPECT_EQ(counter, size_t(2));
}

TEST(SPTK_Timer, repeatTwice)
{
    mutex counterMutex;
    size_t counter = 0;
    Timer timer;

    timer.repeat(
        milliseconds(10),
        [&counter, &counterMutex]() {
            scoped_lock lock(counterMutex);
            ++counter;
        },
        2);

    this_thread::sleep_for(milliseconds(40));

    scoped_lock lock(counterMutex);
    EXPECT_EQ(counter, size_t(2));
}

TEST(SPTK_Timer, repeatMultipleEvents)
{
    if (DateTime::Now() > DateTime()) // always true
    {
        Timer timer;

        vector<Timer::Event> createdEvents;
        for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            TimerTestData::eventData[eventIndex] = eventIndex;
            function<void()> callback = bind(gtestTimerCallback2, (uint8_t*) eventIndex);
            Timer::Event event = timer.repeat(milliseconds(20), callback);
            createdEvents.push_back(event);
        }

        this_thread::sleep_for(milliseconds(110));

        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            Timer::Event event = createdEvents[eventIndex];
            timer.cancel(event);
        }

        this_thread::sleep_for(milliseconds(20));

        int totalEvents(0);
        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            scoped_lock lock(TimerTestData::eventCounterMutex);
            totalEvents += (int) TimerTestData::eventCounter[eventIndex];
        }

        EXPECT_NEAR(MAX_EVENT_COUNTER * 5, totalEvents, 10);
    }
}

TEST(SPTK_Timer, repeatMultipleTimers)
{
    int repeatCount = 10;
    vector<Timer> timers(MAX_TIMERS);

    if (!timers.empty())
    {
        scoped_lock lock(TimerTestData::eventCounterMutex);
        TimerTestData::eventCounter.clear();
        TimerTestData::eventCounter.resize(MAX_EVENT_COUNTER);
    }

    for (auto& timer: timers)
    {
        for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex)
        {
            function<void()> callback = bind(gtestTimerCallback2, (uint8_t*) eventIndex);
            timer.repeat(
                milliseconds(10),
                callback,
                repeatCount);
        }
    }

    this_thread::sleep_for(milliseconds(300));

    int totalEvents(0);
    if (!timers.empty())
    {
        scoped_lock lock(TimerTestData::eventCounterMutex);
        for (auto counter: TimerTestData::eventCounter)
        {
            totalEvents += (int) counter;
        }
    }

    EXPECT_EQ(MAX_TIMERS * MAX_EVENT_COUNTER * repeatCount, totalEvents);
}

class NotifyObject
{
    mutable mutex m_mutex;
    int m_value {0};

public:
    int getValue() const
    {
        scoped_lock lock(m_mutex);
        return m_value;
    }

    void setValue(int v)
    {
        scoped_lock lock(m_mutex);
        m_value = v;
    }
};

TEST(SPTK_Timer, notifyObjects)
{
    NotifyObject object1;
    NotifyObject object2;
    Timer timer;

    function<void()> object1function = bind(&NotifyObject::setValue, &object1, 1);
    function<void()> object2function = bind(&NotifyObject::setValue, &object2, -1);

    timer.fireAt(
        DateTime::Now() + milliseconds(10),
        object1function);
    timer.fireAt(
        DateTime::Now() + milliseconds(10),
        object2function);
    this_thread::sleep_for(milliseconds(30));
    EXPECT_EQ(object1.getValue(), 1);
    EXPECT_EQ(object2.getValue(), -1);
}

TEST(SPTK_Timer, scheduleEventsPerformance)
{
    Timer timer;
    size_t maxEvents = 100000;
    vector<Timer::Event> createdEvents;

    StopWatch stopwatch;

    DateTime when("now");
    when = when + hours(1);

    stopwatch.start();
    for (size_t eventIndex = 0; eventIndex < maxEvents; ++eventIndex)
    {
        function<void()> callback = bind(gtestTimerCallback2, (uint8_t*) eventIndex);
        Timer::Event event = timer.fireAt(when, callback);
        createdEvents.push_back(event);
    }
    stopwatch.stop();

    COUT(maxEvents << " events scheduled, " << maxEvents / stopwatch.seconds() << " events/s" << endl)

    stopwatch.start();
    for (const auto& event: createdEvents)
    {
        timer.cancel(event);
    }
    stopwatch.stop();

    COUT(maxEvents << " events canceled, " << maxEvents / stopwatch.milliseconds() / 1000.0 << " events/s" << endl)
}

#endif
