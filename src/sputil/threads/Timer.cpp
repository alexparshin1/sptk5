#include "Timer.h"

using namespace std;
using namespace sptk;

Timer::TimerThread::TimerThread()
: Thread("Timer thread")
{}

void Timer::TimerThread::threadFunction()
{
    DateTime nextEventTS = DateTime::Now() + chrono::seconds(1);
    while (!terminated()) {
        if (m_semaphore.wait(nextEventTS)) {
            lock_guard<mutex> lock(m_scheduledMutex);
            if (m_scheduledEvents.empty())
                continue;
            auto front = m_scheduledEvents.begin();
            nextEventTS = front->second->timestamp();
        } else {
            Event* event = nullptr;
            {
                lock_guard<mutex> lock(m_scheduledMutex);
                if (!m_scheduledEvents.empty()) {
                    auto front = m_scheduledEvents.begin();
                    event = front->second;
                    m_scheduledEvents.erase(front);
                }
            }
            if (event != nullptr)
                event->callback(event->data());
        }
    }
}

void Timer::TimerThread::terminate()
{
    m_semaphore.post();
    Thread::terminate();
}
