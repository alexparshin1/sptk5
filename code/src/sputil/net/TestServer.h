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

#include "sptk5/net/SSLServerConnection.h"
#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerConnection.h>

namespace sptk {

/**
 * Not encrypted connection that echoes anything sent to it
 */
class EchoConnection
    : public TCPServerConnection
{
public:
    EchoConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* connectionAddress)
        : TCPServerConnection(server, connectionSocket, connectionAddress, connectionFunction)
    {
    }

    ~EchoConnection() override = default;

    static void connectionFunction(Runable& task, TCPSocket& socket, const String& address);
};

/**
 * Not encrypted connection that pushes data as fast as possible
 */
class PushConnection
    : public TCPServerConnection
{
public:
    PushConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* connectionAddress)
        : TCPServerConnection(server, connectionSocket, connectionAddress, connectionFunction)
    {
        socket().blockingMode(false);
    }

    ~PushConnection() override = default;

    static void connectionFunction(Runable& task, TCPSocket& socket, const String& address);
};

/**
 * Encrypted connection that echoes anything sent to it
 */
class EchoSslConnection
    : public SSLServerConnection
{
public:
    EchoSslConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* connectionAddress)
        : SSLServerConnection(server, connectionSocket, connectionAddress, connectionFunction)
    {
    }

    ~EchoSslConnection() override = default;

    static void connectionFunction(Runable& task, TCPSocket& socket, const String& address);
};

/**
 * Encrypted connection that pushes data as fast as possible
 */
class PushSslConnection
    : public SSLServerConnection
{
public:
    PushSslConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* connectionAddress)
        : SSLServerConnection(server, connectionSocket, connectionAddress, connectionFunction)
    {
    }

    ~PushSslConnection() override = default;

    static void connectionFunction(Runable& task, TCPSocket& socket, const String& address);
};

/**
 * @brief Test server used in unit tests
 */
template<typename ConnectionThreadType>
class TestServer
    : public TCPServer
{
public:
    TestServer()
        : TCPServer("TestServer", ServerConnection::Type::TCP)
    {
    }

protected:
    SServerConnection createConnection(SOCKET connectionSocket, const sockaddr_in* peer) override
    {
        return make_shared<ConnectionThreadType>(*this, connectionSocket, peer);
    }
};

} // namespace sptk
