/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include "sptk5/cthreads"

namespace sptk {

/**
 * @brief Base class for Timer and IntervalTimer internal threads
 */
class TimerThread
    : public Thread
{
public:
    /**
     * @brief Constructor
     * @param threadName        Thread name
     */
    TimerThread();
    ~TimerThread() override;

    /**
     * @brief Schedule an event
     * @param event             Event
     */
    void schedule(const STimerEvent& event);

    void clear();

    /**
     * @brief Terminate thread
     */
    void terminate() override;

protected:
    /**
     * @brief Wake up (signal) semaphore
     */
    void wakeUp();

    /**
     * @brief Wait for the next event in the queue
     * @return
     */
    STimerEvent waitForEvent();

    /**
     * @brief Thread function
     */
    void threadFunction() override;

private:
    using EventMap = std::multimap<long, STimerEvent>;

    Semaphore m_semaphore; ///< Semaphore to wait for events
    std::mutex m_scheduledMutex;
    EventMap m_scheduledEvents;

    /**
     * @brief Get next scheduled event
     * @return Event
     */
    STimerEvent nextEvent();

    /**
     * @brief Remove next scheduled event
     */
    void popFrontEvent();
};

} // namespace sptk
