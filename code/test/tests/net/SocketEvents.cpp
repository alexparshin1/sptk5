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

#include "TestEchoServer.h"
#include "sptk5/StopWatch.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sptk5/net/SocketEvents.h>

using namespace std;
using namespace sptk;
using namespace chrono;

namespace {

constexpr uint16_t testEchoServerPort = 5001;

/**
 * @brief Test SocketEvents communication with echo server using selected trigger mode
 */
void testSocketEvents(SocketPool::TriggerMode triggerMode)
{
    shared_ptr<SocketEvents> socketEvents;
    Semaphore                dataReceived;
    Semaphore                hangupReceived;

    auto eventsCallback =
        [&dataReceived, &hangupReceived, &socketEvents, triggerMode](const uint8_t* userData, SocketEventType eventType)
    {
        auto* socket = bit_cast<Socket*>(userData);

        // In a real server or client, we want to prevent async calls of socket events while
        // the current event is processed in another thread.
        // In this test, the event is processed in the same thread, but we still want to test
        // that removing and re-adding the socket works properly.
        // Note that removing socket is not required for OneShot trigger mode.
        if (triggerMode != SocketPool::TriggerMode::OneShot)
        {
            socketEvents->remove(*socket);
        }

        if (eventType.m_data)
        {
            auto bytes = socket->socketBytes();
            if (bytes > 0)
            {
                String data;
                socket->read(data, bytes);
                COUT("Client received " << bytes << " bytes: [" << data << "]");
                dataReceived.post();
            }
        }

        if (eventType.m_hangup)
        {
            COUT("Server hangup");
            socket->close();
            hangupReceived.post();
            // Don't add a socket to SocketEvents after the hangup.
            return;
        }

        socketEvents->add(*socket, bit_cast<uint8_t*>(socket), true);
    };

    socketEvents = make_shared<SocketEvents>("Test Pool", eventsCallback, 1s, triggerMode);

    Buffer buffer;

    try
    {
        TestEchoServer testEchoServer(testEchoServerPort);

        TCPSocket socket;
        socket.open(Host("localhost", testEchoServerPort));
        socketEvents->add(socket, bit_cast<uint8_t*>(&socket));

        try
        {
            const String testData1("Hello, World!");
            socket.write(testData1);
            EXPECT_TRUE(dataReceived.wait_for(100ms));
            const String testData2("This is a test of SocketEvents class.<EOF>");
            socket.write(testData2);
            EXPECT_TRUE(dataReceived.wait_for(100ms));
        }
        catch (const Exception& e)
        {
            CERR(e.what());
        }

        socketEvents->remove(socket);
        socket.close();

        EXPECT_TRUE(hangupReceived.wait_for(100ms));

        testEchoServer.stop();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}
} // namespace

TEST(SPTK_SocketEvents, minimal_levelTriggered)
{
    testSocketEvents(SocketPool::TriggerMode::LevelTriggered);
}

#ifndef _WIN32
/**
 * @brief Test SocketEvents communication with echo server using EdgeTriggered mode
 * @remarks The event count must show the events coming upon new data arrival to client's socket
 */
TEST(SPTK_SocketEvents, minimal_edgeTriggered)
{
    testSocketEvents(SocketPool::TriggerMode::EdgeTriggered);
}
#endif

/**
 * @brief Test SocketEvents communication with echo server using EdgeTriggered mode
 * @remarks The event count must show the event is triggered once data becomes available in client's socket
 */
TEST(SPTK_SocketEvents, minimal_oneShot)
{
    testSocketEvents(SocketPool::TriggerMode::OneShot);
}

TEST(SPTK_SocketEvents, performance)
{
    TestEchoServer testEchoServer(testEchoServerPort);

    SocketEvents socketEvents(
        "test events",
        [](const uint8_t*, SocketEventType)
        {
            // No need to do anything for this test
            return;
        });

    constexpr size_t  maxSockets = 1000;
    vector<TCPSocket> sockets(maxSockets);
    const Host        testServerHost("localhost", testEchoServerPort);
    for (auto& socket: sockets)
    {
        ASSERT_NO_THROW(socket.open(testServerHost, Socket::OpenMode::CONNECT, true, 100ms));
    }

    StopWatch stopWatch;

    stopWatch.start();
    for (auto& socket: sockets)
    {
        socketEvents.add(socket, nullptr);
    }

    for (auto& socket: sockets)
    {
        socketEvents.remove(socket);
    }

    stopWatch.stop();

    COUT("Executed " << maxSockets << " add/remove socket ops: "
                     << fixed << setprecision(2) << maxSockets / stopWatch.milliseconds() << "K/sec\n"
                     << flush);

    socketEvents.stop();
}
