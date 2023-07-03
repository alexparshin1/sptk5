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

#include <sptk5/SystemException.h>
#include <sptk5/net/Socket.h>

#include <gtest/gtest.h>

using namespace sptk;

TEST(SPTK_Socket, minimal)
{
    constexpr uint16_t sslPort {443};
    const Host yahoo("www.yahoo.com", sslPort);
    sockaddr_in address {};
    yahoo.getAddress(address);

    Socket socket;
    socket.open(address, sptk::Socket::OpenMode::CONNECT);
    socket.close();
}

TEST(SPTK_Socket, option)
{
    constexpr uint16_t sslPort {443};
    const Host yahoo("www.yahoo.com", sslPort);
    sockaddr_in address {};
    yahoo.getAddress(address);

    Socket socket;
    int value = 0;
    try
    {
        socket.getOption(SOL_SOCKET, SO_REUSEADDR, value);
        FAIL() << "Shouldn't get socket option for closed socket";
    }
    catch (const Exception&)
    {
        SUCCEED() << "Can't get socket option for closed socket";
    }

    socket.open(address, sptk::Socket::OpenMode::CONNECT);

    socket.getOption(SOL_SOCKET, SO_REUSEADDR, value);
    EXPECT_EQ(value, 0);

    socket.setOption(SOL_SOCKET, SO_REUSEADDR, 1);
    socket.getOption(SOL_SOCKET, SO_REUSEADDR, value);
    EXPECT_TRUE(value != 0);
}
