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

#include <future>
#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

#ifdef USE_GTEST

static constexpr uint16_t testTcpEchoServerPort = 3001;
static constexpr uint16_t testSslEchoServerPort = 3002;

static const size_t packetsInTest = 100000;
static const size_t packetSize = 50;

class TestSSLServer : public TCPServer
{
public:
    TestSSLServer();
    ~TestSSLServer() override = default;

private:
    std::future<void> m_writer;
    SynchronizedQueue<Buffer> echoQueue;
};

TestSSLServer::TestSSLServer()
    : TCPServer("localhost", testSslEchoServerPort)
{
}


static Buffer makePacket()
{
    const uint8_t fiveBits = 0x3F;
    Buffer data;

    data.append("BOF", 3);
    data.append(packetSize);
    for (size_t i = 0; i < packetSize; ++i)
    {
        data.append(uint8_t(i % fiveBits + 32));
    }

    return data;
}

static void performanceTestFunction(const Runable& /*task*/, TCPSocket& socket, const String& /*address*/)
{
    Buffer data;
    StopWatch stopWatch;
    stopWatch.start();

    for (size_t packetNumber = 0; packetNumber < packetsInTest; ++packetNumber)
    {
        try
        {
            if (socket.readyToRead(chrono::milliseconds(1000)))
            {
                auto res = socket.read(data, packetSize + 3 + sizeof(size_t));
                if (res < packetSize + 3 + sizeof(size_t))
                {
                    throwSocketError("Error reading from socket");
                }
                res = (int) socket.write(data);
                if (res < 0)
                {
                    throwSocketError("Error writing to socket");
                }
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

static shared_ptr<TCPServer> makePerformanceTestServer(ServerConnection::Type connectionType)
{
    auto pushTcpServer = make_shared<TCPServer>("Performance Test Server");

    pushTcpServer->onConnection(performanceTestFunction);

    if (connectionType == ServerConnection::Type::SSL)
    {
        auto keys = make_shared<SSLKeys>(String(TEST_DIRECTORY) + "/keys/mycert.pem", String(TEST_DIRECTORY) + "/keys/mycert.pem");
        pushTcpServer->setSSLKeys(keys);
    }

    pushTcpServer->addListener(ServerConnection::Type::TCP, testTcpEchoServerPort);
    pushTcpServer->addListener(ServerConnection::Type::SSL, testSslEchoServerPort);

    return pushTcpServer;
}

template<typename T>
size_t readAllPackets(T& reader, size_t readSize)
{
    auto readBuffer = make_shared<Buffer>(readSize);
    Buffer bof(3);

    size_t packetCount = 0;
    for (; packetCount < packetsInTest; ++packetCount)
    {
        size_t thisPacketSize = 0;
        if (reader.read(bof, 3) == 0 ||
            reader.read((uint8_t*) &thisPacketSize, sizeof(size_t)) == 0 ||
            reader.read(readBuffer->data(), thisPacketSize) == 0)
        {
            break;
        }
        if (String(bof) != "BOF")
        {
            cerr << "Invalid data: expected BOF got '" << String(bof) << "'" << endl;
        }
        if (thisPacketSize != packetSize)
        {
            cerr << "Invalid packet size: expected " << packetSize << " got " << thisPacketSize << endl;
        }
    }

    reader.close();

    return packetCount;
}

static void printPerformanceTestResult(const String& testLabel, const size_t readSize, const StopWatch& stopWatch, size_t packetCount)
{
    COUT(testLabel << " received " << packetCount << " packets at the rate " << fixed << setprecision(2) << packetCount / stopWatch.seconds() << "/s, or "
                   << packetCount * readSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s" << endl
                   << endl);
}

static void testTransferPerformance(ServerConnection::Type connectionType, const String& testLabel)
{
    auto pushTcpServer = makePerformanceTestServer(connectionType);

    constexpr size_t readSize {packetSize};
    StopWatch stopWatch;

    shared_ptr<TCPSocket> socket = connectionType == ServerConnection::Type::TCP
                                       ? make_shared<TCPSocket>()
                                       : make_shared<SSLSocket>();

    auto serverPortNumber = connectionType == ServerConnection::Type::TCP
                                ? testTcpEchoServerPort
                                : testSslEchoServerPort;

    socket->open(Host("localhost", serverPortNumber));

    auto readerThread = async(launch::async,
                              [socket, readSize] {
                                  return readAllPackets(*socket, readSize);
                              });

    auto writerThread = async(launch::async,
                              [socket] {
                                  Buffer packet = makePacket();
                                  for (size_t i = 0; i < packetsInTest; ++i)
                                  {
                                      socket->write(packet);
                                  }
                              });

    stopWatch.start();

    readerThread.wait();
    writerThread.wait();

    size_t packetCount = readerThread.get();

    stopWatch.stop();

    printPerformanceTestResult(testLabel, readSize, stopWatch, packetCount);
}

TEST(SPTK_SSLServer, sslPerformance)
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


#endif
