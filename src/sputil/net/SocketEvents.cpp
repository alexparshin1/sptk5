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

#include "sptk5/net/SocketEvents.h"

using namespace std;
using namespace sptk;

#define MAXEVENTS 128

SocketEvents::SocketEvents(SocketEventCallback eventsCallback, chrono::milliseconds timeout)
: Thread("socket events"), m_socketPool(eventsCallback), m_timeout(timeout)
{
}

SocketEvents::~SocketEvents()
{
    try {
        m_socketPool.close();
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
    }
}

void SocketEvents::add(BaseSocket& socket, void* userData)
{
    m_socketPool.watchSocket(socket, userData);
}

void SocketEvents::remove(BaseSocket& socket)
{
    m_socketPool.forgetSocket(socket);
}

void SocketEvents::threadFunction()
{
    m_socketPool.open();
    while (!terminated()) {
        try {
            m_socketPool.waitForEvents(m_timeout);
        }
        catch (const Exception& e) {
            cerr << e.message() << endl;
        }
    }
    m_socketPool.close();
}
