/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DateTime.h - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday Sep 17 2015                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_SOCKETPOOL_H__
#define __SPTK_SOCKETPOOL_H__

#include <sptk5/Exception.h>
#include <sptk5/threads/Thread.h>
#include <sptk5/net/BaseSocket.h>

#include <functional>
#include <map>
#include <mutex>

namespace sptk {

/**
 * Socket event types
 */
typedef enum {
    ET_UNKNOWN_EVENT,       ///< Event is unknown or undefined
    ET_HAS_DATA,            ///< Socket has data available to read
    ET_CONNECTION_CLOSED    ///< Peer closed connection
} SocketEventType;

/**
 * Type definition of socket event callback function
 */
typedef std::function<void(void *userData, SocketEventType eventType)> SocketEventCallback;

#ifdef _WIN32
#define INVALID_EPOLL nullptr
#else
#define INVALID_EPOLL INVALID_SOCKET
#endif // _WIN32

/**
 * Socket event manager.
 *
 * Uses OS-specific implementation.
 * On Linux it is using epoll, on BSD it is using kqueue,
 * and on Windows WSAAsyncSelect is used.
 */
class SP_EXPORT SocketPool : public std::mutex
{
    /**
     * Socket that controls other sockets events
     */
#ifdef _WIN32
    HANDLE                      m_pool { INVALID_EPOLL };
#else
    SOCKET                      m_pool { INVALID_EPOLL };
#endif // _WIN32

    /**
     * Callback function executed upon socket events
     */
    SocketEventCallback         m_eventsCallback;

    /**
     * Map of sockets to corresponding user data
     */
	std::map<BaseSocket*,void*> m_socketData;

public:
    /**
     * Constructor
     * @param eventCallback SocketEventCallback, Callback function executed upon socket events
     */
    explicit SocketPool(SocketEventCallback eventCallback);

    /**
     * Destructor
     */
    ~SocketPool();

    /**
     * Initialize socket pool
     */
    void open();

    /**
     * Wait until one or more sockets are signaled.
     *
     * Execute callback function for each signaled socket.
     */
    void waitForEvents(std::chrono::milliseconds timeout);

    /**
     * Shutdown socket pool.
     */
    void close();

    /**
     * Add socket to monitored pool
     * @param socket BaseSocket&, Socket to monitor events
     * @param userData void*, User data to pass to callback function
     */
    void watchSocket(BaseSocket& socket, void* userData);

    /**
     * Remove socket from monitored pool
     * @param socket BaseSocket&, Socket from this pool
     */
    void forgetSocket(BaseSocket& socket);

    /**
     * @return true if socket pool is active
     */
    [[nodiscard]] bool active() const;
};

}

#endif
