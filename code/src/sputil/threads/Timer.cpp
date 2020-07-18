/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/threads/Timer.h>
#include <sptk5/cutils>
#include <sptk5/StopWatch.h>

using namespace std;
using namespace sptk;
using namespace chrono;

class EventIdComparator
{
public:
    bool operator()(const Timer::EventId& a, const Timer::EventId& b) const
    {
        if (a.when < b.when)
            return true;
        if (a.when > b.when)
            return false;
        return a.serial < b.serial;
    }
};

class TimerThread : public Thread
{
public:
    void terminate() override;

    TimerThread()
    : Thread("Timer thread")
    {}

    void schedule(Timer::Event event)
    {
        lock_guard<mutex> lock(m_scheduledMutex);
        m_scheduledEvents.insert(pair<Timer::EventId,Timer::Event>(event->getId(), event));
        m_semaphore.post();
    }

    bool waitForEvent(Timer::Event& event)
    {
        DateTime when = nextWakeUp();

        if (m_semaphore.sleep_until(when))
            return false; // Wait interrupted

        return popFrontEvent(event);
    }

    void clear()
    {
        lock_guard<mutex> lock(m_scheduledMutex);
        m_scheduledEvents.clear();
    }

    void forget(Timer::Event event)
    {
        lock_guard<mutex> lock(m_scheduledMutex);
        m_scheduledEvents.erase(event->getId());
    }

    void forget(const set<Timer::Event>& events)
    {
        lock_guard<mutex> lock(m_scheduledMutex);
        for (auto event: events)
            m_scheduledEvents.erase(event->getId());
    }

protected:

    void threadFunction() override;

private:

    typedef map<Timer::EventId, Timer::Event, EventIdComparator> EventMap;

    mutex       m_scheduledMutex;
    EventMap    m_scheduledEvents;
    Semaphore   m_semaphore;

    DateTime nextWakeUp()
    {
        lock_guard<mutex> lock(m_scheduledMutex);
        if (m_scheduledEvents.empty())
            return DateTime::Now() + seconds(1);
        else {
            auto itor = m_scheduledEvents.begin();
            return itor->first.when;
        }
    }

    bool popFrontEvent(Timer::Event& event)
    {
        lock_guard<mutex> lock(m_scheduledMutex);
        if (m_scheduledEvents.empty())
            return false;
        else {
            auto itor = m_scheduledEvents.begin();
            event = itor->second;
            m_scheduledEvents.erase(itor);
            return true;
        }
    }
};

static mutex                timerThreadMutex;
static TimerThread*         timerThread;

atomic<uint64_t>            Timer::nextSerial;

int                         eventAllocations;

Timer::EventId::EventId(const DateTime& when)
: serial(++nextSerial), when(when)
{
}

Timer::EventData::EventData(const DateTime& timestamp, const Callback& eventCallback, milliseconds repeatEvery, int repeatCount)
: m_id(timestamp), m_callback(eventCallback), m_repeatInterval(repeatEvery), m_repeatCount(repeatCount)
{
    ++eventAllocations;
}

Timer::EventData::~EventData()
{
    --eventAllocations;
}

const Timer::EventId& Timer::EventData::getId() const
{
    return m_id;
}

bool Timer::EventData::fire()
{
    try {
        m_callback();
    }
    catch (const Exception& e) {
        CERR(e.what())
    }

    if (m_repeatCount == 0)
        return false;

    if (m_repeatCount > 0) {
        --m_repeatCount;
        if (m_repeatCount == 0)
            return false;
    }

    m_id.when = m_id.when + m_repeatInterval;

    return true;
}

void TimerThread::threadFunction()
{
    while (!terminated()) {
        Timer::Event event;
        if (waitForEvent(event) && event->fire())
            schedule(event);
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

void Timer::unlink(Timer::Event event)
{
    lock_guard<mutex> lock(m_mutex);
    m_events.erase(event);
}

static void checkTimerThreadRunning()
{
    lock_guard<mutex> lock(timerThreadMutex);
    if (timerThread == nullptr) {
        timerThread = new TimerThread();
        timerThread->run();
    }
}

Timer::Event Timer::fireAt(const DateTime& timestamp, const EventData::Callback& eventCallback)
{
    checkTimerThreadRunning();

    Event event = make_shared<EventData>(timestamp, eventCallback, milliseconds(), 0);
    timerThread->schedule(event);

    lock_guard<mutex> lock(m_mutex);
    m_events.insert(event);

    return event;
}

Timer::Event Timer::repeat(milliseconds interval, const EventData::Callback& eventCallback, int repeatCount)
{
    checkTimerThreadRunning();

    Event event = make_shared<EventData>(DateTime::Now() + interval, eventCallback, interval, repeatCount);
    timerThread->schedule(event);

    lock_guard<mutex> lock(m_mutex);
    m_events.insert(event);

    return event;
}

void Timer::cancel(Event event)
{
    lock_guard<mutex> lock(m_mutex);
    if (event) {
        timerThread->forget(event);
        m_events.erase(event);
    }
}

set<Timer::Event> Timer::moveOutEvents()
{
    set<Timer::Event> events;

    // Cancel all events in this timer
    lock_guard<mutex> lock(m_mutex);
    events = move(m_events);

    return events;
}

void Timer::cancel()
{
    set<Timer::Event> events = moveOutEvents();

    // Unregister and destroy events
    for (auto event: events)
        timerThread->forget(event);
}

#if USE_GTEST

static void gtestTimerCallback(void* eventData)
{
    int& eventSet = *(int *) eventData;
    ++eventSet;
}

TEST(SPTK_Timer, repeat)
{
    if (DateTime::Now() > DateTime()) // always true
    {
        Timer timer;

        int eventSet(0);

        function<void()> eventCallback = bind(gtestTimerCallback, &eventSet);
        Timer::Event handle = timer.repeat(milliseconds(20), eventCallback);
        this_thread::sleep_for(milliseconds(105));
        timer.cancel(handle);

        EXPECT_NEAR(5, eventSet, 1);
    }
    EXPECT_EQ(0, eventAllocations);
}


#define MAX_EVENT_COUNTER (10)
#define MAX_TIMERS        (10)
static mutex            eventCounterMutex;
static vector<size_t>   eventCounter(MAX_EVENT_COUNTER);
static vector<size_t>   eventData(MAX_EVENT_COUNTER);

static void gtestTimerCallback2(void* theEventData)
{
    lock_guard<mutex> lock(eventCounterMutex);
    size_t eventIndex = size_t(theEventData);
    ++eventCounter[eventIndex];
}

TEST(SPTK_Timer, fireOnce)
{
    mutex   counterMutex;
    size_t  counter = 1;
    Timer   timer;

    timer.fireAt(
        DateTime::Now() + milliseconds(10),
        [&counter, &counterMutex]() {
            lock_guard<mutex> lock(counterMutex);
            ++counter;
        }
    );

    this_thread::sleep_for(milliseconds(20));

    lock_guard<mutex> lock(counterMutex);
    EXPECT_EQ(counter, size_t(2));
}

TEST(SPTK_Timer, repeatTwice)
{
    mutex   counterMutex;
    size_t  counter = 0;
    Timer   timer;

    timer.repeat(
            milliseconds(10),
            [&counter, &counterMutex]() {
                lock_guard<mutex> lock(counterMutex);
                ++counter;
            },
            2
    );

    this_thread::sleep_for(milliseconds(40));

    lock_guard<mutex> lock(counterMutex);
    EXPECT_EQ(counter, size_t(2));
}

TEST(SPTK_Timer, repeatMultipleEvents)
{
    if (DateTime::Now() > DateTime()) // always true
    {
        Timer timer;

        vector<Timer::Event> createdEvents;
        for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex) {
            eventData[eventIndex] = eventIndex;
            function<void()> callback = bind(gtestTimerCallback2, (void*)eventIndex);
            Timer::Event event = timer.repeat(milliseconds(20), callback);
            createdEvents.push_back(event);
        }

        this_thread::sleep_for(milliseconds(110));

        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex) {
            Timer::Event event = createdEvents[eventIndex];
            timer.cancel(event);
        }

        this_thread::sleep_for(milliseconds(20));

        int totalEvents(0);
        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex) {
            lock_guard<mutex> lock(eventCounterMutex);
            totalEvents += eventCounter[eventIndex];
        }

        EXPECT_NEAR(MAX_EVENT_COUNTER * 5, totalEvents, 10);
    }
    EXPECT_EQ(0, eventAllocations);
}

TEST(SPTK_Timer, repeatMultipleTimers)
{
    int repeatCount = 10;
    vector<Timer> timers(MAX_TIMERS);

    if (!timers.empty()) {
        lock_guard<mutex> lock(eventCounterMutex);
        eventCounter.clear();
        eventCounter.resize(MAX_EVENT_COUNTER);
    }

    for (auto& timer: timers) {
        for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; ++eventIndex) {
            function<void()> callback = bind(gtestTimerCallback2, (void*)eventIndex);
            timer.repeat(
                    milliseconds(10),
                    callback,
                    repeatCount);
        }
    }

    this_thread::sleep_for(milliseconds(300));

    int totalEvents(0);
    if (!timers.empty()) {
        lock_guard<mutex> lock(eventCounterMutex);
        for (auto counter: eventCounter)
            totalEvents += counter;
    }

    EXPECT_EQ(MAX_TIMERS * MAX_EVENT_COUNTER * repeatCount, totalEvents);
}

class NotifyObject
{
    mutable mutex   m_mutex;
    int             m_value {0};
public:
    int  getValue() const
    {
        lock_guard<mutex> lock(m_mutex);
        return m_value;
    }
    void setValue(int v)
    {
        lock_guard<mutex> lock(m_mutex);
        m_value = v;
    }
};

TEST(SPTK_Timer, notifyObjects)
{
    NotifyObject    object1;
    NotifyObject    object2;
    Timer           timer;

    function<void()> object1function = bind(&NotifyObject::setValue, &object1, 1);
    function<void()> object2function = bind(&NotifyObject::setValue, &object2, -1);

    timer.fireAt(
            DateTime::Now() + milliseconds(10),
            object1function
    );
    timer.fireAt(
            DateTime::Now() + milliseconds(10),
            object2function
    );
    this_thread::sleep_for(milliseconds(30));
    EXPECT_EQ(object1.getValue(), 1);
    EXPECT_EQ(object2.getValue(), -1);
}

TEST(SPTK_Timer, scheduleEventsPerformance)
{
    Timer   timer;
    size_t  maxEvents = 100000;
    vector<Timer::Event> createdEvents;

    StopWatch   stopwatch;

    DateTime    when("now");
    when = when + hours(1);

    stopwatch.start();
    for (size_t eventIndex = 0; eventIndex < maxEvents; ++eventIndex) {
        function<void()> callback = bind(gtestTimerCallback2, (void*)eventIndex);
        Timer::Event event = timer.fireAt(when, callback);
        createdEvents.push_back(event);
    }
    stopwatch.stop();

    COUT(maxEvents << " events scheduled, " << maxEvents / stopwatch.seconds() << " events/s" << endl)

    stopwatch.start();
    for (const auto& event: createdEvents) {
        timer.cancel(event);
    }
    stopwatch.stop();

    COUT(maxEvents << " events canceled, " << maxEvents / stopwatch.seconds() << " events/s" << endl)
}

#endif
