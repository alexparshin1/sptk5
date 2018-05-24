#ifndef __TIMER_H__
#define __TIMER_H__

#include "Thread.h"
#include "Semaphore.h"
#include "../../../../../../usr/include/c++/7/set"

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
            typedef std::multimap<uint64_t, Event*> Map;

            /**
             * Event bookmark definition.
             * It's just an iterator to event entry in events map.
             */
            typedef Map::iterator                   Bookmark;

        protected:
            DateTime                    m_timestamp;    ///< Event timestamp - when the event has to fire next time.
            void*                       m_data;         ///< Opaque event data, defined when event is scheduled. Passed by event to callback function.
            std::chrono::milliseconds   m_repeatEvery;  ///< Event repeat interval.
            Callback                    m_callback;     ///< Event callback function.
            Timer*                      m_timer;
            Bookmark                    m_bookmark;

        public:
            const Bookmark& getBookmark() const;
            void setBookmark(const Bookmark& bookmark);

        private:

            Event(const Event& other)
            : m_timer(other.m_timer) {}

            Event& operator = (const Event&) { return *this; }

        public:
            Event(Timer& timer, const DateTime& timestamp, void* eventData, Callback eventCallback, std::chrono::milliseconds repeatEvery);

            ~Event();

            const DateTime& getWhen() const
            {
                return m_timestamp;
            }

            void shift(std::chrono::milliseconds interval)
            {
                m_timestamp = m_timestamp + interval;
            }

            void* getData() const
            {
                return m_data;
            }

            void fire()
            {
                m_callback(m_data);
            }

            Timer& getTimer() const
            {
                return *m_timer;
            }

            const std::chrono::milliseconds& getInterval() const
            {
                return m_repeatEvery;
            }
        };

    protected:

        std::mutex                  m_mutex;
        std::set<Event*>            m_events;

        void unlink(Event* event);

    public:
        Timer() {}
        virtual ~Timer();
        void* fireAt(const DateTime& timestamp, void* eventData, Event::Callback callback);
        void* repeat(std::chrono::milliseconds interval, void* eventData, Event::Callback callback);
        void  cancel(void* handle);
    };

} // namespace sptk

#endif
