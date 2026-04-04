/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <mutex>
#include <shared_mutex>

#ifdef _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#endif

namespace sptk {

/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * Socket event types
 */
struct SocketEventType
{
    bool m_data : 1;   ///< Socket has data available to read
    bool m_hangup : 1; ///< Peer closed connection
    bool m_error : 1;  ///< Connection error
};

#ifdef _WIN32
#define INVALID_EPOLL nullptr
#else
#define INVALID_EPOLL INVALID_SOCKET
#endif // _WIN32

/**
 * @brief Socket event trigger mode
 */
enum class SocketPoolTriggerMode
{
    EdgeTriggered, ///< Execute callback once upon new data arrival
    OneShot,       ///< Execute callback once when data becomes available
    LevelTriggered ///< Execute callback periodically while data is available
};

/**
 * @brief Type definition of the socket event callback function.
 */
template<typename T>
using SocketEventCallback = std::function<void(const std::shared_ptr<T>& userData, SocketEventType eventType)>;

/**
 * @brief Socket event manager.
 *
 * Uses OS-specific implementation.
 * On Linux it is using epoll, on BSD it is using kqueue,
 * and on Windows WSAAsyncSelect is used.
 */
class SP_EXPORT SocketPool
{
public:
    /**
     * @brief Constructor
     * @param triggerMode    Socket event trigger mode
     * @param maxEvents      Maximum number of socket events per poll
     */
    explicit SocketPool(SocketPoolTriggerMode triggerMode, size_t maxEvents = 1024);

    /**
     * @brief Deleted copy constructor
     */
    SocketPool(const SocketPool&) noexcept = delete;

    /**
     * Deleted copy assignment
     */
    SocketPool& operator=(const SocketPool&) = delete;

    /**
     * @brief Destructor.
     */
    virtual ~SocketPool();

    /**
     * @brief Initialize socket pool
     */
    void open();

    /**
     * Wait until one or more sockets are signaled.
     *
     * Execute the callback function for each signaled socket.
     */
    bool waitForEvents(const std::chrono::milliseconds& timeout);

    /**
     * @brief Shutdown socket pool.
     */
    void close();

    /**
     * @return true if the socket pool is active.
     */
    [[nodiscard]] bool active() const;

protected:
    /**
     * @brief Add the socket to the monitored pool
     * @param socketFd            Socket to monitor events
     * @param userData          User data to pass to the callback function
     * @param rearmOneShot      Re-arm the one-shot event that is already watched. Only used in EdgeTriggered mode.
     */
    void addSocket(SocketType socketFd, const uint8_t* userData, bool rearmOneShot = false);

    /**
     * @brief Remove the socket from the monitored pool
     * @param socketFd            Socket from this pool
     */
    void removeSocket(SocketType socketFd) const;

    virtual void onEvent(Socket* socket, SocketEventType eventType) = 0;

private:
    /**
     * Socket that controls other sockets events
     */
#ifdef _WIN32
    HANDLE m_pool {INVALID_EPOLL};
#else
    SocketType m_pool {INVALID_SOCKET};
#endif // _WIN32

    mutable std::mutex    m_mutex;        ///< Mutex for thread-safe operations.
    size_t                m_maxEvents;    ///< Maximum number of socket events per poll.
    int                   m_maxEventsInt; ///< Maximum number of socket events per poll, int cache for syscalls.
    Buffer                m_eventsBuffer; ///< Socket events.
    SocketPoolTriggerMode m_triggerMode;  ///< Socket event trigger mode.
    uint32_t              m_baseEvents;   ///< Base event mask passed to epoll/kqueue add call.

    void processError(int error, const String& operation) const;
};

/**
 * @brief Socket pool that stores user objects in events.
 * @tparam T Socket event object type.
 */
template<typename T>
class SP_EXPORT SocketObjectPool : public SocketPool
{
    struct SocketUserData
    {
        std::shared_ptr<Socket> m_socket;
        std::weak_ptr<T>        m_userData;
    };

public:
    /**
     * @brief Constructor.
     * @param eventsCallback    Socket event callback function.
     * @param triggerMode       Socket event trigger mode.
     * @param maxEvents         Maximum number of socket events per poll.
     */
    SocketObjectPool(const SocketEventCallback<T>& eventsCallback, const SocketPoolTriggerMode triggerMode, size_t maxEvents)
        : SocketPool(triggerMode, maxEvents)
        , m_eventsCallback(eventsCallback)
    {
        m_objects.reserve(maxEvents);
    }

    /**
     * @brief Destructor.
     */
    ~SocketObjectPool() override = default;

    /**
     * @brief Add the socket to the monitored pool.
     * @param socket            Socket to monitor events.
     * @param userData          User data to pass to the callback function.
     * @param rearmOneShot      Re-arm the one-shot event that is already watched. Only used in EdgeTriggered mode.
     */
    void add(const std::shared_ptr<Socket>& socket, const std::shared_ptr<T>& userData, const bool rearmOneShot = false)
    {
        if (!socket)
        {
            throw Exception("SocketObjectPool::add(): socket is null");
        }

        auto* socketPtr = socket.get();
        if (const auto fd = socketPtr->fd();
            fd != INVALID_SOCKET)
        {
            setSocketUserData(socketPtr, socket, userData);
            try
            {
                addSocket(fd, reinterpret_cast<const uint8_t*>(socketPtr), rearmOneShot);
            }
            catch (const Exception&)
            {
                removeSocketUserData(socketPtr);
                throw;
            }
        }
    }

    /**
     * @brief Remove the socket from the monitored pool.
     * @param socket            Socket from this pool.
     */
    void remove(const std::shared_ptr<Socket>& socket)
    {
        if (!socket)
        {
            throw Exception("SocketObjectPool::remove(): socket is null");
        }

        auto* socketPtr = socket.get();
        removeSocketUserData(socketPtr);

        if (const auto fd = socketPtr->fd();
            fd != INVALID_SOCKET)
        {
            removeSocket(fd);
        }
    }

protected:
    /**
     * @brief Handle socket events.
     * @param socket            Socket that triggered the event.
     * @param eventType         Type of event.
     */
    void onEvent(Socket* socket, SocketEventType eventType) override
    {
        if (m_eventsCallback)
        {
            auto weakUserData = findSocketUserData(socket);
            if (const auto userData = weakUserData.lock())
            {
                m_eventsCallback(userData, eventType);
            }
        }
    }

private:
    mutable std::shared_mutex                   m_mutex;          ///< Mutex that protects the socket object pool.
    SocketEventCallback<T>                      m_eventsCallback; ///< Sockets event callback function.
    std::unordered_map<Socket*, SocketUserData> m_objects;        ///< Socket object pool.

    /**
     * @brief Set the user data for a socket.
     * @param socket            The socket to set the user data for.
     * @param userData          The user data to set.
     */
    void setSocketUserData(Socket* socketPtr, const std::shared_ptr<Socket>& socket, const std::shared_ptr<T>& userData)
    {
        const std::unique_lock lock(m_mutex);
        m_objects[socketPtr] = {socket, userData};
    }

    /**
     * @brief Find the user data for a socket.
     * @param socket            The socket to find the user data for.
     */
    std::weak_ptr<T> findSocketUserData(Socket* socket)
    {
        const std::shared_lock lock(m_mutex);
        const auto             it = m_objects.find(socket);
        if (it != m_objects.end())
        {
            return it->second.m_userData;
        }
        return {};
    }

    /**
     * @brief Remove the user data for a socket.
     * @param socket            The socket to remove the user data for.
     */
    void removeSocketUserData(Socket* socketPtr)
    {
        const std::unique_lock lock(m_mutex);
        m_objects.erase(socketPtr);
    }
};

/**
 * @}
 */

} // namespace sptk
