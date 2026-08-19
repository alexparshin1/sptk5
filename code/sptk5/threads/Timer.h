/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include "TimerEvent.h"
#include "TimerThread.h"

namespace sptk {
/**
 * @brief Generic timer class.
 * Can fire one time off and repeatable events.
 */
class SP_EXPORT Timer final
{
public:
    /**
     * @brief Constructor.
     */
    Timer();

    /**
     * @brief Copy constructor.
     * @param other                     Timer to copy from.
     */
    Timer(const Timer& other) = delete;

    /**
     * @brief Copy assignment.
     */
    Timer& operator=(const Timer&) = delete;

    /**
     * @brief Destructor.
     * Cancel all events scheduled by this timer.
     */
    ~Timer();

    /**
     * @brief Schedule a single event.
     * @param timestamp                 Fire at timestamp.
     * @param eventCallback             Event callback.
     * @return event handle that may be used to cancel this event.
     */
    [[nodiscard]] STimerEvent fireAt(const DateTime::time_point& timestamp, const TimerEvent::Callback& eventCallback) const;

    /**
     * @brief Schedule repeatable event.
     * The first event is scheduled at the current time + interval.
     * @param interval                  Event repeat interval.
     * @param eventCallback             Event callback.
     * @param repeatCount               Repeat count, -1 means no limit.
     * @return event handle that may be used to cancel this event.
     */
    [[nodiscard]] STimerEvent repeat(std::chrono::microseconds interval, const TimerEvent::Callback& eventCallback, int repeatCount = -1) const;

private:
    std::shared_ptr<TimerThread> m_timerThread {std::make_shared<TimerThread>()}; ///< Event processing thread.
};

} // namespace sptk
