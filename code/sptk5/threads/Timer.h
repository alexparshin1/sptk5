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

#include <functional>
#include <set>

namespace sptk {

class TimerThread;

/**
 * Generic timer class.
 * Can fire one time off and repeatable events
 */
class SP_EXPORT Timer
{
public:
    /**
     * Timer event class.
     * Stores event data, including references to parent Timer
     * and events map.
     */
    class EventData
    {
        friend class Timer;

    public:
        /**
         * Event callback definition.
         * Events call that function when there is time for them to fire.
         */
        using Callback = std::function<void()>;

        /**
         * Disabled event copy constructor
         * @param other                 Other event
         */
        EventData(const EventData& other) = delete;

        /**
         * Disabled event assignment
         * @param other                 Other event
         */
        EventData& operator=(const EventData& other) = delete;

        /**
         * Constructor
         * @param timestamp             Fire at timestamp
         * @param eventCallback         Event callback function
         * @param repeatEvery           Event repeate interval
         * @param repeatCount           Repeat count, -1 means no limit
         */
        EventData(const DateTime& timestamp, const Callback& eventCallback, std::chrono::milliseconds repeatEvery,
                  int repeatCount = -1);

        /**
         * @return event fire at timestamp
         */
        const DateTime& when() const
        {
            return m_when;
        }

        /**
         * Add interval to event fire at timestamp
         * @param interval              Shift interval
         */
        bool shift(std::chrono::milliseconds interval)
        {
            if (m_repeatCount == 0)
            {
                return false;
            }

            if (m_repeatCount > 0)
            {
                --m_repeatCount;
            }

            // Repeat count < 0 - infinite repeats
            m_when = m_when + interval;
            return true;
        }

        /**
         * Fire event by calling its callback function..
         */
        bool fire();

        bool cancelled() const
        {
            return m_cancelled;
        }

        void cancel()
        {
            m_cancelled = true;
        }

    private:
        DateTime m_when;                            ///< When the event has to fire next time.
        Callback m_callback;                        ///< Event callback function, defined when event is scheduled.
        std::chrono::milliseconds m_repeatInterval; ///< Event repeat interval.
        int m_repeatCount {0};                      ///< Number of event repeats, -1 means no limit.
        bool m_cancelled {false};
    };

    /**
     * Type definition for timer event
     */
    using Event = std::shared_ptr<EventData>;

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
    Event fireAt(const DateTime& timestamp, const EventData::Callback& eventCallback);

    /**
     * Schedule repeatable event.
     * The first event is scheduled at current time + interval.
     * @param interval                  Event repeat interval.
     * @param eventCallback             Event callback.
     * @param repeatCount               Repeat count, -1 means no limit
     * @return event handle, that may be used to cancel this event.
     */
    Event repeat(std::chrono::milliseconds interval, const EventData::Callback& eventCallback, int repeatCount = -1);

    /**
     * Cancel all events
     */
    void cancel();

private:
    std::shared_ptr<TimerThread> m_timerThread; ///< Event processing thread
};

} // namespace sptk
