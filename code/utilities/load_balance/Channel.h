/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/cutils>
#include <sptk5/net/SocketEvents.h>
#include <sptk5/net/TCPSocket.h>

namespace sptk {
class Channel : public std::enable_shared_from_this<Channel>
{
    std::mutex                 m_mutex;
    std::shared_ptr<TCPSocket> m_source;
    std::shared_ptr<TCPSocket> m_destination;

    SocketEvents<Channel>& m_sourceEvents;
    SocketEvents<Channel>& m_destinationEvents;

public:
    Channel(SocketEvents<Channel>& sourceEvents, SocketEvents<Channel>& destinationEvents)
        : m_source(std::make_shared<TCPSocket>())
        , m_destination(std::make_shared<TCPSocket>())
        , m_sourceEvents(sourceEvents)
        , m_destinationEvents(destinationEvents)
    {
    }

    ~Channel()
    {
        try
        {
            close();
        }
        catch (const Exception& e)
        {
            CERR(e.what() << std::endl);
        }
    }

    void open(SocketType sourceFD, const String& interfaceAddess, const Host& destination);
    int  copyData(const TCPSocket& source, const TCPSocket& destination);
    void close();

    TCPSocket& source()
    {
        return *m_source;
    }

    TCPSocket& destination()
    {
        return *m_destination;
    }
};

} // namespace sptk
