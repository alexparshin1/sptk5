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

#ifndef __SPTK_CHANNEL_H__
#define __SPTK_CHANNEL_H__

#include <sptk5/net/TCPSocket.h>
#include <sptk5/net/SocketEvents.h>

namespace sptk {

class Channel
{
protected:

    TCPSocket       m_source;
    TCPSocket       m_destination;

    SocketEvents&   m_sourceEvents;
    SocketEvents&   m_destinationEvents;

    std::mutex      m_mutex;

public:

    Channel(SocketEvents& sourceEvents, SocketEvents& destinationEvents)
    : m_sourceEvents(sourceEvents), m_destinationEvents(destinationEvents)
    {}

    ~Channel()
    {
        try {
            close();
        }
        catch (...) {}
    }

    void open(SOCKET sourceFD, const String& interfaceAddess, const Host& destination);
    int  copyData(TCPSocket& source, TCPSocket& destination);
    void close();

    TCPSocket& source()      { return m_source; }
    TCPSocket& destination() { return m_destination; }
};

}

#endif
