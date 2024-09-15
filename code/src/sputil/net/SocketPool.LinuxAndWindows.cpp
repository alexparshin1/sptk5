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

#include <sptk5/Printer.h>
#include <sptk5/SystemException.h>
#include <sptk5/net/SocketPool.h>

#ifdef _WIN32
#include "../wepoll/wepoll.h"
#else
#include <sys/epoll.h>
#endif

using SocketEventBase = epoll_event;

class SocketEvent : public SocketEventBase
{
public:
    bool m_enabled {true};
};

using namespace std;
using namespace sptk;

void SocketPool::open()
{
    const scoped_lock lock(*this);

    if (m_pool != INVALID_EPOLL)
    {
        return;
    }

    m_pool = epoll_create1(0);

    if (m_pool == INVALID_EPOLL)
    {
        throw SystemException("Can't create epoll");
    }
}

void SocketPool::close()
{
    const scoped_lock lock(*this);

    if (m_pool != INVALID_EPOLL)
    {
#ifdef _WIN32
        epoll_close(m_pool);
#else
        ::close(m_pool);
#endif
        m_pool = INVALID_EPOLL;
    }
}

void SocketPool::watchSocket(Socket& socket, const uint8_t* userData)
{
    const scoped_lock lock(*this);

    m_userData[&socket] = userData;

    SocketEvent event;
    event.data.ptr = bit_cast<uint8_t*>(&socket);
    event.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR;
    switch (m_triggerMode)
    {
        case TriggerMode::EdgeTriggered:
#ifdef _WIN32
            throw Exception("Edge triggered mode isn't supported on Windows");
#else
            event.events |= EPOLLET;
            break;
#endif
        case TriggerMode::OneShot:
            event.events |= EPOLLONESHOT;
            break;
        case TriggerMode::LevelTriggered:
            break;
    }

    if (epoll_ctl(m_pool, EPOLL_CTL_ADD, socket.fd(), &event) == -1)
    {
        processError(errno, "add socket to SocketEvents");
    }
}

void SocketPool::forgetSocket(Socket& socket)
{
    const lock_guard lock(*this);

    SocketEvent event;
    if (socket.active())
    {
        epoll_ctl(m_pool, EPOLL_CTL_DEL, socket.fd(), &event);
    }

    m_userData.erase(&socket);
}

SocketEventAction SocketPool::executeEventAction(Socket* socket, SocketEventType eventType)
{
    const uint8_t* userData;
    {
        const lock_guard lock(*this);
        const auto       iterator = m_userData.find(socket);
        if (iterator == m_userData.end())
        {
            return SocketEventAction::Disable;
        }
        userData = iterator->second;
    }
    return m_eventsCallback(bit_cast<uint8_t*>(userData), eventType);
}

bool SocketPool::waitForEvents(chrono::milliseconds timeout)
{
    std::array<epoll_event, maxEvents> events {};
    const int                          eventCount = epoll_wait(m_pool, events.data(), maxEvents, static_cast<int>(timeout.count()));
    if (eventCount < 0)
    {
        return m_pool != INVALID_EPOLL;
    }

    for (int i = 0; i < eventCount; ++i)
    {
        auto& event = events[i];

        const SocketEventType eventType {
            .m_data = (event.events & EPOLLIN) != 0,
            .m_hangup = (event.events & (EPOLLHUP | EPOLLRDHUP)) != 0,
            .m_error = (event.events & EPOLLERR) != 0,
        };

        if (auto* socket = bit_cast<Socket*>(event.data.ptr))
        {
            if (isSocketRegistered(socket) && executeEventAction(socket, eventType) == SocketEventAction::Disable)
            {
                // Disable events for the socket
                event.events = EPOLLHUP | EPOLLRDHUP | EPOLLERR;
                epoll_ctl(m_pool, EPOLL_CTL_MOD, socket->fd(), &event);
            }
        }
    }

    return true;
}

void SocketPool::processError(int error, const String& operation) const
{
    switch (error)
    {
        case EBADF:
            if (m_pool == INVALID_EPOLL)
            {
                throw SystemException("SocketPool is not open");
            }
            throw SystemException("Socket is closed");

        case EINVAL:
            throw Exception("Invalid event");

        case EEXIST:
            // Socket is already being monitored
            break;

        default:
            throw SystemException("Can't " + operation);
    }
}
