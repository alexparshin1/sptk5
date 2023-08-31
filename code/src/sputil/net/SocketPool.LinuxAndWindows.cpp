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

#include <sptk5/Printer.h>
#include <sptk5/SystemException.h>
#include <sptk5/net/SocketPool.h>

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

    m_socketData.clear();
}

void SocketPool::watchSocket(Socket& socket, const uint8_t* userData)
{
    const scoped_lock lock(*this);

    const auto& [itor, inserted] = m_socketData.emplace(&socket, userData);
    if (!inserted)
    {
        return;
    }

    SocketEvent event;
    event.data.ptr = bit_cast<uint8_t*>(&socket);
    event.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR;

    if (epoll_ctl(m_pool, EPOLL_CTL_ADD, socket.fd(), &event) == -1)
    {
        processError(errno);
    }
}

void SocketPool::forgetSocket(Socket& socket)
{
    const lock_guard lock(*this);

    auto itor = m_socketData.find(&socket);
    if (itor == m_socketData.end())
    {
        return;
    }

    m_socketData.erase(itor);

    SocketEvent event;
    if (socket.active() && epoll_ctl(m_pool, EPOLL_CTL_DEL, socket.fd(), &event) == -1)
    {
        if (errno != ENOENT)
        {
            throw SystemException("Can't remove socket from epoll");
        }
    }
}

bool SocketPool::waitForEvents(chrono::milliseconds timeout)
{
    const int eventCount = epoll_wait(m_pool, m_events.data(), maxEvents, (int) timeout.count());
    if (eventCount < 0)
    {
        if (m_pool == INVALID_EPOLL)
        {
            return false;
        }

        return true;
    }

    for (int i = 0; i < eventCount; ++i)
    {
        epoll_event& event = m_events[i];
        SocketEventType eventType {};

        eventType.m_data = event.events & EPOLLIN;
        eventType.m_hangup = event.events & (EPOLLHUP | EPOLLRDHUP);
        eventType.m_error = event.events & EPOLLERR;

        auto* socket = bit_cast<Socket*>(event.data.ptr);

        auto itor = m_socketData.find(socket);
        if (itor == m_socketData.end())
        {
            continue;
        }
        const auto* userData = itor->second;

        auto eventAction = m_eventsCallback(bit_cast<uint8_t*>(userData), eventType);
        if (eventAction == SocketEventAction::Disable)
        {
            // Disable events for the socket
            COUT("DISABLE" << endl);
            event.events = EPOLLHUP | EPOLLRDHUP | EPOLLERR;
            if (epoll_ctl(m_pool, EPOLL_CTL_MOD, socket->fd(), &event) == -1)
            {
                processError(errno);
            }
        }
    }

    return true;
}

void SocketPool::enableSocketEvents(Socket& socket)
{
    SocketEvent event;
    event.data.ptr = bit_cast<uint8_t*>(&socket);
    event.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR;

    if (epoll_ctl(m_pool, EPOLL_CTL_MOD, socket.fd(), &event) == -1)
    {
        processError(errno);
    }
}

void SocketPool::disableSocketEvents(Socket& socket)
{
    SocketEvent event;
    event.data.ptr = bit_cast<uint8_t*>(&socket);
    event.events = EPOLLHUP | EPOLLRDHUP | EPOLLERR;

    if (epoll_ctl(m_pool, EPOLL_CTL_MOD, socket.fd(), &event) == -1)
    {
        processError(errno);
    }
}

void SocketPool::processError(int error) const
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
            throw SystemException("Can't add or remove socket to/from epoll");
    }
}
