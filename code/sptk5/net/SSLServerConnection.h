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

#include "SSLSocket.h"
#include <sptk5/net/ServerConnection.h>
#include <sptk5/net/TCPServer.h>

namespace sptk {
/**
 * @addtogroup net Networking Classes
 * @{
 */

/**
 * @brief Abstract TCP server connection thread
 *
 * Application derives concrete TCP server connections based on this class,
 * to use with CTCPServer as connection template
 */
class SSLServerConnection : public ServerConnection
{
public:
    /**
     * @brief Constructor
     * @param server             TCP server
     * @param connectionSocket   SOCKET, Already accepted by accept() function incoming connection socket
     */
    SSLServerConnection(TCPServer& server, SocketType connectionSocket, const sockaddr_in* connectionAddress, const ServerConnection::Function& connectionFunction)
        : ServerConnection(server, ServerConnection::Type::SSL, connectionAddress, "SSLServerConnection", connectionFunction)
    {
        auto sslSocket = std::make_shared<SSLSocket>();
        setSocket(sslSocket);
        sslSocket->loadKeys(server.getSSLKeys());
        sslSocket->attach(connectionSocket, true);
    }

    /**
     * @brief Destructor
     */
    ~SSLServerConnection() override = default;
};

/**
 * @}
 */
} // namespace sptk
