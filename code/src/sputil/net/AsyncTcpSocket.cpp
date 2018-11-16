/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       AsyncTcpSocket.h - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday Nove 15 2017                                 ║
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

#include <sptk5/net/AsyncTcpSocket.h>

using namespace std;
using namespace sptk;

AsyncTcpSocket::AsyncTcpSocket(SOCKET_ADDRESS_FAMILY domain, int32_t type, int32_t protocol) noexcept
: TCPSocket(domain, type, protocol)
{
}

void AsyncTcpSocket::_open(const Host& host, BaseSocket::CSocketOpenMode openMode, bool blockingMode,
                           chrono::milliseconds timeout)
{
    TCPSocket::_open(host, openMode, blockingMode, timeout);
}

void AsyncTcpSocket::_open(const struct sockaddr_in& address, BaseSocket::CSocketOpenMode openMode, bool blockingMode,
                           chrono::milliseconds timeout)
{
    TCPSocket::open(address, openMode, blockingMode, timeout);
}

void AsyncTcpSocket::accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo)
{
    TCPSocket::accept(clientSocketFD, clientInfo);
}

size_t AsyncTcpSocket::socketBytes()
{
    return TCPSocket::socketBytes();
}

bool AsyncTcpSocket::readyToRead(std::chrono::milliseconds timeout)
{
    return TCPSocket::readyToRead(timeout);
}
