/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/Printer.h>
#include <sptk5/net/SocketPool.h>
#include <sptk5/threads/Counter.h>
#include <sptk5/threads/Thread.h>

namespace sptk {
/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief Socket events manager.
 *
 * Dynamic collection of sockets that delivers socket events
 * such as data available for read or peer has closed connection
 * to its sockets.
 */
template<typename T>
class SocketEvents
    : public SocketObjectPool<T>
    , public Thread
{
public:
    /**
     * @brief Constructor.
     * @param name               Logical name for event manager (also the thread name).
     * @param eventsCallback     Callback function called for socket events.
     * @param timeout            Timeout in the event monitoring loop.
     * @param triggerMode        Socket event trigger mode.
     * @param maxEvents          Maximum number of events per poll.
     */
    SocketEvents(const String&                    name,
                 const SocketEventCallback<T>&    eventsCallback,
                 const std::chrono::milliseconds& timeout = std::chrono::milliseconds(100),
                 SocketPoolTriggerMode            triggerMode = SocketPoolTriggerMode::LevelTriggered,
                 size_t                           maxEvents = 1024)
        : SocketObjectPool<T>(eventsCallback, triggerMode, maxEvents)
        , Thread(name)
        , m_timeout(timeout)
    {
        Thread::run();
    }


    /**
     * @brief Destructor.
     */
    ~SocketEvents() override
    {
        stop();
    }

    /**
     * @brief Stop the socket events manager and wait until it joins.
     */
    void stop()
    {
        terminate();
        join();
    }

protected:
    /**
     * @brief Event monitoring thread.
     */
    void threadFunction() override
    {
        SocketObjectPool<T>::open();
        while (!terminated())
        {
            try
            {
                if (!SocketObjectPool<T>::waitForEvents(m_timeout))
                {
                    break;
                }
            }
            catch (const Exception& e)
            {
                CERR(e.message());
            }
        }
        SocketObjectPool<T>::close();
    }

private:
    std::chrono::milliseconds m_timeout; ///< Timeout in the event monitoring loop.
};

/**
 * @}
 */

} // namespace sptk
