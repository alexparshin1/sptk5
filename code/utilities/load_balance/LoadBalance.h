/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DateTime.h - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday Sep 17 2015                                   ║
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_LOADBALANCE_H__
#define __SPTK_LOADBALANCE_H__

#include <vector>
#include <sptk5/Loop.h>
#include <sptk5/net/SocketEvents.h>
#include <sptk5/net/TCPSocket.h>

namespace sptk {

class LoadBalance : public Thread
{
    uint16_t              m_listenerPort;
    Loop<Host>&           m_destinations;
    Loop<String>&         m_interfaces;
    SocketEvents          m_sourceEvents;
    SocketEvents          m_destinationEvents;

    TCPSocket             m_listener;

    void threadFunction() override;

    static void sourceEventCallback(void *userData, SocketEventType eventType);
    static void destinationEventCallback(void *userData, SocketEventType eventType);
public:
    LoadBalance(uint16_t listenerPort, Loop<Host>& destinations, Loop<String>& interfaces);
    ~LoadBalance() override = default;
};

}

#endif
