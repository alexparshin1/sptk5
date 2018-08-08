/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DateTime.h - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday Sep 17 2015                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_SOCKETEVENTS_H__
#define __SPTK_SOCKETEVENTS_H__

#include <map>
#include <mutex>
#include <sptk5/Exception.h>
#include <sptk5/threads/Thread.h>
#include <sptk5/net/BaseSocket.h>
#include <sptk5/net/SocketPool.h>
#include "sptk5/SystemException.h"

namespace sptk {

/**
 * Socket events manager.
 *
 * Dynamic collection of sockets that delivers socket events
 * such as data available for read or peer closed connection,
 * to its sockets.
 */
class SocketEvents : public Thread
{
    /**
     * OS-specific event manager
     */
    SocketPool                  m_socketPool;

    /**
     * Map of sockets to corresponding user data
     */
    std::map<int, void*>        m_watchList;

    /**
     * Mutex that protects map of sockets to corresponding user data
     */
    std::mutex                  m_mutex;

    /**
     * Timeout in event monitoring loop
     */
    std::chrono::milliseconds   m_timeout;

protected:

    /**
     * Event monitoring thread
     */
    void threadFunction() override;

public:
    /**
     * Constructor
     * @param eventsCallback        Callback function called for socket events
     * @param timeout	            Timeout in event monitoring loop
     */
    SocketEvents(SocketEventCallback eventsCallback, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

    /**
     * Destructor
     */
    ~SocketEvents();

    /**
     * Add socket to collection and start monitoring its events
     * @param socket	            Socket to monitor
     * @param userData	            User data to pass into callback function
     */
    void add(BaseSocket& socket, void* userData);

    /**
     * Remove socket from collection and stop monitoring its events
     * @param socket	            Socket to remove
     */
    void remove(BaseSocket& socket);
};

}

#endif
