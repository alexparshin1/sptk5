#ifndef __TIMER_H__
#define __TIMER_H__

#include "Thread.h"
#include "Semaphore.h"
#include <set>

namespace sptk {

    /**
     * Generic timer class.
     * Can fire one time off and repeatable events
     */
    class Timer
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
            EventId(const DateTime& when);
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
			typedef void(*Callback) (void* eventData);

		protected:
			EventId                     m_id;				///< Event serial and when the event has to fire next time.
			void*                       m_data {nullptr};   ///< Opaque event data, defined when event is scheduled. Passed by event to callback function.
			std::chrono::milliseconds   m_repeatEvery;		///< Event repeat interval.
			Timer*                      m_timer {nullptr};  ///< Parent timer

        public:
            /**
             * @return Bookmark of event entry in events map.
             */
            const EventId& getId() const;

        private:

            /**
             * Disabled event copy constructor
             * @param other                 Other event
             */
            EventData(const EventData& other)
            : m_id(DateTime()), m_timer(other.m_timer) {}

            /**
             * Disabled event assignment
             * @param other                 Other event
             */
            EventData& operator = (const EventData&) { return *this; }

        public:
            /**
             * Constructor
             * @param timer                 Parent timer
             * @param timestamp             Fire at timestamp
             * @param eventData             Event data that will be passed to timer callback
             * @param repeatEvery           Event repeate interval
             */
            EventData(Timer& timer, const DateTime& timestamp, void* eventData, std::chrono::milliseconds repeatEvery);

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
             * @return event data
             */
            void* getData() const
            {
                return m_data;
            }

            /**
             * @return parent timer
             */
            Timer& getTimer() const
            {
                return *m_timer;
            }

            /**
             * @return event repeat interval
             */
            const std::chrono::milliseconds& getInterval() const
            {
                return m_repeatEvery;
            }

            /**
             * Disconnect event from timer (internal)
             */
            void unlinkFromTimer()
            {
                m_timer = nullptr;
            }
        };

		/**
		 * Type definition for timer event
		 */
        typedef std::shared_ptr<EventData> Event;

    protected:

        std::mutex                  m_mutex;        ///< Mutex protecting events set
        std::set<Event>             m_events;       ///< Events scheduled by this timer
        EventData::Callback         m_callback;     ///< Event callback function.

        void unlink(Event event);                   ///< Remove event from this timer

    public:
        /**
         * Constructor
         * @param callback                  Timer callback function, called when event is up
         */
        Timer(EventData::Callback callback)
        : m_callback(callback)
        {}

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
         * Fire single event.
         * @param event                     User data that will be passed to timer callback function.
         */
        void fire(Event event);

        /**
         * Schedule single event.
         * @param timestamp                 Fire at timestamp
         * @param eventData                 User data that will be passed to timer callback function.
         * @return event handle, that may be used to cancel this event.
         */
        Event fireAt(const DateTime& timestamp, void* eventData);

        /**
         * Schedule repeatable event.
         * The first event is scheduled at current time + interval.
         * @param interval                  Event repeat interval.
         * @param eventData                 User data that will be passed to timer callback function.
         * @return event handle, that may be used to cancel this event.
         */
        Event repeat(std::chrono::milliseconds interval, void* eventData);

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
