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

#include <utility>

using namespace std;
using namespace sptk;

SocketPool::SocketPool(SocketEventCallback eventsCallback)
    : m_eventsCallback(std::move(eventsCallback))
{
}

SocketPool::~SocketPool()
{
    close();
}

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
        throw SystemException("epoll_create1");
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
    const lock_guard lock(*this);
    const uint32_t eventMask = EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR;

    auto socketFD = socket.fd();
    if (socketFD == INVALID_SOCKET)
    {
        throw Exception("Socket is closed");
    }

    auto event = make_shared<epoll_event>();
    event->data.ptr = const_cast<uint8_t*>(userData);
    event->events = eventMask;

    if (epoll_ctl(m_pool, EPOLL_CTL_ADD, socketFD, event.get()) == -1)
    {
        if (m_pool == INVALID_EPOLL)
        {
            throw SystemException("SocketPool is not open");
        }

        throw SystemException("Can't add socket to SocketPool");
    }

    m_socketData[&socket] = event;
}

void SocketPool::forgetSocket(Socket& socket)
{
    const lock_guard lock(*this);

    auto itor = m_socketData.find(&socket);
    if (itor == m_socketData.end())
    {
        return;
    }

    auto event = itor->second;
    m_socketData.erase(itor);

    if (socket.active() &&
        epoll_ctl(m_pool, EPOLL_CTL_DEL, socket.fd(), event.get()) == -1)
    {
        throw SystemException("Can't remove socket from epoll");
    }
}

bool SocketPool::hasSocket(Socket& socket)
{
    const scoped_lock lock(*this);

    auto itor = m_socketData.find(&socket);
    return itor != m_socketData.end();
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
        const epoll_event& event = m_events[i];
        SocketEventType eventType {};

        if (event.events & EPOLLIN) [[likely]]
        {
            eventType.m_data = true;
        }

        if (event.events & (EPOLLHUP | EPOLLRDHUP)) [[unlikely]]
        {
            eventType.m_hangup = true;
        }

        if (event.events & EPOLLERR) [[unlikely]]
        {
            eventType.m_error = true;
        }

        m_eventsCallback(static_cast<uint8_t*>(event.data.ptr), eventType);
    }

    return true;
}

bool SocketPool::active() const
{
    return m_pool != INVALID_EPOLL;
}
