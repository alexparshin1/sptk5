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
            uint64_t    serial;     ///< Serial number
            DateTime    when;       ///< Execution date and time
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

        private:

            EventId                     m_id;                ///< Event serial and when the event has to fire next time.
            Callback                    m_callback;          ///< Event callback function, defined when event is scheduled.
            std::chrono::milliseconds   m_repeatEvery;       ///< Event repeat interval.

        public:

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
             */
            EventData(const DateTime& timestamp, Callback& eventCallback, std::chrono::milliseconds repeatEvery);

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
            void shift(std::chrono::milliseconds interval)
            {
                m_id.when = m_id.when + interval;
            }

            /**
             * @return event repeat interval
             */
            const std::chrono::milliseconds& getInterval() const
            {
                return m_repeatEvery;
            }

            /**
             * Fire event by calling its callback function..
             */
            void fire();

        };

        /**
         * Type definition for timer event
         */
        typedef std::shared_ptr<EventData> Event;

    private:

        mutable std::mutex          m_mutex;        ///< Mutex protecting events set
        std::set<Event>             m_events;       ///< Events scheduled by this timer

        std::set<Timer::Event> moveOutEvents();

    protected:

        void unlink(Event event);                   ///< Remove event from this timer

    public:
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
         * @return event handle, that may be used to cancel this event.
         */
        Event repeat(std::chrono::milliseconds interval, EventData::Callback eventCallback);

        /**
         * Cancel event
         * @param event                     Event handle, returned by event scheduling method.
         */
        void  cancel(Event event);

        /**
         * Cancel all events
         */
        void  cancel();
    };

} // namespace sptk

#endif
