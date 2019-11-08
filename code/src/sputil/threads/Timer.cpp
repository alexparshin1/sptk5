
#include <sptk5/threads/Timer.h>
#include <sptk5/cutils>

#include "sptk5/threads/Timer.h"

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

protected:

    void threadFunction() override;

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

Timer::EventData::EventData(const DateTime& timestamp, Callback& eventCallback, milliseconds repeatEvery)
: m_id(timestamp), m_callback(eventCallback), m_repeatEvery(repeatEvery)
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

void Timer::EventData::fire()
{
    try {
        m_callback();
    }
    catch (const Exception& e) {
        CERR(e.what())
    }
}

void TimerThread::threadFunction()
{
    while (!terminated()) {
        Timer::Event event;
        if (waitForEvent(event)) {
            event->fire();
            if (event->getInterval().count() != 0) {
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

static void checkTimerThreadRunning()
{
    lock_guard<mutex> lock(timerThreadMutex);
    if (timerThread == nullptr) {
        timerThread = new TimerThread();
        timerThread->run();
    }
}

Timer::Event Timer::fireAt(const DateTime& timestamp, EventData::Callback eventCallback)
{
    checkTimerThreadRunning();

    Event event = make_shared<EventData>(timestamp, eventCallback, milliseconds());
    timerThread->schedule(event);

    lock_guard<mutex> lock(m_mutex);
    m_events.insert(event);

    return event;
}

Timer::Event Timer::repeat(milliseconds interval, EventData::Callback eventCallback)
{
    checkTimerThreadRunning();

    Event event = make_shared<EventData>(DateTime::Now() + interval, eventCallback, interval);
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
    eventSet++;
}

TEST(SPTK_Timer, fireAt)
{
    if (DateTime::Now() > DateTime()) // always true
    {
        Timer timer;
        DateTime when = DateTime::Now() + milliseconds(50);

        int eventSet(0);
        function<void()> eventCallback = bind(gtestTimerCallback, &eventSet);
        timer.fireAt(when, eventCallback);
        this_thread::sleep_until((when + milliseconds(20)).timePoint());

        EXPECT_EQ(1, eventSet);
    }
    EXPECT_EQ(0, eventAllocations);
}

TEST(SPTK_Timer, repeat)
{
    if (DateTime::Now() > DateTime()) // always true
    {
        Timer timer;

        int eventSet(0);

        function<void()> eventCallback = bind(gtestTimerCallback, &eventSet);
        Timer::Event handle = timer.repeat(milliseconds(20), eventCallback);
        this_thread::sleep_for(milliseconds(110));
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

static void gtestTimerCallback2(void* theEventData)
{
    UniqueLock(eventCounterMutex);
    size_t eventIndex = size_t(theEventData);
    eventCounter[eventIndex]++;
}

TEST(SPTK_Timer, minimal)
{
    int counter = 1;
    Timer timer;

    timer.fireAt(DateTime::Now() + milliseconds(10), [&counter]()
        {
            counter++;
        }
    );
    this_thread::sleep_for(milliseconds(20));
    EXPECT_EQ(counter, 2);
}

TEST(SPTK_Timer, repeat_multiple_events)
{
    if (DateTime::Now() > DateTime()) // always true
    {
        Timer timer;

        vector<Timer::Event> createdEvents;
        for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; eventIndex++) {
            eventData[eventIndex] = eventIndex;
            function<void()> callback = bind(gtestTimerCallback2, (void*)eventIndex);
            Timer::Event event = timer.repeat(milliseconds(20), callback);
            createdEvents.push_back(event);
        }

        this_thread::sleep_for(milliseconds(110));

        for (int eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; eventIndex++) {
            Timer::Event event = createdEvents[eventIndex];
            timer.cancel(event);
        }

        this_thread::sleep_for(milliseconds(20));

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
    if (DateTime::Now() > DateTime()) // always true
    {
        vector< shared_ptr<Timer> > timers;

        for (size_t timerIndex = 0; timerIndex < MAX_TIMERS; timerIndex++) {
            timers.push_back(make_shared<Timer>());
            shared_ptr<Timer> timer = timers[timerIndex];
            for (size_t eventIndex = 0; eventIndex < MAX_EVENT_COUNTER; eventIndex++) {
                function<void()> callback = bind(gtestTimerCallback2, (void*)eventIndex);
                timer->repeat(milliseconds(20), callback);
            }
        }

        this_thread::sleep_for(milliseconds(110));

        for (size_t timerIndex = 0; timerIndex < MAX_TIMERS; timerIndex++) {
            shared_ptr<Timer> timer = timers[timerIndex];
            timer->cancel();
        }

        this_thread::sleep_for(milliseconds(10));

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
