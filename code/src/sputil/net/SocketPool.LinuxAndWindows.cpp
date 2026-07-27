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

#include <sptk5/Printer.h>
#include <sptk5/SystemException.h>
#include <sptk5/net/SocketPool.h>

#ifdef _WIN32
#include "../wepoll/wepoll.h"
#else
#include <sys/epoll.h>
#endif

using SocketEvent = epoll_event;

using namespace std;
using namespace sptk;

void SocketPool::open()
{
    const scoped_lock lock(m_mutex);

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
    const scoped_lock lock(m_mutex);

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

void SocketPool::addSocket(const SocketType socketFd, const uint64_t token, const bool rearmOneShot) const
{
    SocketEvent event {.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR, .data = {.u64 = token}};
    switch (m_triggerMode)
    {
        using enum SocketPoolTriggerMode;
        case EdgeTriggered:
#ifdef _WIN32
            throw Exception("Edge triggered mode isn't supported on Windows");
#else
            event.events |= EPOLLET;
            break;
#endif
        case OneShot:
            event.events |= EPOLLONESHOT;
            break;
        case LevelTriggered:
            break;
    }

    if (m_triggerMode == SocketPoolTriggerMode::OneShot && rearmOneShot)
    {
        if (epoll_ctl(m_pool, EPOLL_CTL_MOD, socketFd, &event) == -1)
        {
            processError(errno, "rearm socket in SocketEvents");
        }
    }
    else
    {
        if (epoll_ctl(m_pool, EPOLL_CTL_ADD, socketFd, &event) == -1)
        {
            processError(errno, "add socket to SocketEvents");
        }
    }
}

void SocketPool::removeSocket(const SocketType socketFd) const
{
    if (socketFd != INVALID_SOCKET)
    {
        epoll_ctl(m_pool, EPOLL_CTL_DEL, socketFd, nullptr);
    }
}

bool SocketPool::waitForEvents(const chrono::milliseconds& timeout)
{
    m_eventsBuffer.reserve(sizeof(epoll_event) * m_maxEvents);
    auto* events = reinterpret_cast<epoll_event*>(m_eventsBuffer.data());

    const auto eventCount = epoll_wait(m_pool, events, m_maxEvents, static_cast<int>(timeout.count()));
    if (eventCount < 0)
    {
        return m_pool != INVALID_EPOLL;
    }
    m_eventsBuffer.bytes(sizeof(epoll_event) * eventCount);

    dispatchEvents(m_eventsBuffer);

    return true;
}

void SocketPool::dispatchEvents(Buffer& eventsBuffer)
{
    auto*      events = reinterpret_cast<epoll_event*>(eventsBuffer.data());
    const auto eventCount = eventsBuffer.size() / sizeof(epoll_event);
    for (size_t i = 0; i < eventCount; ++i)
    {
        auto& [event, data] = events[i];

        const SocketEventType eventType {
            .m_data = (event & EPOLLIN) != 0,
            .m_hangup = (event & (EPOLLHUP | EPOLLRDHUP)) != 0,
            .m_error = (event & EPOLLERR) != 0,
        };

        onEvent(data.u64, eventType);
    }
}


void SocketPool::processError(const int error, const String& operation) const
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
            throw SystemException("Invalid event");

        case EEXIST:
            // Socket is already being monitored
            break;

        default:
            throw SystemException("Can't " + operation);
    }
}
