/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/net/ServerConnection.h>

namespace sptk {
/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief Abstract TCP server connection thread.
 *
 * Application derives concrete TCP server connections based on this class,
 * to use with TCPServer as a connection template.
 */
class SP_EXPORT TCPServerConnection : public ServerConnection
{
public:
    /**
     * @brief Constructor.
     * @param server            TCP server.
     * @param connectionSocket  Already accepted by accept() function incoming connection socket.
     * @param connectionAddress Incoming connection address.
     * @param connectionFunction Connection function executed for each new client connection to the server.
     */
    explicit TCPServerConnection(FastTCPServer& server, SocketType connectionSocket, const sockaddr_in* connectionAddress, const ServerConnection::Function& connectionFunction)
        : ServerConnection(server, Type::TCP, connectionAddress)
    {
        setSocket(std::make_shared<TCPSocket>());
        getSocket()->attach(connectionSocket, false);
    }
};

/**
 * @}
 */
} // namespace sptk
