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
         * Timer event class.
         * Stores event data, including references to parent Timer
         * and events map.
         */
        class Event
        {
            friend class Timer;
        public:
            /**
             * Event callback definition.
             * Events call that function when there is time for them to fire.
             */
            typedef void (*Callback) (void* eventData);

            /**
             * Events map definition.
             * Events map stores events ordered by their timestamps.
             */
            typedef std::multimap<long, Event*> Map;

            /**
             * Event bookmark definition.
             * It's just an iterator to event entry in events map.
             */
            typedef Map::iterator                   Bookmark;

        protected:
            DateTime                    m_timestamp;    ///< Event timestamp - when the event has to fire next time.
            void*                       m_data;         ///< Opaque event data, defined when event is scheduled. Passed by event to callback function.
            std::chrono::milliseconds   m_repeatEvery;  ///< Event repeat interval.
            Timer*                      m_timer;        ///< Parent timer
            Bookmark                    m_bookmark;     ///< Bookmark of event entry in events map.

        public:
            /**
             * @return Bookmark of event entry in events map.
             */
            const Bookmark& getBookmark() const;

            /**
             * Set bookmark of event entry in events map.
             * @param bookmark              Bookmark of event entry in events map.
             */
            void setBookmark(const Bookmark& bookmark);

        private:

            /**
             * Disabled event copy constructor
             * @param other                 Other event
             */
            Event(const Event& other)
            : m_timer(other.m_timer) {}

            /**
             * Disabled event assignment
             * @param other                 Other event
             */
            Event& operator = (const Event&) { return *this; }

        public:
            /**
             * Constructor
             * @param timer                 Parent timer
             * @param timestamp             Fire at timestamp
             * @param eventData             Event data that will be passed to timer callback
             * @param repeatEvery           Event repeate interval
             */
            Event(Timer& timer, const DateTime& timestamp, void* eventData, std::chrono::milliseconds repeatEvery);

            /**
             * Destructor
             */
            ~Event();

            /**
             * @return event fire at timestamp
             */
            const DateTime& getWhen() const
            {
                return m_timestamp;
            }

            /**
             * Add interval to event fire at timestamp
             * @param interval              Shift interval
             */
            void shift(std::chrono::milliseconds interval)
            {
                m_timestamp = m_timestamp + interval;
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
             * Execute parent timer callback function with event data
             */
            void fire()
            {
                m_timer->fire(this);
            }
        };

    protected:

        std::mutex                  m_mutex;        ///< Mutex protecting events set
        std::set<Event*>            m_events;       ///< Events scheduled by this timer
        Event::Callback             m_callback;     ///< Event callback function.

        void unlink(Event* event);                  ///< Remove event from this timer
        void fire(Event* event);                    ///< Fire event

    public:
        /**
         * Constructor
         * @param callback                  Timer callback function, called when event is up
         */
        Timer(Event::Callback callback)
        : m_callback(callback)
        {}

        /**
         * Destructor.
         * Cancel all events scheduled by this timer.
         */
        virtual ~Timer();

        /**
         * Schedule single event.
         * @param timestamp                 Fire at timestamp
         * @param eventData                 User data that will be passed to timer callback function.
         * @return event handle, that may be used to cancel this event.
         */
        void* fireAt(const DateTime& timestamp, void* eventData);

        /**
         * Schedule repeatable event.
         * The first event is scheduled at current time + interval.
         * @param interval                  Event repeat interval.
         * @param eventData                 User data that will be passed to timer callback function.
         * @return event handle, that may be used to cancel this event.
         */
        void* repeat(std::chrono::milliseconds interval, void* eventData);

        /**
         * Cancel event
         * @param handle                    Event handle, returned by event scheduling method.
         */
        void  cancel(void* handle);
    };

} // namespace sptk

#endif
