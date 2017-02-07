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

#ifndef __SPTK_SOCKETPOOL_H__
#define __SPTK_SOCKETPOOL_H__

#include <map>
#include <mutex>
#include <sys/epoll.h>
#include <sptk5/Exception.h>
#include <sptk5/threads/Thread.h>
#include <sptk5/net/BaseSocket.h>

namespace sptk {

class SocketPool : public Synchronized
{
public:

    enum EventType {
        ET_HAS_DATA,
        ET_CONNECTION_CLOSED
    };

    typedef void (*EventCallback)(void *userData, EventType eventType);

private:

    SOCKET                      m_pool;
    EventCallback               m_eventsCallback;
    std::map<BaseSocket*,void*> m_socketData;

public:
    SocketPool(EventCallback eventCallback);
    ~SocketPool();

    void open() throw (Exception);
    void waitForEvents(size_t timeoutMS) throw (Exception);
    void close() throw (Exception);
    void watchSocket(BaseSocket& socket, void *userData) throw (Exception);
    void forgetSocket(BaseSocket& socket) throw (Exception);
};

}

#endif
