/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-03                                             ║
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
#include <sptk5/net/UDPSocket.h>

using namespace std;
using namespace sptk;

UDPSocket::UDPSocket(const SOCKET_ADDRESS_FAMILY _domain)
    : Socket(_domain, SOCK_DGRAM)
{
    auto socketFd = socket(domain(), type(), protocol());
    if (socketFd == INVALID_SOCKET)
    {
        throw Exception("Can't create socket");
    }
    setSocketFdUnlocked(socketFd);
}

size_t UDPSocket::readUnlocked(uint8_t* buffer, const size_t size, sockaddr* from)
{
    socklen_t addressLength = 0;

    sockaddr_in6 addr {};
    if (from == nullptr)
    {
        from = bit_cast<sockaddr*>(&addr);
        addressLength = sizeof(sockaddr_in6);
    }
    else
    {
        addressLength = from->sa_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    }

    const auto bytes = recvfrom(getSocketFdUnlocked(), bit_cast<char*>(buffer), static_cast<int>(size), 0,
                                from, &addressLength);
    if (bytes == -1)
        throwSocketError("Can't read from socket");

    return static_cast<size_t>(bytes);
}
