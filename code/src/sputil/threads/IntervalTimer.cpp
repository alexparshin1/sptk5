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
#include <sptk5/threads/IntervalTimer.h>

using namespace std;
using namespace sptk;
using namespace chrono;

class sptk::IntervalTimerThread
    : public Thread
{
public:
    void terminate() override;

    IntervalTimerThread()
        : Thread("IntervalTimer thread")
    {
    }

    ~IntervalTimerThread() override
    {
        m_semaphore.post();
    }

    void schedule(const STimerEvent& event)
    {
        scoped_lock lock(m_scheduledMutex);
        m_scheduledEvents.emplace(event);
        if (m_scheduledEvents.size() == 1)
        {
            m_semaphore.post();
        }
    }

    STimerEvent waitForEvent()
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

protected:
    void threadFunction() override;

private:
    mutex m_scheduledMutex;
    IntervalTimer::EventQueue m_scheduledEvents;
    Semaphore m_semaphore;
    //const chrono::milliseconds m_repeatInterval;

    STimerEvent nextEvent()
    {
        scoped_lock lock(m_scheduledMutex);

        while (!m_scheduledEvents.empty())
        {
            if (auto& event = m_scheduledEvents.front();
                !event->cancelled())
            {
                return event;
            }
            m_scheduledEvents.pop();
        }

        return nullptr;
    }

    void popFrontEvent()
    {
        STimerEvent event;

        scoped_lock lock(m_scheduledMutex);
        if (m_scheduledEvents.empty())
        {
            return;
        }

        m_scheduledEvents.pop();
    }
};

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

IntervalTimer::IntervalTimer(std::chrono::milliseconds repeatInterval)
    : m_repeatInterval(repeatInterval)
{
    m_timerThread = std::make_shared<IntervalTimerThread>();
    m_timerThread->run();
}

IntervalTimer::~IntervalTimer()
{
    m_timerThread->terminate();
    m_timerThread->join();
}

STimerEvent IntervalTimer::repeat(const TimerEvent::Callback& eventCallback, int repeatCount)
{
    auto event = make_shared<TimerEvent>(DateTime::Now() + m_repeatInterval, eventCallback, m_repeatInterval, repeatCount);
    m_timerThread->schedule(event);

    return event;
}

void IntervalTimer::cancel()
{
    // Cancel all events in this timer
    scoped_lock lock(m_mutex);
    while (!m_events.empty())
    {
        m_events.front()->cancel();
        m_events.pop();
    }
}
