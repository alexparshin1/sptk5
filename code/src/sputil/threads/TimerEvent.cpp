#include <sptk5/Exception.h>
#include <sptk5/Printer.h>
#include <sptk5/threads/TimerEvent.h>

#include <utility>

using namespace std;
using namespace sptk;

TimerEvent::TimerEvent(const DateTime::time_point timestamp, Callback eventCallback, const std::chrono::milliseconds repeatInterval, const int repeatCount)
    : m_when(timestamp)
    , m_repeatInterval(repeatInterval)
    , m_callback(std::move(eventCallback))
    , m_repeatCount(repeatCount)
{
}

bool TimerEvent::fire()
{
    Callback callback;
    auto     reschedule = true;
    {
        lock_guard lock(m_mutex);

        callback = m_callback;

        if (m_repeatCount == 0)
        {
            reschedule = false;
        }
        else if (m_repeatCount > 0)
        {
            --m_repeatCount;
            if (m_repeatCount == 0)
            {
                reschedule = false;
            }
        }

        if (reschedule)
        {
            m_when = m_when + m_repeatInterval;
        }
    }

    try
    {
        if (callback)
        {
            callback();
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }

    return reschedule;
}
