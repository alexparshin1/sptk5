/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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
#include "TimerEvent.h"
#include "TimerThread.h"

#include <functional>
#include <set>

namespace sptk {

/**
 * Generic timer class.
 * Can fire one time off and repeatable events
 */
class SP_EXPORT Timer
{
public:
    /**
     * Constructor
     */
    Timer();

    /**
     * Copy constructor
     * @param other                     Timer to copy from
     */
    Timer(const Timer& other) = delete;

    /**
     * Copy assignment
     */
    Timer& operator=(const Timer&) = delete;

    /**
     * Destructor.
     * Cancel all events scheduled by this timer.
     */
    virtual ~Timer();

    /**
     * Schedule single event.
     * @param timestamp                 Fire at timestamp
     * @param eventCallback             Event callback.
     * @return event handle, that may be used to cancel this event.
     */
    STimerEvent fireAt(const DateTime::time_point& timestamp, const TimerEvent::Callback& eventCallback) const;

    /**
     * Schedule repeatable event.
     * The first event is scheduled at current time + interval.
     * @param interval                  Event repeat interval.
     * @param eventCallback             Event callback.
     * @param repeatCount               Repeat count, -1 means no limit
     * @return event handle, that may be used to cancel this event.
     */
    STimerEvent repeat(std::chrono::milliseconds interval, const TimerEvent::Callback& eventCallback, int repeatCount = -1) const;

    /**
     * Cancel all events
     */
    void cancel() const;

private:
    std::shared_ptr<TimerThread> m_timerThread {std::make_shared<TimerThread>()}; ///< Event processing thread
};

} // namespace sptk
