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

using SocketEvent = epoll_event;

using namespace std;
using namespace sptk;

void SocketPool::open()
{
    const scoped_lock lock(*this);

    m_eventsBuffer.checkSize(m_maxEvents * sizeof(epoll_event));

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

void SocketPool::add(Socket& socket, const uint8_t* userData, bool rearmOneShot)
{
    SocketEvent event {.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR, .data = {.ptr = bit_cast<uint8_t*>(userData)}};
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

    if (m_triggerMode == TriggerMode::OneShot && rearmOneShot)
    {
        if (epoll_ctl(m_pool, EPOLL_CTL_MOD, socket.fd(), &event) == -1)
        {
            processError(errno, "rearm socket in SocketEvents");
        }
    }
    else
    {
        if (epoll_ctl(m_pool, EPOLL_CTL_ADD, socket.fd(), &event) == -1)
        {
            processError(errno, "add socket to SocketEvents");
        }
    }
}

void SocketPool::remove(Socket& socket) const
{
    if (socket.active())
    {
        epoll_ctl(m_pool, EPOLL_CTL_DEL, socket.fd(), nullptr);
    }
}

bool SocketPool::waitForEvents(const chrono::milliseconds& timeout)
{
    auto* events = reinterpret_cast<epoll_event*>(m_eventsBuffer.data());

    const int eventCount = epoll_wait(m_pool, events, static_cast<int>(m_maxEvents), static_cast<int>(timeout.count()));
    if (eventCount < 0)
    {
        return m_pool != INVALID_EPOLL;
    }

    for (int i = 0; i < eventCount; ++i)
    {
        auto& [event, data] = events[i];

        const SocketEventType eventType {
            .m_data = (event & EPOLLIN) != 0,
            .m_hangup = (event & (EPOLLHUP | EPOLLRDHUP)) != 0,
            .m_error = (event & EPOLLERR) != 0,
        };

        m_eventsCallback(static_cast<const uint8_t*>(data.ptr), eventType);
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
            throw SystemException("Invalid event");

        case EEXIST:
            // Socket is already being monitored
            break;

        default:
            throw SystemException("Can't " + operation);
    }
}
