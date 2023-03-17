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

#include <sptk5/cutils>
#include <sptk5/net/SSLServerConnection.h>
#include <sptk5/net/TCPServerListener.h>

#include "sptk5/net/SocketReader.h"
#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

#ifdef USE_GTEST

static constexpr uint16_t testTcpEchoServerPort = 3001;
static constexpr uint16_t testSslEchoServerPort = 3002;

static void echoTestFunction(const Runable& task, TCPSocket& socket, const String& /*address*/)
{
    SocketReader reader(socket);

    Buffer data;
    while (!task.terminated())
    {
        try
        {
            if (reader.readyToRead(chrono::seconds(1)))
            {
                if (reader.readLine(data) == 0)
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
            CERR(e.what() << endl);
        }
    }
    socket.close();
}

static const size_t packetsInTest = 100000;
static const size_t packetSize = 50;

static void performanceTestFunction(const Runable& /*task*/, TCPSocket& socket, const String& /*address*/)
{
    const uint8_t eightBits = 255;
    Buffer data(packetSize);

    for (size_t i = 0; i < packetSize; ++i)
    {
        data[i] = uint8_t(i % eightBits);
    }
    data.bytes(packetSize);

    StopWatch stopWatch;
    stopWatch.start();

    const auto* dataPtr = data.data();
    for (size_t packetNumber = 0; packetNumber < packetsInTest; ++packetNumber)
    {
        try
        {
            auto res = (int) socket.send(dataPtr, packetSize);
            if (res < 0)
            {
                throwSocketError("Error writing to socket");
            }
        }
        catch (const Exception& e)
        {
            CERR(e.what() << endl);
        }
    }
    stopWatch.stop();

    COUT("Sent " << packetsInTest << " packets at the rate " << fixed << setprecision(2) << packetsInTest / stopWatch.seconds() << "/s, or "
                 << packetsInTest * packetSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s" << endl);


    if (const chrono::seconds timeout(10);
        !socket.readyToRead(timeout))
    {
        CERR("Timeout waiting for response" << endl);
    }
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
        SocketReader socketReader(socket);

        socket.open(Host("localhost", testTcpEchoServerPort));

        const Strings rows("Hello, World!\n"
                           "This is a test of TCPServer class.\n"
                           "Using simple echo server to verify data flow.\n"
                           "The session is terminated when this row is received",
                           "\n");

        int rowCount = 0;
        for (const auto& row: rows)
        {
            socket.write(row + "\n");
            buffer.bytes(0);
            if (socketReader.readyToRead(chrono::seconds(3)))
            {
                socketReader.readLine(buffer);
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
    const chrono::milliseconds smallDelay(100);

    try
    {
        TCPServer echoServer("TestServer", ServerConnection::Type::SSL);
        echoServer.onConnection(echoTestFunction);

        auto keys = make_shared<SSLKeys>(String(TEST_DIRECTORY) + "/keys/mycert.pem", String(TEST_DIRECTORY) + "/keys/mycert.pem");
        echoServer.setSSLKeys(keys);

        echoServer.listen(testSslEchoServerPort);
        this_thread::sleep_for(smallDelay);

        SSLSocket socket;
        SocketReader socketReader(socket);

        socket.open(Host("localhost", testSslEchoServerPort));

        const Strings rows("Hello, World!\n"
                           "This is a test of TCPServer class.\n"
                           "Using simple echo server to verify data flow.\n"
                           "The session is terminated when this row is received",
                           "\n");

        int rowCount = 0;
        for (const auto& row: rows)
        {
            socket.write(row + "\n");
            buffer.bytes(0);
            if (socketReader.readyToRead(chrono::seconds(3)))
            {
                socketReader.readLine(buffer);
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

        constexpr size_t readSize {packetSize};
        auto readBuffer = make_shared<Buffer>(readSize);

        size_t packetCount = 0;

        StopWatch stopWatch;
        stopWatch.start();

        auto* readBufferPtr = readBuffer->data();
        while (packetCount < packetsInTest)
        {
            if (auto result = socket.recv(readBufferPtr, readSize);
                result == 0)
            {
                break;
            }
            ++packetCount;
        }

        readBuffer.reset();

        stopWatch.stop();

        COUT("Received " << packetCount << " packets at the rate " << fixed << setprecision(2) << packetCount / stopWatch.seconds() << "/s, or "
                         << packetCount * readSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s" << endl);

        socket.close();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(SPTK_TCPServer, sslTransferPerformance)
{
    TCPServer pushSslServer("Performance Test Server", ServerConnection::Type::SSL);
    pushSslServer.onConnection(performanceTestFunction);

    auto keys = make_shared<SSLKeys>(String(TEST_DIRECTORY) + "/keys/mycert.pem", String(TEST_DIRECTORY) + "/keys/mycert.pem");
    pushSslServer.setSSLKeys(keys);

    size_t packetCount = 0;

    constexpr size_t readSize {packetSize};
    StopWatch stopWatch;
    String failReason;

    try
    {
        pushSslServer.listen(testSslEchoServerPort);

        SSLSocket socket;
        socket.open(Host("localhost", testSslEchoServerPort));
        socket.blockingMode(false);

        auto readBuffer = make_shared<Buffer>(readSize);

        stopWatch.start();

        while (packetCount < packetsInTest)
        {
            auto result = socket.recv(readBuffer->data(), readSize);
            if (result == 0)
            {
                break;
            }
            if (result != readSize)
            {
                throw Exception("Incomplete read");
            }
            ++packetCount;
        }
        stopWatch.stop();

        socket.close();
    }
    catch (const Exception& e)
    {
        stopWatch.stop();
        failReason = e.what();
    }

    COUT("Received " << packetCount << " packets for " << fixed << setprecision(2) << stopWatch.seconds() << " sec, at the rate " << packetCount / stopWatch.seconds() << "/s, or "
                     << packetCount * readSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s" << endl);

    if (!failReason.empty())
    {
        FAIL() << failReason;
    }
}

TEST(SPTK_TCPServer, tcpTransferReaderPerformance)
{
    try
    {
        TCPServer pushTcpServer("Performance Test Server", ServerConnection::Type::TCP);
        pushTcpServer.onConnection(performanceTestFunction);
        pushTcpServer.listen(testTcpEchoServerPort);

        TCPSocket socket;
        const size_t readerBufferSize = 1024;
        SocketReader socketReader(socket, readerBufferSize);
        socket.open(Host("localhost", testTcpEchoServerPort));

        constexpr size_t readSize {packetSize};
        auto readBuffer = make_shared<Buffer>(readSize);

        size_t packetCount = 0;

        StopWatch stopWatch;
        stopWatch.start();

        auto* readBufferPtr = readBuffer->data();
        while (packetCount < packetsInTest)
        {
            if (socketReader.read(readBufferPtr, 1) != 1)
            {
                COUT("Incomplete read" << endl);
                break;
            }
            if (socketReader.read(readBufferPtr, 3) != 3)
            {
                COUT("Incomplete read" << endl);
                break;
            }
            if (socketReader.read(readBufferPtr, readSize - 4) != readSize - 4)
            {
                COUT("Incomplete read" << endl);
                break;
            }
            ++packetCount;
        }

        readBuffer.reset();

        stopWatch.stop();

        COUT("Received " << packetCount << " packets at the rate " << fixed << setprecision(2) << packetCount / stopWatch.seconds() << "/s, or "
                         << packetCount * readSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s" << endl);

        socket.close();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

#endif
