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

#include "LoadBalance.h"
#include "Channel.h"

using namespace std;
using namespace sptk;

void LoadBalance::sourceEventCallback(void *userData, SocketEventType eventType)
{
    Channel* channel = (Channel*) userData;

    if (eventType == ET_CONNECTION_CLOSED) {
        channel->close();
        delete channel;
        return;
    }

    channel->copyData(channel->source(), channel->destination());
}

void LoadBalance::destinationEventCallback(void *userData, SocketEventType eventType)
{
    Channel* channel = (Channel*) userData;

    if (eventType == ET_CONNECTION_CLOSED) {
        //cout << "Destination socket closed" << endl;
        //cout.flush();
        channel->close();
        delete channel;
        return;
    }

    channel->copyData(channel->destination(), channel->source());
}

LoadBalance::LoadBalance(int listenerPort, Loop<Host>& destinations, Loop<String>& interfaces)
: Thread("load balance"), m_listenerPort(listenerPort), m_destinations(destinations), m_interfaces(interfaces),
  m_sourceEvents(sourceEventCallback), m_destinationEvents(destinationEventCallback)
{
}

LoadBalance::~LoadBalance()
{
}

void LoadBalance::threadFunction()
{
    struct sockaddr_in addr;

    m_sourceEvents.run();
    m_destinationEvents.run();
    m_listener.listen(m_listenerPort);

    while (!terminated()) {
        SOCKET sourceFD;
        m_listener.accept(sourceFD, addr);
        Channel* channel = new Channel(m_sourceEvents, m_destinationEvents);
        const Host& destination = m_destinations.loop();
        const String& interfaceAddress = m_interfaces.loop();
        try {
            channel->open(sourceFD, interfaceAddress, destination);
        }
        catch (const exception& e) {
            delete channel;
            cerr << e.what() << endl;
        }
    }

    m_listener.close();
    m_sourceEvents.terminate();
    m_destinationEvents.terminate();
}
