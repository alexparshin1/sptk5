/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include "TestEchoServer.h"

#include <gtest/gtest.h>
#include <sptk5/net/SocketPool.h>
#include <sptk5/net/TCPSocket.h>

using namespace std;
using namespace sptk;

namespace {

constexpr uint16_t testEchoServerPort = 5002;

class ProbeSocketObjectPool final : public SocketObjectPool<Socket>
{
public:
    explicit ProbeSocketObjectPool(const SocketEventCallback<Socket>& callback)
        : SocketObjectPool(callback, SocketPoolTriggerMode::LevelTriggered, 8)
    {
    }

    void dispatch(Socket* socket, const SocketEventType eventType)
    {
        onEvent(socket, eventType);
    }
};

SocketEventType dataEvent()
{
    return {.m_data = true, .m_hangup = false, .m_error = false};
}

} // namespace
namespace sptk {

TEST(SocketObjectPoolTests,ignoresUnknownSocketEvent)
{
    atomic_int callbackCount = 0;

    ProbeSocketObjectPool pool(
        [&callbackCount](const weak_ptr<Socket>&, SocketEventType)
        {
            ++callbackCount;
        });

    const auto socket = make_shared<TCPSocket>();
    pool.dispatch(socket.get(), dataEvent());

    EXPECT_EQ(callbackCount.load(), 0);
}

TEST(SocketObjectPoolTests,addFailureDoesNotLeaveStaleUserData)
{
    atomic_int callbackCount = 0;

    ProbeSocketObjectPool pool(
        [&callbackCount](const weak_ptr<Socket>&, SocketEventType)
        {
            ++callbackCount;
        });

    TestEchoServer testEchoServer(testEchoServerPort);
    const auto     socket = make_shared<TCPSocket>();
    socket->open(Host("localhost", testEchoServerPort));

    pool.close();

    EXPECT_THROW(pool.add(socket, socket), Exception);

    pool.dispatch(socket.get(), dataEvent());
    EXPECT_EQ(callbackCount.load(), 0);

    socket->close();
    testEchoServer.stop();
}

TEST(SocketObjectPoolTests,removeAfterCloseClearsUserData)
{
    atomic_int callbackCount = 0;

    ProbeSocketObjectPool pool(
        [&callbackCount](const weak_ptr<Socket>&, SocketEventType)
        {
            ++callbackCount;
        });

    TestEchoServer testEchoServer(testEchoServerPort);
    const auto     socket = make_shared<TCPSocket>();
    socket->open(Host("localhost", testEchoServerPort));

    pool.add(socket, socket);
    socket->close();
    pool.remove(socket);

    pool.dispatch(socket.get(), dataEvent());
    EXPECT_EQ(callbackCount.load(), 0);

    testEchoServer.stop();
}

} // namespace sptk_test
