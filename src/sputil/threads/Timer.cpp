#include "Timer.h"

using namespace std;
using namespace sptk;

class TimerThread : public Thread
{
    std::mutex                              m_scheduledMutex;
    std::multimap<uint64_t, Timer::Event*>  m_scheduledEvents;
    Semaphore                               m_semaphore;
protected:
    void threadFunction() override;

public:
    void terminate() override;

public:
    TimerThread()
    : Thread("Timer thread")
    {}

    void schedule(Timer::Event* event)
    {
        chrono::milliseconds timepointMS = chrono::duration_cast<chrono::milliseconds>(event->getWhen().sinceEpoch());

        lock_guard<mutex> lock(m_scheduledMutex);
        Timer::Event::Bookmark bookmark = m_scheduledEvents.insert(pair<uint64_t,Timer::Event*>(timepointMS.count(), event));
        event->setBookmark(bookmark);
        m_semaphore.post();
    }

    bool waitForEvent(Timer::Event*& event)
    {
        DateTime        when;
        {
            lock_guard<mutex> lock(m_scheduledMutex);
            if (m_scheduledEvents.empty()) {
                when = DateTime::Now() + chrono::seconds(1);
            } else {
                auto itor = m_scheduledEvents.begin();
                when = DateTime(chrono::milliseconds(itor->first));
            }
        }

        if (m_semaphore.wait(when)) {
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
            delete itor->second;
            m_scheduledEvents.erase(itor);
        }
    }

    void unlink(set<Timer::Event*>& events)
    {
        lock_guard<mutex> lock(m_scheduledMutex);
    }
};

static mutex                timerThreadMutex;
static TimerThread*         timerThread;

Timer::Event::Event(Timer& timer, const DateTime& timestamp, void* eventData, Callback eventCallback, std::chrono::milliseconds repeatEvery)
: m_timestamp(timestamp), m_data(eventData), m_repeatEvery(repeatEvery), m_timer(timer)
{}

Timer::Event::~Event()
{
    m_timer.unlink(this);
}

const Timer::Event::Bookmark& Timer::Event::getBookmark() const
{
    return m_bookmark;
}

void Timer::Event::setBookmark(const Timer::Event::Bookmark& bookmark)
{
    m_bookmark = bookmark;
}

void TimerThread::threadFunction()
{
    while (!terminated()) {
        Timer::Event* event(nullptr);
        if (waitForEvent(event)) {
            event->fire();
            if (event->getInterval().count() == 0)
                delete event;
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

void Timer::unlink(Timer::Event* event)
{
    lock_guard<mutex> lock(m_mutex);
    m_events.erase(event);
}

void Timer::fireAt(const DateTime& timestamp, void* eventData, Timer::Event::Callback callback)
{
    {
        lock_guard<mutex> lock(timerThreadMutex);
        if (timerThread == nullptr) {
            timerThread = new TimerThread();
            timerThread->run();
        }
    }

    Event* event = new Event(*this, timestamp, eventData, callback, chrono::milliseconds());
    timerThread->schedule(event);

    lock_guard<mutex> lock(m_mutex);
    m_events.insert(event);
}

