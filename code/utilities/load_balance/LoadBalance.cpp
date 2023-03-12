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

#include "LoadBalance.h"
#include "Channel.h"

using namespace std;
using namespace sptk;

void LoadBalance::sourceEventCallback(const uint8_t* userData, SocketEventType eventType)
{
    auto* channel = (Channel*) userData;

    if (eventType == SocketEventType::CONNECTION_CLOSED)
    {
        channel->close();
        delete channel;
        return;
    }

    channel->copyData(channel->source(), channel->destination());
}

void LoadBalance::destinationEventCallback(const uint8_t* userData, SocketEventType eventType)
{
    auto* channel = (Channel*) userData;

    if (eventType == SocketEventType::CONNECTION_CLOSED)
    {
        channel->close();
        delete channel;
        return;
    }

    channel->copyData(channel->destination(), channel->source());
}

LoadBalance::LoadBalance(uint16_t listenerPort, Loop<Host>& destinations, Loop<String>& interfaces)
    : Thread("load balance")
    , m_listenerPort(listenerPort)
    , m_destinations(destinations)
    , m_interfaces(interfaces)
{
}

void LoadBalance::threadFunction()
{
    struct sockaddr_in addr {
    };

    m_sourceEvents.run();
    m_destinationEvents.run();
    m_listener.listen(m_listenerPort);

    constexpr chrono::milliseconds acceptTimeout {500};

    while (!terminated())
    {
        Channel* channel {nullptr};
        try
        {
            SOCKET sourceFD;
            if (m_listener.accept(sourceFD, addr, acceptTimeout))
            {
                channel = new Channel(m_sourceEvents, m_destinationEvents);
                const Host& destination = m_destinations.loop();
                const String& interfaceAddress = m_interfaces.loop();
                channel->open(sourceFD, interfaceAddress, destination);
            }
        }
        catch (const Exception& e)
        {
            delete channel;
            CERR(e.what() << endl);
        }
    }

    m_listener.close();
    m_sourceEvents.terminate();
    m_destinationEvents.terminate();
}
