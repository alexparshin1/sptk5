/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DateTime.h - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday Sep 17 2015                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#include <sys/epoll.h>
#include "sptk5/net/SocketPool.h"
#include "sptk5/SystemException.h"

using namespace std;
using namespace sptk;

SocketPool::SocketPool(SocketEventCallback eventsCallback)
: m_pool(INVALID_SOCKET), m_eventsCallback(eventsCallback)
{
    open();
}

SocketPool::~SocketPool()
{
    close();
}

void SocketPool::open() throw (Exception)
{
    if (m_pool != INVALID_SOCKET)
        return;
    m_pool = epoll_create1(0);
    if (m_pool == -1)
        new SystemException("epoll_create1");
}

void SocketPool::close() throw (Exception)
{
    if (m_pool != INVALID_SOCKET) {
        ::close(m_pool);
        m_pool = INVALID_SOCKET;
    }

    SYNCHRONIZED_CODE;
    for (auto itor: m_socketData)
        free(itor.second);
    m_socketData.clear();
}

void SocketPool::watchSocket(BaseSocket& socket, void* userData) throw (Exception)
{
    if (!socket.active())
        throw Exception("Socket is closed");

    SYNCHRONIZED_CODE;

    int socketFD = socket.handle();

    auto event = (epoll_event*) malloc(sizeof(epoll_event));
    event->data.ptr = userData;
    event->events = EPOLLIN | EPOLLHUP | EPOLLRDHUP;

    int rc = epoll_ctl(m_pool, EPOLL_CTL_ADD, socketFD, event);
    if (rc == -1)
      throw SystemException("Can't add socket to epoll");

    m_socketData[&socket] = event;
}

void SocketPool::forgetSocket(BaseSocket& socket) throw (Exception)
{
    if (!socket.active())
        throw Exception("Socket is closed");

    epoll_event* event;

    {
        SYNCHRONIZED_CODE;

        auto itor = m_socketData.find(&socket);
        if (itor == m_socketData.end())
            return;

        event = (epoll_event*) itor->second;
        m_socketData.erase(itor);
    }

    int rc = epoll_ctl(m_pool, EPOLL_CTL_DEL, socket.handle(), event);
    if (rc == -1)
        throw SystemException("Can't remove socket from epoll");

    free(event);
}

#define MAXEVENTS 16

void SocketPool::waitForEvents(size_t timeoutMS) throw (Exception)
{
    epoll_event events[MAXEVENTS];

    int eventCount = epoll_wait(m_pool, events, MAXEVENTS, (int) timeoutMS);
    if (eventCount < 0)
        throw SystemException("Error waiting for socket activity");

    //cout << "Got " << eventCount << " events" << endl;

    for (int i = 0; i < eventCount; i++) {
        epoll_event& event = events[i];
        if ((event.events & (EPOLLHUP | EPOLLRDHUP)) != 0)
            m_eventsCallback(event.data.ptr, ET_CONNECTION_CLOSED);
        else
            m_eventsCallback(event.data.ptr, ET_HAS_DATA);
    }
}
