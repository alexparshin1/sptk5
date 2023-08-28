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

#include <sptk5/Exception.h>
#include <sptk5/net/Socket.h>
#include <sptk5/threads/Thread.h>

#include <functional>
#include <map>
#include <mutex>

#ifdef _WIN32

// Windows
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <wepoll.h>
using SocketEventBase = epoll_event;

#else

#if __linux__ == 1
// Linux
#include <sys/epoll.h>

using SocketEventBase = epoll_event;

#else
// BSD
#include <sys/event.h>
using SocketEventBase = struct kevent;

#endif
#endif

namespace sptk {

class SocketEvent : public SocketEventBase
{
public:
    bool m_enabled {true};
};

enum class SocketEventAction
{
    Continue,
    Disable,
    Forget
};

/**
 * Socket event types
 */
struct SocketEventType {
    bool m_data : 1;   ///< Socket has data available to read
    bool m_hangup : 1; ///< Peer closed connection
    bool m_error : 1;  ///< Connection error
};

/**
 * Type definition of socket event callback function
 */
using SocketEventCallback = std::function<SocketEventAction(const uint8_t* userData, SocketEventType eventType)>;

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
class SP_EXPORT SocketPool
    : public std::mutex
{
public:
    /**
     * @brief Socket event trigger mode
     */
    enum class TriggerMode
    {
        EdgeTriggered,  ///< Execute callback once upon data arrival
        LevelTriggered, ///< Execute callback after each data arrival
        OneShot         ///< Execute callback once and stop monitoring
    };

    /**
     * Constructor
     * @param eventsCallback SocketEventCallback, Callback function executed upon socket events
     */
    explicit SocketPool(SocketEventCallback eventsCallback);

    /**
     * Deleted copy constructor
     */
    SocketPool(const SocketPool&) noexcept = delete;

    /**
     * Deleted copy assignment
     */
    SocketPool& operator=(const SocketPool&) = delete;

    /**
     * Initialize socket pool
     */
    void open();

    /**
     * Destructor
     */
    ~SocketPool();

    /**
     * Wait until one or more sockets are signaled.
     *
     * Execute callback function for each signaled socket.
     */
    bool waitForEvents(std::chrono::milliseconds timeout);

    /**
     * Shutdown socket pool.
     */
    void close();

    /**
     * @brief Add socket to monitored pool
     * @param socket            Socket to monitor events
     * @param userData          User data to pass to callback function
     * @param edgeTrigerred     If true, use edge-triggered events
     */
    void watchSocket(Socket& socket, const uint8_t* userData);

    /**
     * Remove socket from monitored pool
     * @param socket            Socket from this pool
     */
    void forgetSocket(Socket& socket);

    /**
     * Check if socket is already being monitored
     * @param socket            Socket
     */
    bool hasSocket(Socket& socket);

    /**
     * @brief Enable socket events if socket is already being monitored
     * @param socket            Socket
     */
    void enableSocketEvents(Socket& socket);

    /**
     * @brief Disable socket events if socket is already being monitored
     * @param socket            Socket
     */
    void disableSocketEvents(Socket& socket);

    /**
     * @return true if socket pool is active
     */
    [[nodiscard]] bool active() const;

private:
    /**
     * Socket that controls other sockets events
     */
#ifdef _WIN32
    HANDLE m_pool {INVALID_EPOLL};
#else
    SocketType m_pool {INVALID_SOCKET};
#endif // _WIN32

    /**
     * Callback function executed upon socket events
     */
    SocketEventCallback m_eventsCallback;

    /**
     * Map of sockets to corresponding user data
     */
    std::map<Socket*, SocketEvent> m_socketData;

    static const int maxEvents = 128;

    /**
     * Events received by epoll
     */
    std::array<SocketEventBase, maxEvents> m_events {};
};

} // namespace sptk
