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

#ifndef __SPTK_SOCKETEVENTS_H__
#define __SPTK_SOCKETEVENTS_H__

#include <map>
#include <mutex>
#include <sptk5/Exception.h>
#include <sptk5/threads/Thread.h>
#include <sptk5/net/BaseSocket.h>
#include <sptk5/net/SocketPool.h>
#include "sptk5/SystemException.h"

namespace sptk {

class SocketEvents : public Thread
{
    SocketPool              m_socketPool;

    std::mutex              m_mutex;
    std::map<int, void*>	m_watchList;

    size_t                  m_timeoutMS;

protected:

    void threadFunction() override;

public:
    SocketEvents(SocketEventCallback eventsCallback, size_t timeoutMS=1000);
    ~SocketEvents();

    void watch(BaseSocket& socket, void *userData) throw (Exception);
    void forget(BaseSocket& socket) throw (Exception);
};

}

#endif
