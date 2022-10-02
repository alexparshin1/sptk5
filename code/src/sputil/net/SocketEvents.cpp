/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Printer.h>
#include <sptk5/cutils>
#include <sptk5/net/SocketEvents.h>
#include <sptk5/net/TCPSocket.h>

#ifdef USE_GTEST
#include "sptk5/net/TCPServer.h"
#include <gtest/gtest.h>
#endif

using namespace std;
using namespace sptk;
using namespace chrono;

SocketEvents::SocketEvents(const String& name, const SocketEventCallback& eventsCallback, milliseconds timeout)
    : Thread(name)
    , m_socketPool(eventsCallback)
    , m_timeout(timeout)
{
    m_socketPool.open();
}

SocketEvents::~SocketEvents()
{
    stop();
}

void SocketEvents::stop()
{
    try
    {
        m_socketPool.close();
        if (running())
        {
            terminate();
            join();
        }
    }
    catch (const Exception& e)
    {
        CERR(e.message() << endl)
    }
}

void SocketEvents::add(BaseSocket& socket, uint8_t* userData)
{
    if (!running())
    {
        scoped_lock lock(m_mutex);
        if (m_shutdown)
        {
            throw Exception("SocketEvents already stopped");
        }
        run();
        m_started.wait_for(true, seconds(1));
    }
    m_socketPool.watchSocket(socket, userData);
}

void SocketEvents::remove(BaseSocket& socket)
{
    m_socketPool.forgetSocket(socket);
}

void SocketEvents::threadFunction()
{
    m_socketPool.open();
    m_started = true;
    while (!terminated())
    {
        try
        {
            m_socketPool.waitForEvents(m_timeout);
        }
        catch (const Exception& e)
        {
            if (m_socketPool.active())
            {
                CERR(e.message() << endl)
            }
            else
            {
                break;
            }
        }
    }
    m_socketPool.close();
}

void SocketEvents::terminate()
{
    Thread::terminate();
    scoped_lock lock(m_mutex);
    m_shutdown = true;
}

size_t SocketEvents::size() const
{
    scoped_lock lock(m_mutex);
    return m_watchList.size();
}

#ifdef USE_GTEST

static constexpr uint16_t testEchoServerPort = 5001;

static void echoTestFunction(const Runable& task, TCPSocket& socket, const String& /*address*/)
{
    Buffer data;
    while (!task.terminated())
    {
        try
        {
            if (socket.readyToRead(chrono::seconds(1)))
            {
                if (socket.readLine(data) == 0)
                {
                    return;
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
            CERR(e.what() << endl)
        }
    }
    socket.close();
}

TEST(SPTK_SocketEvents, minimal)
{
    Semaphore eventReceived;
    auto eventsCallback = [&eventReceived](uint8_t* userData, SocketEventType eventType) {
        auto* reader = (TCPSocket*) userData;
        String line;
        switch (eventType)
        {
            case SocketEventType::HAS_DATA:
                reader->readLine(line);
                COUT("Socket has data: " << line << endl)
                eventReceived.post();
                break;
            case SocketEventType::CONNECTION_CLOSED:
                COUT("Socket closed" << endl)
                break;
            default:
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
                          "The session is terminated when this row is received",
                          ""});

        TCPSocket socket;
        socket.open(Host("localhost", testEchoServerPort));

        socketEvents.add(socket, (uint8_t*) &socket);

        size_t receivedEventCount {0};
        String hello("Hello, World!\n");
        auto sentBytes = socket.send((const uint8_t*) hello.c_str(), hello.length());
        if (eventReceived.sleep_for(chrono::milliseconds(10)))
        {
            receivedEventCount++;
        }
        socketEvents.remove(socket);
        socket.close();
        EXPECT_EQ(1u, receivedEventCount);
        EXPECT_EQ(hello.length(), sentBytes);
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

#endif
