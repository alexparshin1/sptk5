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

#include "SSLSocket.h"
#include <sptk5/net/FastTCPServer.h>
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
 * to use with TCPServer as the connection template.
 */
class SP_EXPORT SSLServerConnection : public ServerConnection
{
public:
    /**
     * @brief Constructor.
     * @param server             TCP server.
     * @param connectionSocket   SOCKET, Already accepted by accept() function incoming connection socket.
     * @param connectionAddress  Address of the connected client.
     */
    SSLServerConnection(FastTCPServer& server, SocketType connectionSocket, const sockaddr_in* connectionAddress)
        : ServerConnection(server, Type::SSL, connectionAddress)
    {
        auto sslSocket = std::make_shared<SSLSocket>();
        setSocket(sslSocket);
        const auto keys = server.getSSLKeys();
        if (!keys)
        {
            throw Exception("SSL connection created but server has no SSL keys configured");
        }
        sslSocket->loadKeys(*keys);
        sslSocket->attach(connectionSocket, true);
    }

    /**
     * @brief Destructor.
     */
    ~SSLServerConnection() override = default;
};

/**
 * @}
 */
} // namespace sptk
