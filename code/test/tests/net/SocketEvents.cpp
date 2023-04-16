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

#include "sptk5/net/TCPServer.h"
#include <gtest/gtest.h>
#include <sptk5/Printer.h>
#include <sptk5/net/SocketEvents.h>
#include <sptk5/net/SocketReader.h>

using namespace std;
using namespace sptk;
using namespace chrono;

static constexpr uint16_t testEchoServerPort = 5001;

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
                COUT("Server received: " << str);
                socket.write(str);
            }
            else
            {
                break;
            }
        }
        catch (const Exception& e)
        {
            CERR(e.what() << endl);
        }
    }
    socket.close();
}

TEST(SPTK_SocketEvents, minimal)
{
    Semaphore eventReceived;
    shared_ptr<SocketReader> socketReader;

    auto eventsCallback =
        [&eventReceived, &socketReader](const uint8_t* /*userData*/, SocketEventType eventType) {
            Buffer line;
            switch (eventType)
            {
                case SocketEventType::HAS_DATA:
                    while (socketReader->readLine(line, '\n') != 0)
                    {
                        COUT("Client received: " << line.c_str() << endl);
                        eventReceived.post();
                    }
                    break;
                case SocketEventType::CONNECTION_CLOSED:
                    COUT("Socket closed" << endl);
                    break;
                default:
                    COUT("Unknown event" << endl);
                    break;
            }
        };

    SocketEvents socketEvents("Test Pool", eventsCallback, chrono::milliseconds(100));

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

        size_t receivedEventCount {0};
        for (const auto& row: testRows)
        {
            auto bytes = socket.write((const uint8_t*) row.c_str(), row.length());
            auto bytes2 = socket.write((const uint8_t*) "\n", 1);
            if (bytes != row.length() || bytes2 != 1)
            {
                FAIL() << "Client can't send data";
            }
        }

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
