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

#include "sptk5/net/SocketPool.h"
#include "sptk5/SystemException.h"
#include <errno.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/event.h>
#include <unistd.h>

using namespace std;
using namespace sptk;

void SocketPool::open()
{
    const scoped_lock lock(*this);

    if (m_pool != INVALID_SOCKET)
    {
        return;
    }

    m_pool = kqueue();

    if (m_pool == INVALID_SOCKET)
    {
        new SystemException("Can't create kqueue");
    }
}

void SocketPool::close()
{
    scoped_lock lock(*this);

    if (m_pool == INVALID_SOCKET)
    {
        return;
    }

    ::shutdown(m_pool, SHUT_RDWR);

    m_pool = INVALID_SOCKET;
}

void SocketPool::watchSocket(Socket& socket, const uint8_t* userData)
{
    const scoped_lock lock(*this);

    struct kevent event {
    };
    auto eventFlags = EV_ADD | EV_ENABLE;
    switch (m_triggerMode)
    {
        case TriggerMode::EdgeTriggered:
            eventFlags |= EV_CLEAR;
            break;
        case TriggerMode::OneShot:
            eventFlags |= EV_ONESHOT;
            break;
        case TriggerMode::LevelTriggered:
            break;
    }
    EV_SET(&event, socket.fd(), EVFILT_READ, eventFlags, 0, 0, bit_cast<void*>(userData));

    int rc = kevent(m_pool, &event, 1, NULL, 0, NULL);
    if (rc == -1)
    {
        if (m_pool == INVALID_SOCKET)
        {
            throw SystemException("SocketPool is not open");
        }

        throw SystemException("Can't add socket to kqueue");
    }
}

void SocketPool::forgetSocket(const Socket& socket)
{
    const scoped_lock lock(*this);

    struct kevent event {
    };
    EV_SET(&event, socket.fd(), 0, EV_DELETE, 0, 0, 0);

    if (int rc = kevent(m_pool, &event, 1, NULL, 0, NULL);
        rc == -1)
    {
        throw SystemException("Can't remove socket from kqueue");
    }
}

bool SocketPool::waitForEvents(std::chrono::milliseconds timeoutMS)
{
    const scoped_lock lock(*this);

    static const struct timespec timeout = {time_t(timeoutMS.count() / 1000),
                                            long((timeoutMS.count() % 1000) * 1000000)};

    std::array<struct kevent, maxEvents> events {};
    int eventCount = kevent(m_pool, NULL, 0, events.data(), maxEvents, &timeout);
    if (eventCount < 0)
    {
        if (m_pool == INVALID_SOCKET)
        {
            return false;
        }
        return true;
    }

    for (int i = 0; i < eventCount; i++)
    {
        auto& event = events[i];

        SocketEventType eventType {};
        eventType.m_data = event.data > 0;
        eventType.m_hangup = event.flags & EV_EOF;
        eventType.m_error = event.flags & EV_ERROR;

        m_eventsCallback(bit_cast<const uint8_t*>(event.udata), eventType);
    }

    return true;
}
