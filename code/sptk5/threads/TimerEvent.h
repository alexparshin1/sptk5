#pragma once

#include <cstdint>
#include <functional>
#include <atomic>
#include <sptk5/DateTime.h>

namespace sptk {
/**
 * @brief Timer event class.
 * Stores event data, including references to parent IntervalTimer and events map.
 */
class SP_EXPORT TimerEvent
{
    friend class IntervalTimer;
    friend class TimerEvents;

public:
    /**
     * @brief Event callback definition.
     * Events call that function when there is time for them to fire.
     */
    using Callback = std::function<void()>;

    /**
     * @brief Disabled event copy constructor.
     * @param other                 The other event.
     */
    TimerEvent(const TimerEvent& other) = delete;

    /**
     * @brief Disabled event assignment.
     * @param other                 The other event.
     */
    TimerEvent& operator=(const TimerEvent& other) = delete;

    /**
     * @brief Constructor.
     * @param timestamp             Fire at timestamp.
     * @param eventCallback         Event callback function.
     * @param repeatInterval        Repeat interval.
     * @param repeatCount           Repeat count, -1 means no limit.
     */
    TimerEvent(DateTime::time_point timestamp, Callback eventCallback,
               std::chrono::microseconds repeatInterval, int repeatCount = -1);

    /**
     * @return event fire at timestamp.
     */
    DateTime::time_point when() const
    {
        return m_when.load(std::memory_order_acquire);
    }

    /**
     * @brief Fire the event by calling its callback function.
     */
    bool fire();

    /**
     * @brief Cancel the event, so that it is dropped rather than fired.
     *
     * The callback is not cleared: it is written once, when the event is made, and never again, and
     * that is what lets the event do without a mutex. Whatever the callback captured is released
     * when the event is - which for a cancelled event still in the queue is when the queue drops it,
     * at the time it was scheduled for.
     */
    void cancel()
    {
        m_cancelled.store(true, std::memory_order_release);
    }

    bool cancelled() const
    {
        return m_cancelled.load(std::memory_order_acquire);
    }

private:
    // No mutex. It used to be here for one thing only - cancel() clearing the callback while fire()
    // was copying it - and it was 40 of the event's 96 bytes, on an object a busy broker keeps one
    // of per session. The callback is now immutable, cancellation is a flag, and the one field that
    // two threads really do share is atomic.
    //
    // m_when is that field: fire() moves it on for a repeating event, while mcs_since_epoch() reads
    // it from whichever thread is adding to the queue. That read was never taken under the mutex, so
    // this also closes a data race that was there all along.
    std::atomic<DateTime::time_point> m_when;            ///< When the event has to fire next time.
    std::chrono::microseconds         m_repeatInterval;  ///< Event repeat interval.
    const Callback                    m_callback;        ///< Event callback, given when the event is made.
    int                               m_repeatCount {0}; ///< Number of event repeats, -1 means no limit; the timer thread's own.
    std::atomic_bool                  m_cancelled {false}; ///< Set by cancel(), read before firing.

    /**
     * @return event fire at timestamp.
     */
    int64_t mcs_since_epoch() const
    {
        auto duration = m_when.load(std::memory_order_acquire).time_since_epoch();
        // int64_t, not long: microseconds since the epoch need 51 bits, and long is 32-bit on
        // Windows. Truncating wrapped the value every ~71 minutes, so an event scheduled past a
        // wrap sorted ahead of events that were already due and starved them until it fired.
        return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    }
};

/**
 * @brief Type definition for timer event.
 */
using STimerEvent = std::shared_ptr<TimerEvent>;

} // namespace sptk
