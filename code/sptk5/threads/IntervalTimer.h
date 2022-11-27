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

#pragma once

#include "Semaphore.h"
#include "Thread.h"
#include "TimerEvent.h"

#include <functional>
#include <set>

namespace sptk {

class IntervalTimerThread;

/**
 * Interval timer class.
 * Fires one time or repeatable events in a defined interval.
 */
class SP_EXPORT IntervalTimer
{
public:
    using EventQueue = std::queue<STimerEvent>;

    /**
     * Constructor
     */
    IntervalTimer(std::chrono::milliseconds repeatInterval);

    /**
     * Copy constructor
     * @param other                     IntervalTimer to copy from
     */
    IntervalTimer(const IntervalTimer& other) = delete;

    /**
     * Destructor.
     * Cancel all events scheduled by this timer.
     */
    virtual ~IntervalTimer();

    /**
     * Schedule repeatable event  to fire in the interval defined in the constructor.
     * The first event is scheduled at current time + interval.
     * @param eventCallback             Event callback.
     * @param repeatCount               Repeat count, -1 means no limit
     * @return event handle, that may be used to cancel this event.
     */
    STimerEvent repeat(const TimerEvent::Callback& eventCallback, int repeatCount = -1);

    /**
     * Cancel all events
     */
    void cancel();

private:
    mutable std::mutex m_mutex;                         ///< Mutex protecting events set
    std::chrono::milliseconds m_repeatInterval;         ///< Repeat interval
    EventQueue m_events;                                ///< Events scheduled by this timer
    std::mutex m_timerThreadMutex;                      ///< IntervalTimer thread mutex
    std::shared_ptr<IntervalTimerThread> m_timerThread; ///< IntervalTimer thread
};

} // namespace sptk
