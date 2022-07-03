/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerConnection.h>

namespace sptk {

/**
 * Not encrypted connection to echo server
 */
class EchoConnection
    : public TCPServerConnection
{
public:
    EchoConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* connectionAddress);

    ~EchoConnection() override = default;

    /**
     * Terminate connection thread
     */
    void terminate() override;

    /**
     * Connection thread function
     */
    void run() override;
};

/**
 * @brief Echo server used in unit tests
 */
class EchoServer
    : public TCPServer
{
public:
    EchoServer();

protected:
    SServerConnection createConnection(SOCKET connectionSocket, sockaddr_in* peer) override;
};

} // namespace sptk
