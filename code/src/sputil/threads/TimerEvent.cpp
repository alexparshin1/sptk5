#include <sptk5/Exception.h>
#include <sptk5/Printer.h>
#include <sptk5/threads/TimerEvent.h>

#include <utility>

using namespace std;
using namespace sptk;

TimerEvent::TimerEvent(DateTime timestamp, Callback eventCallback, std::chrono::milliseconds repeatInterval, int repeatCount)
    : m_when(std::move(timestamp))
    , m_repeatInterval(repeatInterval)
    , m_callback(std::move(eventCallback))
    , m_repeatCount(repeatCount)
{
}

bool TimerEvent::fire()
{
    std::scoped_lock lock(m_mutex);

    try
    {
        if (m_callback)
        {
            m_callback();
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }

    if (m_repeatCount == 0)
    {
        return false;
    }

    if (m_repeatCount > 0)
    {
        --m_repeatCount;
        if (m_repeatCount == 0)
        {
            return false;
        }
    }

    m_when = m_when + m_repeatInterval;

    return true;
}
