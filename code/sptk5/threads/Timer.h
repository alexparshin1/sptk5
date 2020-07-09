#ifndef __TIMER_H__
#define __TIMER_H__

#include "Thread.h"
#include "Semaphore.h"

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
         * Timer Event Id
         */
        struct EventId
        {
            uint64_t    serial {Timer::nextSerial++};  ///< Serial number
            DateTime    when;                   ///< Execution date and time
            /**
             * Constructor
             * @param when      Event execution time
             */
            explicit EventId(const DateTime& when);
        };

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
            typedef std::function<void()> Callback;

            /**
             * Disabled event copy constructor
             * @param other                 Other event
             */
            EventData(const EventData& other) = delete;

            /**
             * @return Bookmark of event entry in events map.
             */
            const EventId& getId() const;

            /**
             * Disabled event assignment
             * @param other                 Other event
             */
            EventData& operator = (const EventData& other) = delete;

            /**
             * Constructor
             * @param timestamp             Fire at timestamp
             * @param eventCallback         Event callback function
             * @param repeatEvery           Event repeate interval
             * @param repeatCount           Repeat count, -1 means no limit
             */
            EventData(const DateTime& timestamp, Callback& eventCallback, std::chrono::milliseconds repeatEvery, int repeatCount=-1);

            /**
             * Destructor
             */
            ~EventData();

            /**
             * @return event fire at timestamp
             */
            const DateTime& getWhen() const
            {
                return m_id.when;
            }

            /**
             * Add interval to event fire at timestamp
             * @param interval              Shift interval
             */
            bool shift(std::chrono::milliseconds interval)
            {
                if (m_repeatCount == 0)
                    return false;

                if (m_repeatCount > 0) {
                    m_id.when = m_id.when + interval;
                    m_repeatCount--;
                    return true;
                }

                // Repeat count < 0 - infinite repeats
                m_id.when = m_id.when + interval;
                return true;
            }

            /**
             * @return event repeat interval
             */
            const std::chrono::milliseconds& getInterval() const
            {
                return m_repeatInterval;
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
            bool fire();

        private:

            EventId                     m_id;                ///< Event serial and when the event has to fire next time.
            Callback                    m_callback;          ///< Event callback function, defined when event is scheduled.
            std::chrono::milliseconds   m_repeatInterval;    ///< Event repeat interval.
            int                         m_repeatCount {0};   ///< Number of event repeats, -1 means no limit.

        };

        /**
         * Type definition for timer event
         */
        typedef std::shared_ptr<EventData> Event;

        /**
         * Constructor
         */
        Timer() = default;

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
        Event fireAt(const DateTime& timestamp, EventData::Callback eventCallback);

        /**
         * Schedule repeatable event.
         * The first event is scheduled at current time + interval.
         * @param interval                  Event repeat interval.
         * @param eventCallback             Event callback.
         * @param repeatCount               Repeat count, -1 means no limit
         * @return event handle, that may be used to cancel this event.
         */
        Event repeat(std::chrono::milliseconds interval, EventData::Callback eventCallback, int repeatCount=-1);

        /**
         * Cancel event
         * @param event                     Event handle, returned by event scheduling method.
         */
        void  cancel(Event event);

        /**
         * Cancel all events
         */
        void  cancel();

    protected:

        void unlink(Event event);                   ///< Remove event from this timer

    private:

        mutable std::mutex              m_mutex;        ///< Mutex protecting events set
        std::set<Event>                 m_events;       ///< Events scheduled by this timer
        static std::atomic<uint64_t>    nextSerial;     ///< Event id serial

        std::set<Timer::Event> moveOutEvents();
    };

} // namespace sptk

#endif
