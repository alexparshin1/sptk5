#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <sptk5/DateTime.h>

namespace sptk {

/**
 * Timer event class.
 * Stores event data, including references to parent IntervalTimer
 * and events map.
 */
class SP_EXPORT TimerEvent
{
    friend class IntervalTimer;

public:
    /**
     * @brief Event callback definition.
     * Events call that function when there is time for them to fire.
     */
    using Callback = std::function<void()>;

    /**
     * @brief Disabled event copy constructor
     * @param other                 Other event
     */
    TimerEvent(const TimerEvent& other) = delete;

    /**
     * @brief Disabled event assignment
     * @param other                 Other event
     */
    TimerEvent& operator=(const TimerEvent& other) = delete;

    /**
     * @brief Constructor
     * @param timestamp             Fire at timestamp
     * @param eventCallback         Event callback function
     * @param repeatCount           Repeat count, -1 means no limit
     */
    TimerEvent(DateTime::time_point timestamp, Callback eventCallback,
               std::chrono::milliseconds repeatInterval, int repeatCount = -1);

    /**
     * @return event fire at timestamp
     */
    const DateTime::time_point when() const
    {
        return m_when;
    }

    /**
     * @brief Fire event by calling its callback function..
     */
    bool fire();

    void cancel()
    {
        std::scoped_lock lock(m_mutex);
        m_callback = nullptr;
    }

    bool cancelled() const
    {
        std::scoped_lock lock(m_mutex);
        return m_callback == nullptr;
    }

private:
    mutable std::mutex        m_mutex;           ///< Mutex that protects internal data
    DateTime::time_point      m_when;            ///< Event serial and when the event has to fire next time.
    std::chrono::milliseconds m_repeatInterval;  ///< Event repeat interval
    Callback                  m_callback;        ///< Event callback function, defined when event is scheduled.
    int                       m_repeatCount {0}; ///< Number of event repeats, -1 means no limit.
};

/**
 * Type definition for timer event
 */
using STimerEvent = std::shared_ptr<TimerEvent>;

} // namespace sptk
