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

#include "sptk5/net/SocketPool.h"
#include <sys/event.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include "sptk5/SystemException.h"

using namespace std;
using namespace sptk;

SocketPool::SocketPool(SocketPool::EventCallback eventsCallback)
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
    m_pool = kqueue();
    if (m_pool == -1)
        new SystemException("epoll_create1");
}

void SocketPool::close() throw (Exception)
{
    if (m_pool == INVALID_SOCKET)
        return;

    ::close(m_pool);

    SYNCHRONIZED_CODE;
    for (auto itor: m_socketData)
        free(itor.second);

    m_socketData.clear();
    m_pool = INVALID_SOCKET;
}

void SocketPool::watchSocket(BaseSocket& socket, void* userData) throw (Exception)
{
    if (!socket.active())
        throw Exception("Socket is closed");

    SYNCHRONIZED_CODE;

    int socketFD = socket.handle();
    struct kevent* event = (struct kevent*) malloc(sizeof(struct kevent));
    EV_SET(event, socketFD, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, userData);

    int rc = kevent(m_pool, event, 1, NULL, 0, NULL);
    if (rc == -1)
        throw SystemException("Can't add socket to kqueue");

    m_socketData[&socket] = event;
}

void SocketPool::forgetSocket(BaseSocket& socket) throw (Exception)
{
    if (!socket.active())
        throw Exception("Socket is closed");

    struct kevent* event;

    {
        SYNCHRONIZED_CODE;

        map<BaseSocket*,void*>::iterator itor = m_socketData.find(&socket);
        if (itor == m_socketData.end())
            return;

        event = (struct kevent*) itor->second;
        m_socketData.erase(itor);
    }

    int socketFD = socket.handle();
    EV_SET(event, socketFD, 0, EV_DELETE, 0, 0, 0);
    int rc = kevent(m_pool, event, 1, NULL, 0, NULL);
    if (rc == -1)
        throw SystemException("Can't remove socket from kqueue");

    free(event);
}

#define MAXEVENTS 16

void SocketPool::waitForEvents(size_t timeoutMS) throw (Exception)
{
    static const struct timespec timeout = { 0, 10000000 };
    struct kevent events[MAXEVENTS];

    int eventCount = kevent(m_pool, NULL, 0, events, MAXEVENTS, &timeout);
    if (eventCount < 0)
        throw SystemException("Error waiting for socket activity");

    for (int i = 0; i < eventCount; i++) {
        struct kevent& event = events[i];
        if (event.flags & EV_EOF)
            m_eventsCallback(event.udata, ET_CONNECTION_CLOSED);
        else
            m_eventsCallback(event.udata, ET_HAS_DATA);
    }
}
