#pragma once

#include <functional>
#include <sptk5/DateTime.h>

namespace sptk {

/**
 * Timer event class.
 * Stores event data, including references to parent IntervalTimer
 * and events map.
 */
class TimerEvent
{
    friend class IntervalTimer;

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
    TimerEvent(const TimerEvent& other) = delete;

    /**
         * Disabled event assignment
         * @param other                 Other event
         */
    TimerEvent& operator=(const TimerEvent& other) = delete;

    /**
         * Constructor
         * @param timestamp             Fire at timestamp
         * @param eventCallback         Event callback function
         * @param repeatCount           Repeat count, -1 means no limit
         */
    TimerEvent(const DateTime& timestamp, const Callback& eventCallback, int repeatCount = -1);

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

        m_when = m_when + interval;
        if (m_repeatCount > 0)
        {
            --m_repeatCount;
            return true;
        }

        // Repeat count < 0 - infinite repeats
        return true;
    }

    /**
         * @return event repeat count
         */
    int getRepeatCount() const
    {
        return m_repeatCount;
    }

    /**
         * Fire event by calling its callback function..
         */
    bool fire(std::chrono::milliseconds repeatInterval);

    void cancel()
    {
        m_cancelled = true;
    }
    bool cancelled() const
    {
        return m_cancelled;
    }

private:
    DateTime m_when;          ///< Event serial and when the event has to fire next time.
    Callback m_callback;      ///< Event callback function, defined when event is scheduled.
    int m_repeatCount {0};    ///< Number of event repeats, -1 means no limit.
    bool m_cancelled {false}; ///< True if event is cancelled
};

/**
 * Type definition for timer event
 */
using STimerEvent = std::shared_ptr<TimerEvent>;

} // namespace sptk
