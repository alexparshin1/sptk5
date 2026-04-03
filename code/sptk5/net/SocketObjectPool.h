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

#include "sptk5/net/SocketPool.h"

namespace sptk {

/**
 * Type definition of the socket event callback function.
 */
template<typename T>
using SocketEventCallback = std::function<void(const std::weak_ptr<T>& userData, SocketEventType eventType)>;

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
     * @brief Constructor
     */
    SocketObjectPool(const SocketEventCallback<T>& eventsCallback, SocketPoolTriggerMode triggerMode, size_t maxEvents)
        : SocketPool(triggerMode, maxEvents)
        , m_eventsCallback(eventsCallback)
    {
    }

    /**
     * @brief Destructor
     */
    ~SocketObjectPool() override = default;

    /**
     * @brief Add the socket to the monitored pool
     * @param socket            Socket to monitor events
     * @param userData          User data to pass to the callback function
     * @param rearmOneShot      Re-arm the one-shot event that is already watched. Only used in EdgeTriggered mode.
     */
    void add(const std::shared_ptr<Socket>& socket, const std::shared_ptr<T>& userData, bool rearmOneShot = false)
    {
        if (auto fd = socket->fd();
            fd != INVALID_SOCKET)
        {
            addSocket(socket->fd(), reinterpret_cast<const uint8_t*>(socket.get()), rearmOneShot);
            std::scoped_lock lock(m_mutex);
            m_objects[socket.get()] = {socket, userData};
        }
    }

    /**
     * @brief Remove the socket from the monitored pool
     * @param socket            Socket from this pool
     */
    void remove(const std::shared_ptr<Socket>& socket)
    {
        if (auto fd = socket->fd();
            fd != INVALID_SOCKET)
        {
            removeSocket(socket->fd());
            std::scoped_lock lock(m_mutex);
            m_objects.erase(socket.get());
        }
    }


protected:
    void onEvent(Socket* socket, SocketEventType eventType) override
    {
        if (m_eventsCallback)
        {
            if (auto it = m_objects.find(socket);
                it != m_objects.end())
            {
                m_eventsCallback(it->second.m_userData, eventType);
            }
        }
    }

private:
    std::mutex                                  m_mutex;
    SocketEventCallback<T>                      m_eventsCallback; ///< Sockets event callback function
    std::unordered_map<Socket*, SocketUserData> m_objects;
};

} // namespace sptk
