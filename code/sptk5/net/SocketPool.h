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

#include "ServerConnection.h"
#include "sptk5/threads/SynchronizedQueue.h"


#include <sptk5/Buffer.h>
#include <sptk5/Exception.h>
#include <sptk5/net/Socket.h>

#include <atomic>
#include <cstdint>
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
     * @param token             Opaque per-registration token to pass to the callback function.
     * @param rearmOneShot      Re-arm the one-shot event that is already watched. Only used in EdgeTriggered mode.
     */
    void addSocket(SocketType socketFd, uint64_t token, bool rearmOneShot = false) const;

    /**
     * @brief Remove the socket from the monitored pool.
     * @param socketFd          Socket from this pool.
     */
    void removeSocket(SocketType socketFd) const;

    /**
     * @brief Event callback method.
     * @param token             Opaque per-registration token supplied to addSocket() (stored in the
     *                          poll event), identifying the signaled socket's registration. A fresh,
     *                          never-reused value per addSocket() call - unlike a raw pointer, it
     *                          can't alias a different, later registration if the original socket's
     *                          memory gets freed and reused before a stale batched event is processed.
     * @param eventType         Event type.
     */
    virtual void onEvent(uint64_t token, SocketEventType eventType) = 0;

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
 *
 * A socket may be registered with at most one pool at a time: its registration token lives on the
 * Socket (see Socket::poolToken()), so adding the same socket to a second pool would invalidate the
 * first pool's registration. Registering several sockets that share one user object is fine - the
 * load balancer does exactly that, watching a channel's source and destination in separate pools.
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

        // Every registration gets a fresh, never-reused token instead of using socketPtr itself as
        // the identity: socketPtr is a raw address, and once this Socket is destroyed that address
        // can be handed straight back out to an unrelated Socket by the allocator. A stale event for
        // the old registration, already sitting in a poll batch collected before the removal, would
        // then resolve to the new registration instead of being recognized as stale - delivering one
        // connection's data to a completely different connection. A monotonic token can't alias like
        // that: an old token is simply never found once erased, no matter what address gets reused.
        //
        // The token is kept on the socket itself, so rearm() and remove() find it without a
        // socket-to-token map (and therefore without taking the lock to read it).
        const auto token = m_nextToken.fetch_add(1, std::memory_order_relaxed);

        // Publish the registration before the kernel registration: an event may be dispatched
        // as soon as addSocket() returns, and onEvent() must find the entry.
        {
            std::scoped_lock lock(m_mutex);
            if (const auto previousToken = socketPtr->poolToken(); previousToken != 0)
            {
                // Re-adding an already-tracked socket (only reachable via rearmOneShot=true, unused by
                // current callers but kept correct): drop its old token so it doesn't leak in m_objects.
                m_objects.erase(previousToken);
            }
            m_objects.emplace(token, Registration {socket, userData});
            socketPtr->poolToken(token);
        }

        try
        {
            addSocket(fd, token, rearmInPlace);
        }
        catch (...)
        {
            socketPtr->poolToken(0);
            std::scoped_lock lock(m_mutex);
            m_objects.erase(token);
            throw;
        }
    }

    /**
     * @brief Re-arm an already-watched one-shot socket (EPOLL_CTL_MOD).
     *
     * The registration created by add() is left untouched; this just re-issues the same token
     * already on file for the socket. Valid only in OneShot mode for a socket that is still
     * registered with this pool; use add() for the first arm.
     *
     * Takes no lock: the token lives on the socket, so this is an atomic load plus the
     * epoll_ctl(MOD) syscall. In OneShot mode this runs once per request, on worker threads.
     * @param socket            Socket previously added to this pool.
     */
    void rearm(const std::shared_ptr<Socket>& socket)
    {
        if (!socket)
        {
            throw Exception("SocketObjectPool::rearm(): socket is null");
        }

        const auto fd = socket->fd();
        if (fd == INVALID_SOCKET)
        {
            return;
        }

        const auto token = socket->poolToken();
        if (token == 0)
        {
            throw Exception("SocketObjectPool::rearm(): socket is not registered");
        }

        addSocket(fd, token, true);
    }

    /**
     * @brief Remove the socket from the monitored pool.
     *
     * Events already collected by the current poll batch may still carry this socket's token;
     * they are ignored (the token is a lookup key only, never dereferenced), and since tokens are
     * never reused, a stale one can never resolve to some other, later registration.
     *
     * Removing a socket that isn't registered is a no-op: callers legitimately overlap (a
     * connection may be unwatched and then closed), and claiming the token up front means the
     * redundant calls cost neither a syscall nor the lock.
     * @param socket            Socket from this pool.
     */
    void remove(const std::shared_ptr<Socket>& socket)
    {
        if (!socket)
        {
            throw Exception("SocketObjectPool::remove(): socket is null");
        }

        const auto token = socket->takePoolToken();
        if (token == 0)
        {
            return;
        }

        removeSocket(socket->fd()); // DEL: the kernel delivers no further events for this socket.

        std::shared_ptr<Socket> erased;
        {
            std::scoped_lock lock(m_mutex);
            erased = eraseLocked(token);
        }
        // 'erased' is destroyed here, outside the lock.
    }

protected:
    /**
     * @brief Handle a socket event.
     * @param token             Token stored at add() time, identifying the registration.
     * @param eventType         Type of event.
     */
    void onEvent(const uint64_t token, SocketEventType eventType) override
    {
        std::weak_ptr<T>        userData;
        SocketType              fd = INVALID_SOCKET;
        std::shared_ptr<Socket> socket;
        {
            std::scoped_lock lock(m_mutex);
            const auto       it = m_objects.find(token);
            if (it == m_objects.end())
            {
                // Removed after this batch was collected; whoever removed it did the cleanup.
                return;
            }
            userData = it->second.userData;
            socket = it->second.socket;
            fd = socket->fd();
        }

        if (userData.expired())
        {
            // The user object is gone: deregister the socket and drop its registration. Claim the
            // token first, so a concurrent remove() doesn't do the same work twice - and so that a
            // socket already removed and re-added under a newer token keeps that newer
            // registration, since only the token this event carries is released.
            if (!socket->releasePoolToken(token))
            {
                return;
            }

            removeSocket(fd); // DEL: the kernel delivers no further events for this socket.

            std::shared_ptr<Socket> erased;
            {
                std::scoped_lock lock(m_mutex);
                erased = eraseLocked(token);
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
        std::shared_ptr<Socket> socket;   ///< Keeps the socket alive while registered.
        std::weak_ptr<T>        userData; ///< User data delivered to the callback.
    };

    std::mutex                                 m_mutex;          ///< Protects m_objects.
    SocketEventCallback<T>                     m_eventsCallback; ///< Sockets event callback function.
    std::unordered_map<uint64_t, Registration> m_objects;        ///< Map of registration tokens to their registrations.
    std::atomic<uint64_t>                      m_nextToken {1};  ///< Never-reused per-registration token generator.

    /**
     * @brief Erase a registration. Call with m_mutex held.
     * @param token             Registration token, as claimed from the socket.
     * @return The co-owned socket, so the caller can destroy it after releasing m_mutex.
     */
    std::shared_ptr<Socket> eraseLocked(const uint64_t token)
    {
        std::shared_ptr<Socket> owned;

        if (const auto it = m_objects.find(token); it != m_objects.end())
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
