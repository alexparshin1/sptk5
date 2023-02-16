/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/net/BaseSocket.h>
#include <sptk5/net/SocketPool.h>
#include <sptk5/threads/Flag.h>
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
    : public Thread
{
public:
    /**
     * Constructor
     * @param name                  Logical name for event manager (also the thread name)
     * @param eventsCallback        Callback function called for socket events
     * @param timeout	            Timeout in event monitoring loop
     */
    SocketEvents(const String& name, const SocketEventCallback& eventsCallback,
                 std::chrono::milliseconds timeout = std::chrono::milliseconds(
                     100));

    /**
     * Destructor
     */
    ~SocketEvents() override;

    /**
     * Add socket to collection and start monitoring its events
     * @param socket	            Socket to monitor
     * @param userData	            User data to pass into callback function
     */
    void add(BaseSocket& socket, uint8_t* userData);

    /**
     * Remove socket from collection and stop monitoring its events
     * @param socket	            Socket to remove
     */
    void remove(BaseSocket& socket);

    /**
     * Check if socket is already being monitored
     * @param socket	            Socket to check
     */
    bool has(BaseSocket& socket);

    /**
     * Stop socket events manager and wait until it joins.
     */
    void stop();

    /**
     * Terminate socket events manager and continue.
     */
    void terminate() override;

    /**
     * Get the size of socket collection
     * @return number of sockets being watched
     */
    size_t size() const;

protected:
    /**
     * Event monitoring thread
     */
    void threadFunction() override;

private:
    mutable std::mutex m_mutex;          ///< Mutex that protects map of sockets to corresponding user data
    SocketPool m_socketPool;             ///< OS-specific event manager
    std::map<int, void*> m_watchList;    ///< Map of sockets to corresponding user data
    std::chrono::milliseconds m_timeout; ///< Timeout in event monitoring loop

    Flag m_started;          ///< Is watching started?
    bool m_shutdown {false}; ///< Is watching shutdown?
};

} // namespace sptk
