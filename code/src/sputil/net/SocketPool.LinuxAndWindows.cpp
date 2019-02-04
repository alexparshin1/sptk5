/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DateTime.h - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday Sep 17 2015                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include "sptk5/SystemException.h"
#include "sptk5/net/SocketPool.h"
#ifdef _WIN32
#include <wepoll.h>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#else
#include <sys/epoll.h>
#endif
#include <sptk5/net/SocketPool.h>


using namespace std;
using namespace sptk;

SocketPool::SocketPool(SocketEventCallback eventsCallback)
: m_pool(INVALID_EPOLL), m_eventsCallback(eventsCallback)
{
}

SocketPool::~SocketPool()
{
    close();
}

void SocketPool::open()
{
    lock_guard<mutex> lock(*this);

    if (m_pool != INVALID_EPOLL)
        return;

    m_pool = epoll_create1(0);

    if (m_pool == INVALID_EPOLL)
        throw SystemException("epoll_create1");
}

void SocketPool::close()
{
    lock_guard<mutex> lock(*this);

    if (m_pool != INVALID_EPOLL) {
#ifdef _WIN32
        epoll_close(m_pool);
#else
        ::close(m_pool);
#endif
        m_pool = INVALID_EPOLL;
    }

    for (auto itor: m_socketData)
        free(itor.second);

    m_socketData.clear();
}

void SocketPool::watchSocket(BaseSocket& socket, void* userData)
{
    if (!socket.active())
        throw Exception("Socket is closed");

    lock_guard<mutex> lock(*this);

    int socketFD = socket.handle();

    auto* event = (epoll_event*) malloc(sizeof(epoll_event));
    event->data.ptr = userData;
    event->events = EPOLLIN | EPOLLHUP | EPOLLRDHUP;

    int rc = epoll_ctl(m_pool, EPOLL_CTL_ADD, socketFD, event);
    if (rc == -1)
      throw SystemException("Can't add socket to epoll");

    m_socketData[&socket] = event;
}

void SocketPool::forgetSocket(BaseSocket& socket)
{
    epoll_event* event;

    if (socket.active()) {
        lock_guard<mutex> lock(*this);

        auto itor = m_socketData.find(&socket);
        if (itor == m_socketData.end())
            return;

        event = (epoll_event*) itor->second;
        m_socketData.erase(itor);
    } else
        throw Exception("Socket is closed");

    int rc = epoll_ctl(m_pool, EPOLL_CTL_DEL, socket.handle(), event);
    if (rc == -1)
        throw SystemException("Can't remove socket from epoll");

    free(event);
}

#define MAXEVENTS 16

void SocketPool::waitForEvents(chrono::milliseconds timeout)
{
    epoll_event events[MAXEVENTS];

    int eventCount = epoll_wait(m_pool, events, MAXEVENTS, (int) timeout.count());
    if (eventCount < 0)
        throw SystemException("Error waiting for socket activity");

    for (int i = 0; i < eventCount; i++) {
        epoll_event& event = events[i];
        if ((event.events & (EPOLLHUP | EPOLLRDHUP)) != 0)
            m_eventsCallback(event.data.ptr, ET_CONNECTION_CLOSED);
        else
            m_eventsCallback(event.data.ptr, ET_HAS_DATA);
    }
}

bool SocketPool::active()
{
    return m_pool != INVALID_EPOLL;
}
