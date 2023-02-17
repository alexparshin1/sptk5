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

#pragma once

#include <sptk5/Loop.h>
#include <sptk5/net/SocketEvents.h>
#include <sptk5/net/TCPSocket.h>
#include <vector>

namespace sptk {

class LoadBalance
    : public Thread
{
    uint16_t m_listenerPort;
    Loop<Host>& m_destinations;
    Loop<String>& m_interfaces;
    SocketEvents m_sourceEvents {"Source Events", sourceEventCallback};
    SocketEvents m_destinationEvents {"Destination Events", destinationEventCallback};

    TCPSocket m_listener;

    void threadFunction() override;

    static void sourceEventCallback(const uint8_t* userData, SocketEventType eventType);

    static void destinationEventCallback(const uint8_t* userData, SocketEventType eventType);

public:
    LoadBalance(uint16_t listenerPort, Loop<Host>& destinations, Loop<String>& interfaces);

    ~LoadBalance() override = default;
};

} // namespace sptk
