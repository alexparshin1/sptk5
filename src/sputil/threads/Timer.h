#ifndef __TIMER_H__
#define __TIMER_H__

#include <sptk5/threads/Thread.h>
#include <sptk5/threads/Semaphore.h>
#include <set>

namespace sptk {

    class Timer
    {
    public:

        class Event
        {
        public:
            typedef void (*Callback) (void* eventData);
            typedef std::multimap<uint64_t, Event*> Map;
            typedef Map::iterator                   Bookmark;
        private:
            DateTime                    m_timestamp;
            void*                       m_data;
            std::chrono::milliseconds   m_repeatEvery;
            Callback                    m_callback;
            Timer&                      m_timer;

            Event(const Event& other)
            : m_timer(other.m_timer) {}

            Event& operator = (const Event&) { return *this; }

        public:
            Event(Timer& timer, const DateTime& timestamp, void* eventData, Callback eventCallback, std::chrono::milliseconds repeatEvery);

            ~Event();

            const DateTime& when() const
            {
                return m_timestamp;
            }

            void when(const DateTime& timestamp)
            {
                m_timestamp = timestamp;
            }

            void* data() const
            {
                return m_data;
            }

            void fire()
            {
                m_callback(m_data);
            }

            Timer& timer() const
            {
                return m_timer;
            }

            const std::chrono::milliseconds& interval() const
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
        void fireAt(const DateTime& timestamp, void* eventData, Event::Callback callback);
        void repeat(std::chrono::milliseconds interval, void* eventData, Event::Callback callback);
    };

} // namespace sptk

#endif
