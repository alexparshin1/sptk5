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

#include "sptk5/StopWatch.h"
#include "sptk5/net/TCPServer.h"
#include <gtest/gtest.h>
#include <sptk5/net/SocketEvents.h>
#include <sptk5/net/SocketReader.h>

using namespace std;
using namespace sptk;
using namespace chrono;

static constexpr uint16_t testEchoServerPort = 5001;

/**
 * @brief Test TCP echo server function
 */
static void echoTestFunction(const Runable& task, TCPSocket& socket, const String& /*address*/)
{
    SocketReader socketReader(socket);
    Buffer data;
    while (!task.terminated())
    {
        try
        {
            if (socketReader.readyToRead(chrono::seconds(1)))
            {
                if (socketReader.readLine(data) == 0)
                {
                    continue;
                }
                string str(data.c_str());
                str += "\n";
                socket.write(str);
            }
            else
            {
                break;
            }
        }
        catch (const Exception& e)
        {
            break;
        }
    }
    socket.close();
}

/**
 * @brief Test SocketEvents communication with echo server using LevelTriggered mode
 */
TEST(SPTK_SocketEvents, minimal_levelTriggered)
{
    Semaphore eventReceived;
    shared_ptr<SocketReader> socketReader;

    auto eventsCallback =
        [&eventReceived, &socketReader](const uint8_t* /*userData*/, SocketEventType eventType) {
            Buffer line;

            if (eventType.m_data)
            {
                while (socketReader->readLine(line, '\n') != 0)
                {
                    eventReceived.post();
                    COUT("Client received: " << line.c_str() << endl);
                }
            }

            if (eventType.m_hangup)
            {
                COUT("Socket closed" << endl);
            }

            return SocketEventAction::Continue;
        };

    SocketEvents socketEvents("Test Pool", eventsCallback, chrono::milliseconds(100),
                              SocketPool::TriggerMode::LevelTriggered);

    Buffer buffer;

    try
    {
        TCPServer echoServer("TestServer", ServerConnection::Type::TCP);
        echoServer.onConnection(echoTestFunction);
        echoServer.listen(testEchoServerPort);

        Strings testRows({"Hello, World!",
                          "This is a test of SocketEvents class.",
                          "Using simple echo server to support data flow.",
                          "The session is terminated when this row is received"});

        TCPSocket socket;
        socket.open(Host("localhost", testEchoServerPort));

        socketEvents.add(socket, (uint8_t*) &socket);

        socketReader = make_shared<SocketReader>(socket);

        for (const auto& row: testRows)
        {
            auto bytes = socket.write((const uint8_t*) row.c_str(), row.length());
            auto bytes2 = socket.write((const uint8_t*) "\n", 1);
            if (bytes != row.length() || bytes2 != 1)
            {
                FAIL() << "Client can't send data";
            }
        }

        size_t receivedEventCount {0};
        while (eventReceived.wait_for(chrono::milliseconds(100)))
        {
            receivedEventCount++;
        }

        socketEvents.remove(socket);
        socket.close();

        EXPECT_EQ(4u, receivedEventCount);
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

/**
 * @brief Test SocketEvents communication with echo server using EdgeTriggered mode
 * @remarks The event count must show the events coming upon new data arrival to client's socket
 */
TEST(SPTK_SocketEvents, minimal_edgeTriggered)
{
    atomic_size_t eventCount;
    Semaphore receivedEvent;

    auto eventsCallback =
        [&eventCount, &receivedEvent](const uint8_t* /*userData*/, SocketEventType eventType) {
            if (eventType.m_hangup)
            {
                return SocketEventAction::Forget;
            }
            else
            {
                receivedEvent.post();
                eventCount++;
            }

            return SocketEventAction::Continue;
        };

    SocketEvents socketEvents("Test Pool", eventsCallback, chrono::milliseconds(100),
                              SocketPool::TriggerMode::EdgeTriggered);

    Buffer buffer;

    try
    {
        TCPServer echoServer("TestServer", ServerConnection::Type::TCP);
        echoServer.onConnection(echoTestFunction);
        echoServer.listen(testEchoServerPort);

        Strings testRows({"Hello, World!",
                          "This is a test of SocketEvents class.",
                          "Using simple echo server to support data flow."});

        TCPSocket socket;
        socket.open(Host("localhost", testEchoServerPort));

        socketEvents.add(socket, (uint8_t*) &socket);

        for (const auto& row: testRows)
        {
            auto bytes = socket.write((const uint8_t*) row.c_str(), row.length());
            auto bytes2 = socket.write((const uint8_t*) "\n", 1);
            if (bytes != row.length() || bytes2 != 1)
            {
                FAIL() << "Client can't send data";
            }
        }

        receivedEvent.wait_for(chrono::milliseconds(100));
        this_thread::sleep_for(chrono::milliseconds(50));

        EXPECT_GT(eventCount, 1);

        socketEvents.remove(socket);
        socket.close();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

/**
 * @brief Test SocketEvents communication with echo server using EdgeTriggered mode
 * @remarks The event count must show the event is triggered once data becomes available in client's socket
 */
TEST(SPTK_SocketEvents, minimal_oneShot)
{
    atomic_size_t eventCount;
    Semaphore receivedEvent;

    auto eventsCallback =
        [&eventCount, &receivedEvent](const uint8_t* /*userData*/, SocketEventType eventType)
        {
            if (eventType.m_hangup)
            {
                return SocketEventAction::Forget;
            }
            else
            {
                receivedEvent.post();
                eventCount++;
            }

            return SocketEventAction::Continue;
        };

    SocketEvents socketEvents("Test Pool", eventsCallback, chrono::milliseconds(100),
                              SocketPool::TriggerMode::OneShot);

    Buffer buffer;

    try
    {
        TCPServer echoServer("TestServer", ServerConnection::Type::TCP);
        echoServer.onConnection(echoTestFunction);
        echoServer.listen(testEchoServerPort);

        Strings testRows({"Hello, World!",
                          "This is a test of SocketEvents class.",
                          "Using simple echo server to support data flow."});

        TCPSocket socket;
        socket.open(Host("localhost", testEchoServerPort));

        socketEvents.add(socket, (uint8_t*) &socket);

        for (const auto& row: testRows)
        {
            auto bytes = socket.write((const uint8_t*) row.c_str(), row.length());
            auto bytes2 = socket.write((const uint8_t*) "\n", 1);
            if (bytes != row.length() || bytes2 != 1)
            {
                FAIL() << "Client can't send data";
            }
        }

        receivedEvent.wait_for(chrono::milliseconds(100));
        this_thread::sleep_for(chrono::milliseconds(50));

        EXPECT_EQ(eventCount, 1);

        socketEvents.remove(socket);
        socket.close();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(SPTK_SocketEvents, performance)
{
    SocketEvents socketEvents(
        "test events",
        [](const uint8_t*, SocketEventType) {
            // No need to do anything for this test
            return SocketEventAction::Continue;
        });

    const size_t maxSockets = 512;
    vector<shared_ptr<TCPSocket>> sockets;
    for (size_t index = 0; index < maxSockets; ++index)
    {
        auto socket = make_shared<TCPSocket>();
        socket->open(Host("theater", 80));
        sockets.push_back(socket);
        socketEvents.add(*socket, nullptr);
    }

    StopWatch stopWatch;

    stopWatch.start();
    for (const auto& socket: sockets)
    {
        socketEvents.remove(*socket);
        socketEvents.add(*socket, nullptr);
    }
    stopWatch.stop();

    COUT("Executed " << maxSockets << " add/remove socket ops: "
                     << fixed << setprecision(2) << maxSockets / stopWatch.seconds() / 1E3 << "K/sec" << endl);
}
