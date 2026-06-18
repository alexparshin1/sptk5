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

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

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
 * The user object is delivered as a weak_ptr so the reactor never touches the shared reference
 * count on the hot path; the callback locks it only if it needs to keep the object alive.
 */
template<typename T>
using SocketEventCallback = std::function<void(const std::weak_ptr<T>& userData, SocketEventType eventType)>;

/**
 * @brief Socket event manager.
 *
 * Uses OS-specific implementation.
 * On Linux it is using epoll, on BSD it is using kqueue,.
 * and on Windows WSAAsyncSelect is used.
 */
class SP_EXPORT SocketPool
{
public:
    /**
     * @brief Constructor.
     * @param triggerMode    Socket event trigger mode.
     * @param maxEvents      Maximum number of socket events per poll.
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
    void addSocket(SocketType socketFd, const uint8_t* userData, bool rearmOneShot = false);

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

    mutable std::mutex    m_mutex;            ///< Mutex for thread-safe operations.
    size_t                m_maxEvents;        ///< Maximum number of socket events per poll.
    int                   m_maxEventsInt {0}; ///< Maximum number of socket events per poll, int cache for syscalls.
    Buffer                m_eventsBuffer;     ///< Socket events.
    SocketPoolTriggerMode m_triggerMode;      ///< Socket event trigger mode.
    uint32_t              m_baseEvents {0};   ///< Base event mask passed to epoll/kqueue add call.

    void processError(int error, const String& operation) const;
};

/**
 * @brief Socket pool that stores user objects in events.
 * @tparam T Socket event object type.
 */
template<typename T>
class SocketObjectPool : public SocketPool
{
    /**
     * @brief Per-socket registration.
     *
     * Its heap address is used directly as the poll-event cookie, so event dispatch needs neither a
     * map lookup nor a lock. The map only stores it (via unique_ptr, keeping the address stable
     * across rehashes) so add()/remove() can find it. Retired registrations are freed by the reactor
     * thread between event batches (reclaimRetired()), so no in-flight event references freed memory.
     */
    struct SocketRegistration
    {
        std::shared_ptr<Socket> m_socket;        ///< Keeps the socket alive while monitored.
        std::weak_ptr<T>        m_userData;      ///< User object delivered to the callback.
        std::atomic_bool        m_active {true}; ///< Cleared by remove(): suppresses any in-flight event still referencing this registration.
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

        SocketRegistration* registration = nullptr;
        {
            const std::scoped_lock lock(m_mutex);
            const auto             it = m_objects.find(socketPtr);
            if (rearmOneShot)
            {
                if (it == m_objects.end())
                {
                    return; // Socket already removed: nothing to re-arm.
                }
                registration = it->second.get(); // Reuse the existing cookie.
            }
            else
            {
                if (it != m_objects.end())
                {
                    // Re-adding a still-registered socket: retire the old registration rather than
                    // overwriting it, so an in-flight event referencing it stays valid until reclaim.
                    it->second->m_active.store(false);
                    m_retired.push_back(std::move(it->second));
                    m_objects.erase(it);
                }
                auto created = std::make_unique<SocketRegistration>();
                created->m_socket = socket;
                created->m_userData = userData;
                registration = created.get();
                m_objects[socketPtr] = std::move(created);
            }
        }

        try
        {
            addSocket(fd, reinterpret_cast<const uint8_t*>(registration), rearmOneShot);
        }
        catch (const Exception&)
        {
            if (!rearmOneShot)
            {
                const std::scoped_lock lock(m_mutex);
                m_objects.erase(socketPtr);
            }
            throw;
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

        {
            const std::scoped_lock lock(m_mutex);
            if (const auto it = m_objects.find(socketPtr);
                it != m_objects.end())
            {
                // Suppress any event for this socket still in the batch the reactor is dispatching
                // (the pre-change semantics: a removed socket delivers no more events), then retire
                // the registration. reclaimRetired() frees it once that batch is fully processed.
                it->second->m_active.store(false);
                m_retired.push_back(std::move(it->second));
                m_objects.erase(it);
            }
        }

        if (const auto fd = socketPtr->fd();
            fd != INVALID_SOCKET)
        {
            removeSocket(fd); // DEL: the kernel delivers no further events for this socket.
        }
    }

protected:
    /**
     * @brief Handle a socket event.
     * @param eventData         Cookie stored at add() time: a pointer to the socket's registration.
     * @param eventType         Type of event.
     */
    void onEvent(void* eventData, SocketEventType eventType) override
    {
        // Lock-free hot path: the cookie points straight at the registration, which stays valid
        // until reclaimRetired() runs between batches. The weak_ptr is passed through without being
        // locked here; the callback locks it only if it needs the object.
        auto* registration = static_cast<SocketRegistration*>(eventData);
        if (registration == nullptr || !registration->m_active.load())
        {
            return; // Socket was removed: drop this in-flight event.
        }
        if (m_eventsCallback)
        {
            m_eventsCallback(registration->m_userData, eventType);
        }
    }

    /**
     * @brief Free registrations retired since the previous call.
     *
     * Must be called by the reactor thread between event batches (the previous batch fully
     * dispatched, the next wait not yet started), so no in-flight event can reference freed memory.
     */
    void reclaimRetired()
    {
        std::vector<std::unique_ptr<SocketRegistration>> retired;
        {
            const std::scoped_lock lock(m_mutex);
            if (m_retired.empty())
            {
                return;
            }
            retired.swap(m_retired);
        }
        // Registrations destroyed here, outside the lock and between batches.
    }

private:
    std::mutex                                                       m_mutex;          ///< Protects m_objects and m_retired.
    SocketEventCallback<T>                                           m_eventsCallback; ///< Sockets event callback function.
    std::unordered_map<Socket*, std::unique_ptr<SocketRegistration>> m_objects;        ///< Active registrations (stable addresses used as cookies).
    std::vector<std::unique_ptr<SocketRegistration>>                 m_retired;         ///< Registrations awaiting reclamation by the reactor thread.
};

/**
 * @}
 */

} // namespace sptk
