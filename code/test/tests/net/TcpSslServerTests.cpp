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

#include <sptk5/cutils>
#include <sptk5/net/SSLServerConnection.h>
#include <sptk5/net/TCPServerListener.h>

#include "sptk5/net/SocketReader.h"
#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

namespace {

constexpr uint16_t testTcpEchoServerPort = 3001;
constexpr uint16_t testSslEchoServerPort = 3002;

void echoTestFunction(ServerConnection& connection)
{
    SocketReader reader(connection.socket());

    COUT("Server connection started\n");
    Buffer data;
    while (true)
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
                connection.socket().write(str);
            }
        }
        catch (const Exception& e)
        {
            CERR(e.what());
            break;
        }
    }
    connection.socket().close();
    COUT("Server connection closed\n");
}

constexpr size_t packetsInTest = 100000;
constexpr size_t packetSize = 50;

void performanceTestFunction(ServerConnection& serverConnection)
{
    Buffer data(packetSize);

    for (size_t i = 0; i < packetSize; ++i)
    {
        constexpr uint8_t eightBits = 255;
        data[i] = static_cast<uint8_t>(i % eightBits);
    }
    data.bytes(packetSize);

    StopWatch stopWatch;
    stopWatch.start();

    const auto* dataPtr = data.data();
    for (size_t packetNumber = 0; packetNumber < packetsInTest; ++packetNumber)
    {
        try
        {
            if (const auto res = static_cast<int>(serverConnection.socket().write(dataPtr, packetSize));
                res < 0)
            {
                throwSocketError("Error writing to socket");
            }
        }
        catch (const Exception& e)
        {
            CERR(e.what());
        }
    }
    stopWatch.stop();

    COUT("Sent " << packetsInTest << " packets at the rate " << fixed << setprecision(2) << packetsInTest / stopWatch.seconds() << "/s, or "
                 << packetsInTest * packetSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s\n");

    if (constexpr chrono::seconds timeout(10);
        !serverConnection.socket().readyToRead(timeout))
    {
        CERR("Timeout waiting for response");
    }
    serverConnection.socket().close();
}

} // namespace

TEST(SPTK_TCPServer, tcpMinimal)
{
    Buffer buffer;

    try
    {
        TCPServer echoServer("TestServer");
        echoServer.onConnection(echoTestFunction);
        echoServer.addListener(ServerConnection::Type::TCP, testTcpEchoServerPort);

        TCPSocket    socket;
        SocketReader socketReader(socket);

        socket.open(Host("localhost", testTcpEchoServerPort));

        const Strings rows({
            "Hello, World!",
            "This is a test of TCPServer class.",
            "Using simple echo server to verify data flow.",
            "The session is terminated when this row is received.",
        });

        this_thread::sleep_for(5ms);

        int rowCount = 0;
        for (const auto& row: rows)
        {
            socket.write(row + "\n");
            buffer.bytes(0);
            if (socketReader.readyToRead(3s))
            {
                socketReader.readLine(buffer);
            }
            EXPECT_STREQ(row.c_str(), buffer.c_str());
            ++rowCount;
        }

        EXPECT_EQ(4, rowCount);
        socket.close();

        COUT("Client connection closed\n");

        echoServer.stop();
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
        TCPServer echoServer("TestServer");
        echoServer.onConnection(echoTestFunction);

        const auto keys = make_shared<SSLKeys>(String(TEST_DIRECTORY) + "/keys/mycert.pem", String(TEST_DIRECTORY) + "/keys/mycert.pem");

        if (!filesystem::exists(keys->certificateFileName()))
        {
            GTEST_SKIP() << "Certificate file " << keys->certificateFileName() << " does not exist.";
        }

        echoServer.setSSLKeys(keys);

        echoServer.addListener(ServerConnection::Type::TCP, testTcpEchoServerPort);
        echoServer.addListener(ServerConnection::Type::SSL, testSslEchoServerPort);
        this_thread::sleep_for(100ms);

        SSLSocket    socket;
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
            if (socketReader.readyToRead(3s))
            {
                socketReader.readLine(buffer);
            }
            EXPECT_STREQ(row.c_str(), buffer.c_str());
            ++rowCount;
        }
        EXPECT_EQ(4, rowCount);

        socket.close();

        echoServer.stop();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

namespace {
shared_ptr<TCPServer> makePerformanceTestServer(ServerConnection::Type connectionType)
{
    auto pushTcpServer = make_shared<TCPServer>("Performance Test Server");

    pushTcpServer->onConnection(performanceTestFunction);

    if (connectionType == ServerConnection::Type::SSL)
    {
        const auto keys = make_shared<SSLKeys>(String(TEST_DIRECTORY) + "/keys/mycert.pem", String(TEST_DIRECTORY) + "/keys/mycert.pem");
        pushTcpServer->setSSLKeys(keys);
    }

    pushTcpServer->addListener(ServerConnection::Type::TCP, testTcpEchoServerPort);
    pushTcpServer->addListener(ServerConnection::Type::SSL, testSslEchoServerPort);

    return pushTcpServer;
}

template<typename T>
size_t readAllPackets(T& reader, size_t readSize)
{
    const auto readBuffer = make_shared<Buffer>(readSize);

    size_t packetCount = 0;
    for (; packetCount < packetsInTest; ++packetCount)
    {
        if (reader.read(readBuffer->data(), 1) == 0 ||
            reader.read(readBuffer->data(), 3) == 0 ||
            reader.read(readBuffer->data(), readSize - 4) == 0)
        {
            break;
        }
    }

    reader.close();

    return packetCount;
}

void printPerformanceTestResult(const String& testLabel, const size_t readSize, const StopWatch& stopWatch, size_t packetCount)
{
    COUT(testLabel << " received " << packetCount
                   << " packets at the rate " << fixed << setprecision(2) << static_cast<double>(packetCount) / stopWatch.seconds() << "/s, or "
                   << static_cast<double>(packetCount * readSize) / stopWatch.seconds() / 1024 / 1024 << " Mb/s\n\n");
}

void testTransferPerformance(ServerConnection::Type connectionType, const String& testLabel)
{
    auto pushTcpServer = makePerformanceTestServer(connectionType);

    constexpr size_t readSize {packetSize};
    StopWatch        stopWatch;

    const shared_ptr<TCPSocket> socket = connectionType == ServerConnection::Type::TCP
                                             ? make_shared<TCPSocket>()
                                             : make_shared<SSLSocket>();

    const auto serverPortNumber = connectionType == ServerConnection::Type::TCP
                                      ? testTcpEchoServerPort
                                      : testSslEchoServerPort;

    socket->open(Host("localhost", serverPortNumber));

    stopWatch.start();
    const size_t packetCount = readAllPackets(*socket, readSize);
    stopWatch.stop();

    printPerformanceTestResult(testLabel, readSize, stopWatch, packetCount);
}
} // namespace

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

namespace {
void testReaderTransferPerformance(ServerConnection::Type connectionType, const String& testLabel)
{
    auto pushTcpServer = makePerformanceTestServer(connectionType);

    const shared_ptr<TCPSocket> socket = connectionType == ServerConnection::Type::TCP
                                             ? make_shared<TCPSocket>()
                                             : make_shared<SSLSocket>();

    const auto serverPortNumber = connectionType == ServerConnection::Type::TCP
                                      ? testTcpEchoServerPort
                                      : testSslEchoServerPort;

    socket->open(Host("localhost", serverPortNumber));

    constexpr size_t readerBufferSize = 2048;
    SocketReader     socketReader(*socket, readerBufferSize);

    constexpr size_t readSize {packetSize};

    StopWatch stopWatch;
    stopWatch.start();
    const size_t packetCount = readAllPackets(socketReader, readSize);
    stopWatch.stop();

    printPerformanceTestResult(testLabel, readSize, stopWatch, packetCount);

    socket->close();
}
} // namespace

TEST(SPTK_TCPServer, tcpReaderTransferPerformance)
{
    try
    {
        testReaderTransferPerformance(ServerConnection::Type::TCP, "TCPReader");
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
        testReaderTransferPerformance(ServerConnection::Type::SSL, "SSLReader");
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}
