/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cutils>
#include <sptk5/net/ServerConnection.h>
#include <sptk5/net/TCPServer.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

using namespace std;
using namespace sptk;

size_t ServerConnection::nextSerial()
{
    static mutex  aMutex;
    static size_t serial = 0;

    const scoped_lock lock(aMutex);
    return ++serial;
}

TCPSocket& ServerConnection::socket() const
{
    const scoped_lock lock(m_mutex);
    return *m_socket;
}

STCPSocket ServerConnection::getSocket() const
{
    const scoped_lock lock(m_mutex);
    return m_socket;
}

STCPSocket ServerConnection::setSocket(const STCPSocket& socket)
{
    const scoped_lock lock(m_mutex);
    auto              priorSocket = m_socket;
    m_socket = socket;
    return priorSocket;
}

TCPServer& ServerConnection::server() const
{
    const scoped_lock lock(m_mutex);
    return m_server;
}

ServerConnection::ServerConnection(TCPServer& server, Type type, const sockaddr_in* connectionAddress,
                                   const String& taskName, ServerConnection::Function connectionFunction)
    : Runable(taskName)
    , m_server(server)
    , m_serial(nextSerial())
    , m_type(type)
    , m_connectionFunction(std::move(connectionFunction))
{
    parseAddress(connectionAddress);
}

void ServerConnection::parseAddress(const sockaddr_in* connectionAddress)
{
    constexpr int               maxAddressSize {128};
    array<char, maxAddressSize> address {"127.0.0.1"};
    if (connectionAddress)
    {
        if (connectionAddress->sin_family == AF_INET)
        {
            inet_ntop(AF_INET, &connectionAddress->sin_addr, address.data(), sizeof(address));
            m_port = ntohs(connectionAddress->sin_port);
        }
        else if (connectionAddress->sin_family == AF_INET6)
        {
            const auto* connectionAddress6 = bit_cast<const sockaddr_in6*>(connectionAddress);
            inet_ntop(AF_INET6, &connectionAddress6->sin6_addr, address.data(), sizeof(address));
            m_port = ntohs(connectionAddress6->sin6_port);
        }
    }
    m_address = String(address.data());
}

uint16_t ServerConnection::port() const
{
    return m_port;
}

void ServerConnection::close()
{
    const scoped_lock lock(m_mutex);
    if (m_socket->active())
    {
        m_socket->close();
    }
}
