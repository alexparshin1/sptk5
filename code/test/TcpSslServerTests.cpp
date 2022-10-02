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

#include <sptk5/cutils>

#include <sptk5/net/SSLServerConnection.h>
#include <sptk5/net/TCPServerListener.h>

#ifdef USE_GTEST
#include <gtest/gtest.h>
#endif

using namespace std;
using namespace sptk;

#ifdef USE_GTEST

static constexpr uint16_t testTcpEchoServerPort = 3001;
static constexpr uint16_t testSslEchoServerPort = 3002;

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

static void performanceTestFunction(const Runable& /*task*/, TCPSocket& socket, const String& /*address*/)
{
    const size_t packetCount = 100000;
    const size_t packetSize = 50;
    Buffer data(packetSize);

    for (size_t i = 0; i < packetSize; ++i)
    {
        data[i] = uint8_t(i % 255);
    }
    data.bytes(packetSize);

    StopWatch stopWatch;
    stopWatch.start();

    for (size_t packetNumber = 0; packetNumber < packetCount; ++packetNumber)
    {
        try
        {
            int res = (int) socket.send(data.data(), packetSize);
            if (res < 0)
            {
                throwSocketError("Error writing to socket", __FILE__, __LINE__);
            }
        }
        catch (const Exception& e)
        {
            CERR(e.what() << endl)
        }
    }
    stopWatch.stop();

    COUT("Sent " << packetCount << " packets at the rate " << fixed << setprecision(2) << packetCount / stopWatch.seconds() << "/s, or "
                 << packetCount * packetSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s" << endl)

    socket.close();
}

TEST(SPTK_TCPServer, tcpMinimal)
{
    Buffer buffer;

    try
    {
        TCPServer echoServer("TestServer", ServerConnection::Type::TCP);
        echoServer.onConnection(echoTestFunction);
        echoServer.listen(testTcpEchoServerPort);

        TCPSocket socket;
        socket.open(Host("localhost", testTcpEchoServerPort));

        Strings rows("Hello, World!\n"
                     "This is a test of TCPServer class.\n"
                     "Using simple echo server to verify data flow.\n"
                     "The session is terminated when this row is received",
                     "\n");

        int rowCount = 0;
        for (const auto& row: rows)
        {
            socket.write(row + "\n");
            buffer.bytes(0);
            if (socket.readyToRead(chrono::seconds(3)))
            {
                socket.readLine(buffer);
            }
            EXPECT_STREQ(row.c_str(), buffer.c_str());
            ++rowCount;
        }
        EXPECT_EQ(4, rowCount);

        socket.close();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(SPTK_TCPServer, sslMinimal)
{
    Buffer buffer;

    try
    {
        TCPServer echoServer("TestServer", ServerConnection::Type::SSL);
        echoServer.onConnection(echoTestFunction);

        auto keys = make_shared<SSLKeys>(String(TEST_DIRECTORY) + "/keys/mycert.pem", String(TEST_DIRECTORY) + "/keys/mycert.pem");
        echoServer.setSSLKeys(keys);

        echoServer.listen(testSslEchoServerPort);
        this_thread::sleep_for(chrono::milliseconds(100));

        SSLSocket socket;
        socket.open(Host("localhost", testSslEchoServerPort));

        Strings rows("Hello, World!\n"
                     "This is a test of TCPServer class.\n"
                     "Using simple echo server to verify data flow.\n"
                     "The session is terminated when this row is received",
                     "\n");

        int rowCount = 0;
        for (const auto& row: rows)
        {
            socket.write(row + "\n");
            buffer.bytes(0);
            if (socket.readyToRead(chrono::seconds(3)))
            {
                socket.readLine(buffer);
            }
            EXPECT_STREQ(row.c_str(), buffer.c_str());
            ++rowCount;
        }
        EXPECT_EQ(4, rowCount);

        socket.close();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(SPTK_TCPServer, tcpTransferPerformance)
{
    try
    {
        TCPServer pushTcpServer("Performance Test Server", ServerConnection::Type::TCP);
        pushTcpServer.onConnection(performanceTestFunction);
        pushTcpServer.listen(testTcpEchoServerPort);

        TCPSocket socket;
        socket.open(Host("localhost", testTcpEchoServerPort));

        constexpr size_t readSize {50};
        auto readBuffer = make_shared<Buffer>(readSize);

        size_t packetCount = 0;

        StopWatch stopWatch;
        stopWatch.start();

        while (true)
        {
            if (auto rc = socket.recv(readBuffer->data(), readSize);
                rc == 0)
            {
                break;
            }
            ++packetCount;
        }

        readBuffer.reset();

        stopWatch.stop();

        COUT("Received " << packetCount << " packets at the rate " << fixed << setprecision(2) << packetCount / stopWatch.seconds() << "/s, or "
                         << packetCount * readSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s" << endl)

        socket.close();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(SPTK_TCPServer, sslTransferPerformance)
{
    try
    {
        TCPServer pushSslServer("Performance Test Server", ServerConnection::Type::SSL);
        pushSslServer.onConnection(performanceTestFunction);

        auto keys = make_shared<SSLKeys>(String(TEST_DIRECTORY) + "/keys/mycert.pem", String(TEST_DIRECTORY) + "/keys/mycert.pem");
        pushSslServer.setSSLKeys(keys);

        pushSslServer.listen(testSslEchoServerPort);

        SSLSocket socket;
        socket.open(Host("localhost", testSslEchoServerPort));
        socket.blockingMode(false);

        constexpr size_t readSize {50};
        auto readBuffer = make_shared<Buffer>(readSize);

        size_t packetCount = 0;

        StopWatch stopWatch;
        stopWatch.start();

        while (true)
        {
            if (auto rc = socket.recv(readBuffer->data(), readSize);
                rc == 0)
            {
                break;
            }
            ++packetCount;
        }

        readBuffer.reset();

        stopWatch.stop();

        COUT("Received " << packetCount << " packets at the rate " << fixed << setprecision(2) << packetCount / stopWatch.seconds() << "/s, or "
                         << packetCount * readSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s" << endl)

        socket.close();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

#endif
