/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include "sptk5/SystemException.h"
#include <map>
#include <mutex>
#include <sptk5/Exception.h>
#include <sptk5/net/Socket.h>
#include <sptk5/net/SocketPool.h>
#include <sptk5/threads/Counter.h>
#include <sptk5/threads/Thread.h>

namespace sptk {

/**
 * Socket events manager.
 *
 * Dynamic collection of sockets that delivers socket events
 * such as data available for read or peer closed connection,
 * to its sockets.
 */
class SP_EXPORT SocketEvents
    : public SocketPool
    , public Thread
{
public:
    /**
     * @brief Constructor
     * @param name               Logical name for event manager (also the thread name)
     * @param eventsCallback     Callback function called for socket events
     * @param timeout            Timeout in event monitoring loop
     * @param triggerMode        Socket event trigger mode
     * @param maxEvents          Maximum number of events per poll
     */
    SocketEvents(const String&                    name,
                 const SocketEventCallback&       eventsCallback,
                 const std::chrono::milliseconds& timeout = std::chrono::milliseconds(100),
                 SocketPool::TriggerMode          triggerMode = SocketPool::TriggerMode::LevelTriggered,
                 size_t                           maxEvents = 1024);

    /**
     * @brief Destructor
     */
    ~SocketEvents() override;

    /**
     * @brief Stop socket events manager and wait until it joins.
     */
    void stop();

protected:
    /**
     * @brief Event monitoring thread
     */
    void threadFunction() override;

private:
    mutable std::mutex        m_mutex;   ///< Mutex that protects map of sockets to corresponding user data
    std::chrono::milliseconds m_timeout; ///< Timeout in event monitoring loop
};

} // namespace sptk
