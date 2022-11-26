#include <sptk5/Exception.h>
#include <sptk5/Printer.h>
#include <sptk5/threads/TimerEvent.h>

using namespace std;
using namespace sptk;

TimerEvent::TimerEvent(const DateTime& timestamp, const Callback& eventCallback, int repeatCount)
    : m_when(timestamp)
    , m_callback(eventCallback)
    , m_repeatCount(repeatCount)
{
}

bool TimerEvent::fire(chrono::milliseconds repeatInterval)
{
    try
    {
        m_callback();
    }
    catch (const Exception& e)
    {
        CERR(e.what())
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

    m_when = m_when + repeatInterval;

    return true;
}
