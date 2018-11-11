
#include <sptk5/threads/Timer.h>

#include "sptk5/threads/Timer.h"

using namespace std;
using namespace sptk;

struct EventIdComparator
{
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
    typedef map<Timer::EventId, Timer::Event, EventIdComparator> EventMap;

    mutex       m_scheduledMutex;
    EventMap    m_scheduledEvents;
    Semaphore   m_semaphore;
protected:
    void threadFunction() override;

public:
    void terminate() override;

public:
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
        DateTime        when;
        {
            lock_guard<mutex> lock(m_scheduledMutex);
            if (m_scheduledEvents.empty()) {
                when = DateTime::Now() + chrono::seconds(1);
            } else {
                auto itor = m_scheduledEvents.begin();
                when = itor->first.when;
            }
        }

        if (m_semaphore.sleep_until(when)) {
            // Wait interrupted
            return false;
        }

        {
            lock_guard<mutex> lock(m_scheduledMutex);
            if (m_scheduledEvents.empty()) {
                return false;
            } else {
                auto itor = m_scheduledEvents.begin();
                event = itor->second;
                m_scheduledEvents.erase(itor);
            }
        }

        return true;
    }

    void clear()
    {
        lock_guard<mutex> lock(m_scheduledMutex);
        while (!m_scheduledEvents.empty()) {
            auto itor = m_scheduledEvents.begin();
            Timer::Event event = itor->second;
            event->unlinkFromTimer();
            m_scheduledEvents.erase(itor);
        }
    }

    void forget(Timer::Event event)
    {
        lock_guard<mutex> lock(m_scheduledMutex);
        m_scheduledEvents.erase(event->getId());
    }

    void forget(set<Timer::Event>& events)
    {
        lock_guard<mutex> lock(m_scheduledMutex);
        for (auto event: events)
            m_scheduledEvents.erase(event->getId());
    }
};

static mutex                timerThreadMutex;
static TimerThread*         timerThread;
static atomic<uint64_t>     nextSerial;

int                         eventAllocations;

Timer::EventId::EventId(const DateTime& when)
: serial(nextSerial++), when(when)
{
}

Timer::EventData::EventData(Timer& timer, const DateTime& timestamp, void* eventData, std::chrono::milliseconds repeatEvery)
: m_id(timestamp), m_data(eventData), m_repeatEvery(repeatEvery), m_timer(&timer)
{
    eventAllocations++;
}

Timer::EventData::~EventData()
{
    eventAllocations--;
}

const Timer::EventId& Timer::EventData::getId() const
{
    return m_id;
}

void TimerThread::threadFunction()
{
    while (!terminated()) {
        Timer::Event event;
        if (waitForEvent(event)) {
            event->getTimer().fire(event);
            if (event->getInterval().count() == 0)
                event->unlinkFromTimer();
            else {
                event->shift(event->getInterval());
                schedule(event);
            }
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

void Timer::unlink(Timer::Event event)
{
    lock_guard<mutex> lock(m_mutex);
    m_events.erase(event);
}

Timer::Event Timer::fireAt(const DateTime& timestamp, void* eventData)
{
    {
        lock_guard<mutex> lock(timerThreadMutex);
        if (timerThread == nullptr) {
            timerThread = new TimerThread();
            timerThread->run();
        }
    }

    Event event = make_shared<EventData>(*this, timestamp, eventData, chrono::milliseconds());
    timerThread->schedule(event);

    lock_guard<mutex> lock(m_mutex);
    m_events.insert(event);

    return event;
}

Timer::Event Timer::repeat(std::chrono::milliseconds interval, void* eventData)
{
    {
        lock_guard<mutex> lock(timerThreadMutex);
        if (timerThread == nullptr) {
            timerThread = new TimerThread();
            timerThread->run();
        }
    }

    Event event = make_shared<EventData>(*this, DateTime::Now() + interval, eventData, interval);
    timerThread->schedule(event);

    lock_guard<mutex> lock(m_mutex);
    m_events.insert(event);

    return event;
}

void Timer::fire(Timer::Event event)
{
    m_callback(event->getData());
}

void Timer::cancel(Event event)
{
    lock_guard<mutex> lock(m_mutex);
    timerThread->forget(event);
    m_events.erase(event);
}

void Timer::cancel()
{
    set<Timer::Event> events;

    // Cancel all events in this timer
    {
        lock_guard<mutex> lock(m_mutex);
        events = move(m_events);
    }

    // Unregister and destroy events
    for (auto event: events) {
        timerThread->forget(event);
        event->m_timer = nullptr;
    }
}

#if USE_GTEST
#include <gtest/gtest.h>

static void gtestTimerCallback(void* eventData)
{
    int& eventSet = *(int *) eventData;
    eventSet++;
}

TEST(SPTK_Timer, fireAt)
{
    {
        Timer timer(gtestTimerCallback);
        DateTime when = DateTime::Now() + chrono::milliseconds(50);

        int eventSet(0);

        timer.fireAt(when, &eventSet);
        this_thread::sleep_until((when + chrono::milliseconds(20)).timePoint());

        EXPECT_EQ(1, eventSet);
    }
    EXPECT_EQ(0, eventAllocations);
}

TEST(SPTK_Timer, repeat)
{
    {
        Timer timer(gtestTimerCallback);

        int eventSet(0);

        Timer::Event handle = timer.repeat(chrono::milliseconds(20), &eventSet);
        this_thread::sleep_for(chrono::milliseconds(110));
        timer.cancel(handle);

        EXPECT_NEAR(5, eventSet, 1);
    }
    EXPECT_EQ(0, eventAllocations);
}


#define MAX_EVENT_COUNTER (10)
#define MAX_TIMERS        (10)
SharedMutex eventCounterMutex;
vector<int> eventCounter(MAX_EVENT_COUNTER);
vector<int> eventData(MAX_EVENT_COUNTER);

static void gtestTimerCallback2(void* eventData)
{
    UniqueLock(eventCounterMutex);
    size_t eventIndex = size_t(eventData);
    eventCounter[eventIndex]++;
}

TEST(SPTK_Timer, repeat_multiple_events)
{
    {
        Timer timer(gtestTimerCallback2);

        vector<Timer::Event> createdEvents;
        for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; eventIndex++) {
            eventData[eventIndex] = eventIndex;
            Timer::Event event = timer.repeat(chrono::milliseconds(20), (void*)eventIndex);
            createdEvents.push_back(event);
        }

        this_thread::sleep_for(chrono::milliseconds(110));

        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; eventIndex++) {
            Timer::Event event = createdEvents[eventIndex];
            timer.cancel(event);
        }

        this_thread::sleep_for(chrono::milliseconds(20));

        int totalEvents(0);
        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; eventIndex++) {
            UniqueLock(eventCounterMutex);
            totalEvents += eventCounter[eventIndex];
        }

        EXPECT_NEAR(MAX_EVENT_COUNTER * 5, totalEvents, 10);
    }
    EXPECT_EQ(0, eventAllocations);
}

TEST(SPTK_Timer, repeat_multiple_timers)
{
    {
        vector< shared_ptr<Timer> > timers;

        for (size_t timerIndex = 0; timerIndex < MAX_TIMERS; timerIndex++) {
            timers.push_back(make_shared<Timer>(gtestTimerCallback2));
            shared_ptr<Timer> timer = timers[timerIndex];
            for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; eventIndex++)
                timer->repeat(chrono::milliseconds(20), (void*) eventIndex);
        }

        this_thread::sleep_for(chrono::milliseconds(110));

        for (size_t timerIndex = 0; timerIndex < MAX_TIMERS; timerIndex++) {
            shared_ptr<Timer> timer = timers[timerIndex];
            timer->cancel();
        }

        this_thread::sleep_for(chrono::milliseconds(10));

        int totalEvents(0);
        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; eventIndex++) {
            UniqueLock(eventCounterMutex);
            totalEvents += eventCounter[eventIndex];
        }

        EXPECT_NEAR(MAX_TIMERS * MAX_EVENT_COUNTER * 6, totalEvents, 10 * MAX_TIMERS);
    }
    EXPECT_EQ(0, eventAllocations);
}

#endif
