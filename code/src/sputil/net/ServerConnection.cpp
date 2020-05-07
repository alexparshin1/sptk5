/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ServerConnection.cpp - description                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
#include <sptk5/net/TCPServer.h>
#include <sptk5/net/ServerConnection.h>

#ifndef _WIN32
#include <arpa/inet.h>
#else
#include <ws2tcpip.h>
#endif

using namespace std;
using namespace sptk;

TCPSocket& ServerConnection::socket() const
{
    lock_guard<mutex>   lock(m_mutex);
    return *m_socket;
}

void ServerConnection::setSocket(TCPSocket* socket)
{
    lock_guard<mutex>   lock(m_mutex);
    m_socket = socket;
}

TCPServer& ServerConnection::server() const
{
    lock_guard<mutex>   lock(m_mutex);
    return m_server;
}

ServerConnection::ServerConnection(TCPServer& server, SOCKET, const sockaddr_in* connectionAddress, const String& taskName)
: Runable(taskName), m_server(server), m_socket(nullptr)
{
    parseAddress(connectionAddress);
}

ServerConnection::~ServerConnection()
{
    lock_guard<mutex>   lock(m_mutex);
    delete m_socket;
}

void ServerConnection::parseAddress(const sockaddr_in* connectionAddress)
{
    char address[128] { "127.0.0.1" };
    if (connectionAddress) {
        if (connectionAddress->sin_family == AF_INET) {
            inet_ntop(AF_INET, &connectionAddress->sin_addr, address, sizeof(address));
        } else if (connectionAddress->sin_family == AF_INET6) {
            auto* connectionAddress6 = (sockaddr_in6*) connectionAddress;
            inet_ntop(AF_INET6, &connectionAddress6->sin6_addr, address, sizeof(address));
        }
    }
    m_address = address;
}
