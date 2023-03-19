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
            auto res = (int) socket.write(dataPtr, packetSize);
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

static shared_ptr<TCPServer> makePerformanceTestServer(ServerConnection::Type connectionType)
{
    auto pushTcpServer = make_shared<TCPServer>("Performance Test Server", connectionType);

    pushTcpServer->onConnection(performanceTestFunction);

    if (connectionType == ServerConnection::Type::SSL)
    {
        auto keys = make_shared<SSLKeys>(String(TEST_DIRECTORY) + "/keys/mycert.pem", String(TEST_DIRECTORY) + "/keys/mycert.pem");
        pushTcpServer->setSSLKeys(keys);
        pushTcpServer->listen(testSslEchoServerPort);
    }
    else
    {
        pushTcpServer->listen(testTcpEchoServerPort);
    }

    return pushTcpServer;
}

template<typename T>
size_t readAllPackets(T& reader, size_t readSize)
{
    auto readBuffer = make_shared<Buffer>(readSize);

    size_t packetCount = 0;
    for (; packetCount < packetsInTest; ++packetCount)
    {
        auto result = reader.read(readBuffer->data(), 1);
        if (result == 0)
        {
            break;
        }

        result = reader.read(readBuffer->data(), 3);
        if (result == 0)
        {
            break;
        }

        result = reader.read(readBuffer->data(), readSize - 4);
        if (result == 0)
        {
            break;
        }
    }

    reader.close();

    return packetCount;
}

static void printPerformanceTestResult(const String& testLabel, const size_t readSize, const StopWatch& stopWatch, size_t packetCount)
{
    COUT(testLabel << " Reader Received " << packetCount << " packets at the rate " << fixed << setprecision(2) << packetCount / stopWatch.seconds() << "/s, or "
                   << packetCount * readSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s" << endl
                   << endl);
}

static void testTransferPerformance(ServerConnection::Type connectionType, const String& testLabel)
{
    auto pushTcpServer = makePerformanceTestServer(connectionType);

    constexpr size_t readSize {packetSize};
    StopWatch stopWatch;
    String failReason;

    shared_ptr<TCPSocket> socket = connectionType == ServerConnection::Type::TCP
                                       ? make_shared<TCPSocket>()
                                       : make_shared<SSLSocket>();

    auto serverPortNumber = connectionType == ServerConnection::Type::TCP
                                ? testTcpEchoServerPort
                                : testSslEchoServerPort;

    socket->open(Host("localhost", serverPortNumber));

    stopWatch.start();
    size_t packetCount = readAllPackets(*socket, readSize);
    stopWatch.stop();

    printPerformanceTestResult(testLabel, readSize, stopWatch, packetCount);
}

TEST(SPTK_TCPServer, tcpTransferPerformance)
{
    try
    {
        testTransferPerformance(ServerConnection::Type::TCP, "TCP");
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
        testTransferPerformance(ServerConnection::Type::SSL, "SSL");
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

static void testReaderTransferPerformance(ServerConnection::Type connectionType, const String& testLabel)
{
    auto pushTcpServer = makePerformanceTestServer(connectionType);

    shared_ptr<TCPSocket> socket = connectionType == ServerConnection::Type::TCP
                                       ? make_shared<TCPSocket>()
                                       : make_shared<SSLSocket>();

    auto serverPortNumber = connectionType == ServerConnection::Type::TCP
                                ? testTcpEchoServerPort
                                : testSslEchoServerPort;

    socket->open(Host("localhost", serverPortNumber));

    const size_t readerBufferSize = 2048;
    SocketReader socketReader(*socket, readerBufferSize);

    constexpr size_t readSize {packetSize};

    StopWatch stopWatch;
    stopWatch.start();
    size_t packetCount = readAllPackets(socketReader, readSize);
    stopWatch.stop();

    printPerformanceTestResult(testLabel, readSize, stopWatch, packetCount);

    socket->close();
}

TEST(SPTK_TCPServer, tcpReaderTransferPerformance)
{
    try
    {
        testReaderTransferPerformance(ServerConnection::Type::TCP, "TCP");
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(SPTK_TCPServer, sslReaderTransferPerformance)
{
    try
    {
        testReaderTransferPerformance(ServerConnection::Type::TCP, "TCP");
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

#endif
