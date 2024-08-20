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

#include <sptk5/Exception.h>
#include <sptk5/net/Socket.h>
#include <sptk5/threads/Thread.h>

#include <functional>
#include <map>
#include <mutex>

#ifdef _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#endif

namespace sptk {

enum class SocketEventAction
{
    Continue,
    Disable,
    Forget
};

/**
 * Socket event types
 */
struct SocketEventType
{
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
        EdgeTriggered, ///< Execute callback once upon new data arrival
        OneShot,       ///< Execute callback once when data becomes available
        LevelTriggered ///< Execute callback periodically while data is available
    };

    /**
     * Constructor
     * @param eventsCallback SocketEventCallback, Callback function executed upon socket events
     * @param triggerMode    Socket event trigger mode
     */
    explicit SocketPool(SocketEventCallback eventsCallback, TriggerMode triggerMode);

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
     */
    void watchSocket(Socket& socket, const uint8_t* userData);

    /**
     * Remove socket from monitored pool
     * @param socket            Socket from this pool
     */
    void forgetSocket(Socket& socket);

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
    SocketEventCallback               m_eventsCallback; ///< Sockets event callback function
    static const int                  maxEvents = 128;  ///< Maximum number of socket events per poll
    TriggerMode                       m_triggerMode;    ///< Socket event trigger mode
    std::map<Socket*, const uint8_t*> m_userData;       ///< User data related to socket

    void              processError(int error, const String& operation) const;
    SocketEventAction executeEventAction(Socket* socket, SocketEventType eventType);
};

} // namespace sptk
