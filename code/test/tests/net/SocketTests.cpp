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

#include <sptk5/SystemException.h>
#include <sptk5/net/Socket.h>

#include <bit>

#include <gtest/gtest.h>

using namespace sptk;
namespace sptk {

TEST(SocketTests,minimal)
{
    constexpr uint16_t sslPort {443};
    const Host testHost("test_http_host", sslPort);
    sockaddr_in address {};
    testHost.getAddress(address);

    Socket socket;
    socket.open(address, sptk::Socket::OpenMode::CONNECT);
    socket.close();
}

TEST(SocketTests,option)
{
    constexpr uint16_t sslPort {443};
    const Host testHost("test_http_host", sslPort);
    sockaddr_in address {};
    testHost.getAddress(address);

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

/**
 * @brief The address a listening socket actually ended up bound to.
 */
namespace {
sockaddr_in boundAddress(const Socket& socket)
{
    sockaddr_in address {};
    auto        length = static_cast<socklen_t>(sizeof(address));
    if (getsockname(socket.fd(), std::bit_cast<sockaddr*>(&address), &length) != 0)
    {
        throw SystemException("getsockname");
    }
    return address;
}
} // namespace

TEST(SocketTests, listenBindsTheAddressItWasGiven)
{
    // "Listen on this address" is a boundary as often as it is a routing choice, and a listener
    // that quietly binds every address instead cannot be caught by using it: everything the
    // narrower binding was meant to exclude still works. Checked against the kernel rather than
    // against what was asked for.
    constexpr uint16_t testPort {31245};

    Socket socket;
    socket.host(Host("127.0.0.1", testPort));
    socket.listen(0, true);

    const auto address = boundAddress(socket);
    EXPECT_EQ(AF_INET, address.sin_family);
    EXPECT_EQ(htonl(INADDR_LOOPBACK), address.sin_addr.s_addr);
    EXPECT_EQ(htons(testPort), address.sin_port);
    socket.close();
}

TEST(SocketTests, listenBindsEveryAddressWhenNoneIsNamed)
{
    // The other half of the same rule: "0.0.0.0" means every address on the machine, and that is
    // what nearly every server in this library asks for.
    constexpr uint16_t testPort {31246};

    Socket socket;
    socket.host(Host("0.0.0.0", testPort));
    socket.listen(0, true);

    const auto address = boundAddress(socket);
    EXPECT_EQ(htonl(INADDR_ANY), address.sin_addr.s_addr);
    socket.close();
}

} // namespace sptk_test
