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

#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerListener.h>

#include "TestServer.h"

#ifdef USE_GTEST
#include <gtest/gtest.h>
#endif

using namespace std;
using namespace sptk;

const map<String, LogDetails::MessageDetail> LogDetails::detailNames {
    {"serial_id", MessageDetail::SERIAL_ID},
    {"source_ip", MessageDetail::SOURCE_IP},
    {"request_name", MessageDetail::REQUEST_NAME},
    {"request_duration", MessageDetail::REQUEST_DURATION},
    {"request_data", MessageDetail::REQUEST_DATA},
    {"response_data", MessageDetail::RESPONSE_DATA},
    {"thread_pooling", MessageDetail::THREAD_POOLING}};

LogDetails::LogDetails(const Strings& details)
{
    for (const auto& detailName: details)
    {
        auto itor = detailNames.find(detailName);
        if (itor == detailNames.end())
        {
            continue;
        }
        m_details.insert(itor->second);
    }
}

String LogDetails::toString(const String& delimiter) const
{
    Strings names;
    for (const auto& [name, value]: detailNames)
    {
        if (m_details.find(value) != m_details.end())
        {
            names.push_back(name);
        }
    }
    return names.join(delimiter.c_str());
}

TCPServer::TCPServer(const String& listenerName, ServerConnection::Type connectionType, size_t threadLimit, LogEngine* logEngine, const LogDetails& logDetails)
    : ThreadPool((uint32_t) threadLimit,
                 chrono::minutes(1),
                 listenerName,
                 logDetails.has(LogDetails::MessageDetail::THREAD_POOLING) ? logEngine : nullptr)
    , m_logDetails(logDetails)
    , m_connectionType(connectionType)
{
    if (logEngine != nullptr)
    {
        m_logger = make_shared<Logger>(*logEngine);
    }

    constexpr unsigned maxHostNameLength = 128;
    array<char, maxHostNameLength> hostname = {"localhost"};
    int result = gethostname(hostname.data(), sizeof(hostname));
    if (result == 0)
    {
        m_host = Host(hostname.data());
    }
}

TCPServer::~TCPServer()
{
    TCPServer::stop();
}

const Host& TCPServer::host() const
{
    SharedLock(m_mutex);
    return m_host;
}

void TCPServer::host(const Host& host)
{
    SharedLock(m_mutex);
    if (!m_listenerThread)
    {
        m_host = host;
    }
}

void TCPServer::listen(uint16_t port)
{
    UniqueLock(m_mutex);
    m_host.port(port);
    m_listenerThread = make_shared<TCPServerListener>(this, port);
    m_listenerThread->listen();
}

bool TCPServer::allowConnection(sockaddr_in*)
{
    return true;
}

void TCPServer::stop()
{
    UniqueLock(m_mutex);
    ThreadPool::stop();
    if (m_listenerThread)
    {
        m_listenerThread->stop();
        m_listenerThread.reset();
    }
}

void TCPServer::setSSLKeys(shared_ptr<SSLKeys> sslKeys)
{
    UniqueLock(m_mutex);
    m_sslKeys = std::move(sslKeys);
}

const SSLKeys& TCPServer::getSSLKeys() const
{
    SharedLock(m_mutex);
    return *m_sslKeys;
}

void TCPServer::threadEvent(Thread* thread, Type eventType, SRunable runable)
{
    if (eventType == Type::RUNABLE_FINISHED)
    {
        runable.reset();
    }
    ThreadPool::threadEvent(thread, eventType, runable);
}

SServerConnection TCPServer::createConnection(SOCKET connectionSocket, const sockaddr_in* peer)
{
    if (m_connectionType == ServerConnection::Type::TCP)
    {
        return make_shared<TCPServerConnection>(*this, connectionSocket, peer, m_connectionFunction);
    }
    else
    {
        return make_shared<SSLServerConnection>(*this, connectionSocket, peer, m_connectionFunction);
    }
    return sptk::SServerConnection();
}

void TCPServer::onConnection(const ServerConnection::Function& function)
{
    m_connectionFunction = function;
}

#ifdef USE_GTEST

static constexpr uint16_t testEchoServerPort = 3001;

static void echoConnectionFunction(Runable& task, TCPSocket& socket, const String& address)
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

TEST(SPTK_TCPServer, tcpMinimal)
{
    Buffer buffer;

    try
    {
        TestServer<EchoConnection> echoServer;
        echoServer.listen(testEchoServerPort);

        TCPSocket socket;
        socket.open(Host("localhost", testEchoServerPort));

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
        TestServer<EchoSslConnection> echoServer;

        auto keys = make_shared<SSLKeys>(String(TEST_DIRECTORY) + "/keys/mycert.pem", String(TEST_DIRECTORY) + "/keys/mycert.pem");
        echoServer.setSSLKeys(keys);

        echoServer.listen(testEchoServerPort);
        this_thread::sleep_for(chrono::milliseconds(100));

        SSLSocket socket;
        socket.open(Host("localhost", testEchoServerPort));

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
        TestServer<PushConnection> echoServer;

        auto keys = make_shared<SSLKeys>(String(TEST_DIRECTORY) + "/keys/test.key", String(TEST_DIRECTORY) + "/keys/test.cert");
        echoServer.setSSLKeys(keys);

        echoServer.listen(testEchoServerPort);

        TCPSocket socket;
        socket.open(Host("localhost", testEchoServerPort));

        constexpr size_t readSize {50};
        auto readBuffer = make_shared<Buffer>(readSize);

        size_t packetCount = 0;

        StopWatch stopWatch;
        stopWatch.start();

        while (true)
        {
            auto rc = socket.recv(readBuffer->data(), readSize);
            if (rc == 0)
                break;
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
        TestServer<PushSslConnection> pushSslServer;

        auto keys = make_shared<SSLKeys>(String(TEST_DIRECTORY) + "/keys/mycert.pem", String(TEST_DIRECTORY) + "/keys/mycert.pem");
        pushSslServer.setSSLKeys(keys);

        pushSslServer.listen(testEchoServerPort);

        SSLSocket socket;
        socket.open(Host("localhost", testEchoServerPort));
        //socket.blockingMode(false);

        constexpr size_t readSize {50};
        auto readBuffer = make_shared<Buffer>(readSize);

        size_t packetCount = 0;

        StopWatch stopWatch;
        stopWatch.start();

        while (true)
        {
            auto rc = socket.recv(readBuffer->data(), readSize);
            if (rc == 0)
                break;
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
