#include <sptk5/Exception.h>
#include <sptk5/Printer.h>
#include <sptk5/threads/TimerEvent.h>

#include <utility>

using namespace std;
using namespace sptk;

TimerEvent::TimerEvent(const DateTime::time_point timestamp, Callback eventCallback, const std::chrono::microseconds repeatInterval, const int repeatCount)
    : m_when(timestamp)
    , m_repeatInterval(repeatInterval)
    , m_callback(std::move(eventCallback))
    , m_repeatCount(repeatCount)
{
}

bool TimerEvent::fire()
{
    auto reschedule = true;

    // The repeat count belongs to the thread that fires events, and no other touches it.
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
        m_when.store(m_when.load(std::memory_order_acquire) + m_repeatInterval, std::memory_order_release);
    }

    try
    {
        // Called straight out of the member: it is written once, when the event is made, so there is
        // nothing here to copy it away from. A cancelled event is not fired at all - the queue drops
        // it when it reaches the front - and this second look is for the one cancelled between there
        // and here.
        if (m_callback && !cancelled())
        {
            m_callback();
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }

    return reschedule;
}
