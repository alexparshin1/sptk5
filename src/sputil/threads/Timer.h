#ifndef __TIMER_H__
#define __TIMER_H__

#include <sptk5/threads/Thread.h>
#include <sptk5/threads/Semaphore.h>

namespace sptk {

    class Timer
    {
    public:
        class Event
        {
            const DateTime  m_timestamp;
            void*           m_data;
        public:
            Event(const DateTime& timestamp, void* m_data)
            : m_timestamp(timestamp), m_data(m_data)
            {}

            const DateTime& timestamp() const
            {
                return m_timestamp;
            }

            void* data() const
            {
                return m_data;
            }

            virtual void callback(void* data) {}
        };

    protected:
        class TimerThread : public Thread
        {
            std::mutex                      m_scheduledMutex;
            std::multimap<uint64_t, Event*> m_scheduledEvents;
            Semaphore                       m_semaphore;
        protected:
            void threadFunction() override;

        public:
            void terminate() override;

        public:
            TimerThread();
            void schedule(Event* event);
            bool waitForEvent(Event*& event);
        };
    };

} // namespace sptk

#endif
