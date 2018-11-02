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

#ifndef __ASYNC_TCP_SOCKET_H__
#define __ASYNC_TCP_SOCKET_H__

#include <sptk5/net/TCPSocket.h>
#include <sptk5/net/SocketEvents.h>

namespace sptk
{

class AsyncTcpSocket : public TCPSocket
{
    static SocketEvents socketEvents;
public:
    /**
     * Constructor
     * @param domain            Socket domain type
     * @param type              Socket type
     * @param protocol          Protocol type
     */
    AsyncTcpSocket(SOCKET_ADDRESS_FAMILY domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0) noexcept;

    /**
     * Destructor
     */
    virtual ~AsyncTcpSocket() = default;

protected:

    void _open(const Host& host, CSocketOpenMode openMode, bool blockingMode, std::chrono::milliseconds timeout) override;

    void _open(const struct sockaddr_in& address, CSocketOpenMode openMode, bool blockingMode,
               std::chrono::milliseconds timeout) override;

public:

    void accept(SOCKET& clientSocketFD, struct sockaddr_in& clientInfo) override;

    size_t socketBytes() override;

    bool readyToRead(std::chrono::milliseconds timeout) override;
};

} // namespace sptk

#endif
