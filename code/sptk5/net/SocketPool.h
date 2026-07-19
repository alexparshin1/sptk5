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

#include "ServerConnection.h"
#include "sptk5/threads/SynchronizedQueue.h"


#include <sptk5/Buffer.h>
#include <sptk5/Exception.h>
#include <sptk5/net/Socket.h>
#include <sptk5/threads/Thread.h>

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

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
 * @brief Socket event types.
 */
struct SocketEventType
{
    bool m_data : 1;   ///< Socket has data available to read.
    bool m_hangup : 1; ///< Peer closed connection.
    bool m_error : 1;  ///< Connection error.
};

#ifdef _WIN32
#define INVALID_EPOLL nullptr
#else
#define INVALID_EPOLL INVALID_SOCKET
#endif // _WIN32

/**
 * @brief Socket event trigger mode.
 */
enum class SocketPoolTriggerMode
{
    EdgeTriggered, ///< Execute callback once upon new data arrival.
    OneShot,       ///< Execute callback once when data becomes available.
    LevelTriggered ///< Execute callback periodically while data is available.
};

/**
 * @brief Type definition of the socket event callback function.
 *
 * The user object is delivered as a weak_ptr, so the reactor never touches the shared reference
 * count on the hot path; the callback locks it only if it needs to keep the object alive.
 */
template<typename T>
using SocketEventCallback = std::function<void(const std::weak_ptr<T>& userData, SocketEventType eventType)>;

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
     * @brief Constructor.
     * @param triggerMode           Socket event trigger mode.
     * @param maxEvents             Maximum number of socket events per poll.
     */
    explicit SocketPool(SocketPoolTriggerMode triggerMode, size_t maxEvents = 1024);

    /**
     * @brief Deleted copy constructor.
     */
    SocketPool(const SocketPool&) noexcept = delete;

    /**
     * @brief Deleted copy assignment.
     */
    SocketPool& operator=(const SocketPool&) = delete;

    /**
     * @brief Destructor.
     */
    virtual ~SocketPool();

    /**
     * @brief Initialize socket pool.
     */
    void open();

    /**
     * @brief Wait until one or more sockets are signaled.
     *
     * Execute the callback function for each signaled socket.
     * Not reentrant: call it from a single (event loop) thread only.
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

    /**
     * @brief Get trigger mode.
     * @return trigger mode.
     */
    SocketPoolTriggerMode getTriggerMode() const
    {
        return m_triggerMode;
    }

protected:
    /**
     * @brief Add the socket to the monitored pool.
     * @param socketFd            Socket to monitor events.
     * @param userData          User data to pass to the callback function.
     * @param rearmOneShot      Re-arm the one-shot event that is already watched. Only used in EdgeTriggered mode.
     */
    void addSocket(SocketType socketFd, const uint8_t* userData, bool rearmOneShot = false) const;

    /**
     * @brief Remove the socket from the monitored pool.
     * @param socketFd          Socket from this pool.
     */
    void removeSocket(SocketType socketFd) const;

    /**
     * @brief Event callback method.
     * @param eventData         Opaque per-socket cookie supplied to addSocket() (stored in the poll
     *                          event), identifying the signaled socket without a lookup.
     * @param eventType         Event type.
     */
    virtual void onEvent(void* eventData, SocketEventType eventType) = 0;

private:
    /**
     * @brief Socket that controls other sockets events.
     */
#ifdef _WIN32
    HANDLE m_pool {INVALID_EPOLL};
#else
    SocketType m_pool {INVALID_SOCKET};
#endif // _WIN32

    mutable std::mutex    m_mutex;               ///< Mutex for thread-safe operations.
    int                   m_maxEvents;           ///< Maximum number of socket events per poll.
    size_t                m_dispatchThreadCount; ///< Number of threads dispatching events.
    SocketPoolTriggerMode m_triggerMode;         ///< Socket event trigger mode.
    uint32_t              m_baseEvents {0};      ///< Base event mask passed to epoll/kqueue add call.
    Buffer                m_eventsBuffer;        ///< Poll events batch, reused by waitForEvents() calls.

    void dispatchEvents(Buffer& eventsBuffer);
    void processError(int error, const String& operation) const;
};

/**
 * @brief Socket pool that stores user objects in events.
 * @tparam T Socket event object type.
 */
template<typename T>
class SocketObjectPool : public SocketPool
{
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
    }

    /**
     * @brief Destructor.
     */
    ~SocketObjectPool() override = default;

    /**
     * @brief Add the socket to the monitored pool.
     * @param socket            Socket to monitor events.
     * @param userData          User data to pass to the callback function.
     * @param rearmOneShot      Re-arm an already-watched one-shot socket (EPOLL_CTL_MOD) instead of adding it.
     */
    void add(const std::shared_ptr<Socket>& socket, const std::shared_ptr<T>& userData, const bool rearmOneShot = false)
    {
        if (!socket)
        {
            throw Exception("SocketObjectPool::add(): socket is null");
        }

        auto*      socketPtr = socket.get();
        const auto fd = socketPtr->fd();
        if (fd == INVALID_SOCKET)
        {
            return;
        }

        // EPOLL_CTL_MOD (re-arm in place) is only valid when the socket is still registered with the
        // poll.
        const auto rearmInPlace = getTriggerMode() == SocketPoolTriggerMode::OneShot && rearmOneShot;

        // Publish the registration before the kernel registration: an event may be dispatched
        // as soon as addSocket() returns, and onEvent() must find the entry.
        {
            std::scoped_lock lock(m_mutex);
            m_objects[socketPtr] = Registration {socket, userData};
        }

        try
        {
            addSocket(fd, reinterpret_cast<uint8_t*>(socketPtr), rearmInPlace);
        }
        catch (...)
        {
            std::scoped_lock lock(m_mutex);
            m_objects.erase(socketPtr);
            throw;
        }
    }

    /**
     * @brief Re-arm an already-watched one-shot socket (EPOLL_CTL_MOD).
     *
     * The registration created by add() is left untouched, so unlike add() this path takes
     * no lock and copies no shared pointers. Valid only in OneShot mode for a socket that
     * is still registered with this pool; use add() for the first arm.
     * @param socket            Socket previously added to this pool.
     */
    void rearm(const std::shared_ptr<Socket>& socket)
    {
        if (!socket)
        {
            throw Exception("SocketObjectPool::rearm(): socket is null");
        }

        auto*      socketPtr = socket.get();
        const auto fd = socketPtr->fd();
        if (fd == INVALID_SOCKET)
        {
            return;
        }

        addSocket(fd, reinterpret_cast<uint8_t*>(socketPtr), true);
    }

    /**
     * @brief Remove the socket from the monitored pool.
     *
     * Events already collected by the current poll batch may still carry this socket's cookie;
     * they are ignored, and the cookie is used only as a lookup key, never dereferenced.
     * @param socket            Socket from this pool.
     */
    void remove(const std::shared_ptr<Socket>& socket)
    {
        if (!socket)
        {
            throw Exception("SocketObjectPool::remove(): socket is null");
        }

        removeSocket(socket->fd()); // DEL: the kernel delivers no further events for this socket.

        std::shared_ptr<Socket> erased;
        {
            std::scoped_lock lock(m_mutex);
            erased = eraseLocked(socket.get());
        }
        // 'erased' is destroyed here, outside the lock.
    }

protected:
    /**
     * @brief Handle a socket event.
     * @param eventData         Cookie stored at add() time: a pointer to the registered socket.
     * @param eventType         Type of event.
     */
    void onEvent(void* eventData, SocketEventType eventType) override
    {
        auto* socket = static_cast<Socket*>(eventData);
        if (socket == nullptr)
        {
            return;
        }

        std::weak_ptr<T> userData;
        SocketType       fd = INVALID_SOCKET;
        {
            std::scoped_lock lock(m_mutex);
            const auto       it = m_objects.find(socket);
            if (it == m_objects.end())
            {
                // Removed after this batch was collected; whoever removed it did the cleanup.
                return;
            }
            userData = it->second.userData;
            fd = it->second.socket->fd();
        }

        if (userData.expired())
        {
            // The user object is gone: deregister the socket and drop its registration.
            removeSocket(fd); // DEL: the kernel delivers no further events for this socket.

            std::shared_ptr<Socket> erased;
            {
                std::scoped_lock lock(m_mutex);
                erased = eraseLocked(socket);
            }
            return; // 'erased' is destroyed here, outside the lock.
        }

        if (m_eventsCallback)
        {
            m_eventsCallback(userData, eventType);
        }
    }

private:
    /**
     * @brief Per-socket registration.
     */
    struct Registration
    {
        std::shared_ptr<Socket> socket;   ///< Keeps the cookie address valid while registered.
        std::weak_ptr<T>        userData; ///< User data delivered to the callback.
    };

    std::mutex                                m_mutex;          ///< Protects m_objects.
    SocketEventCallback<T>                    m_eventsCallback; ///< Sockets event callback function.
    std::unordered_map<Socket*, Registration> m_objects;        ///< Map of monitored sockets to their registrations.

    /**
     * @brief Erase the socket's registration. Call with m_mutex held.
     * @param socket            Registered socket.
     * @return The co-owned socket, so the caller can destroy it after releasing m_mutex.
     */
    std::shared_ptr<Socket> eraseLocked(Socket* socket)
    {
        std::shared_ptr<Socket> owned;

        const auto it = m_objects.find(socket);
        if (it != m_objects.end())
        {
            owned = std::move(it->second.socket);
            m_objects.erase(it);
        }
        return owned;
    }
};

/**
 * @}
 */

} // namespace sptk
