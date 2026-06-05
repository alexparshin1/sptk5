/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <gtest/gtest.h>
#include <sptk5/cutils>
#include <sptk5/net/FastTCPServer.h>
#include <sptk5/net/SSLSocket.h>

#include "test/TestData.h"

#include <atomic>
#include <filesystem>
#include <iomanip>
#include <vector>

#ifndef _WIN32
#include <netinet/tcp.h>
#endif

using namespace std;
using namespace sptk;

namespace {

constexpr uint16_t testPort = 12399;
constexpr size_t   messageSize = 100;

/**
 * @brief Test server that counts every byte received and signals when a target is reached.
 *
 * All reads happen on the single SocketEvents reactor thread, so no extra
 * synchronization is needed around the receive buffer.
 */
class CountingServer : public FastTCPServer
{
public:
    explicit CountingServer(const string& name)
        : FastTCPServer(name)
    {
    }

    ~CountingServer() override
    {
        // Must stop the reactor before our members are destroyed.
        stop();
    }

    atomic<size_t> bytesReceived {0}; ///< Total bytes received across all connections.
    Semaphore      done;              ///< Posted once targetBytes have been received.
    size_t         targetBytes {0};   ///< Target byte count.

protected:
    void socketEventCallback(const shared_ptr<ServerConnection>& connection, const SocketEventType eventType) override
    {
        if (eventType.m_hangup || eventType.m_error)
        {
            closeConnection(connection);
            return;
        }

        if (!eventType.m_data)
        {
            return;
        }

        const auto socket = connection->getSocket();
        auto       available = socket->socketBytes();
        if (available == 0)
        {
            return;
        }
        available = min(available, m_buffer.size());

        const auto bytes = socket->read(m_buffer.data(), available);
        if (bytes == 0)
        {
            return;
        }

        if (const auto total = bytesReceived.fetch_add(bytes) + bytes;
            targetBytes != 0 && total >= targetBytes)
        {
            done.post();
        }
    }

private:
    vector<uint8_t> m_buffer = vector<uint8_t>(64 * 1024);
};

/**
 * @brief Test server that echoes back everything it receives.
 */
class EchoServer : public FastTCPServer
{
public:
    explicit EchoServer(const string& name, size_t ackSize = 0)
        : FastTCPServer(name)
        , m_messageSize(messageSize)
        , m_ackSize(ackSize)
    {
    }

    ~EchoServer() override
    {
        stop();
    }

protected:
    void socketEventCallback(const shared_ptr<ServerConnection>& connection, const SocketEventType eventType) override
    {
        if (eventType.m_hangup || eventType.m_error)
        {
            closeConnection(connection);
            return;
        }

        if (!eventType.m_data)
        {
            return;
        }

        const auto socket = connection->getSocket();
        auto       available = socket->socketBytes();
        while (available >= m_messageSize)
        {
            const auto bytes = socket->read(m_buffer.data(), m_messageSize);
            const auto echoSize = m_ackSize ? min(m_ackSize, bytes) : bytes;
            socket->write(m_buffer.data(), echoSize);
            available -= m_messageSize;
            ++m_sentMessages;
        }
    }

private:
    vector<uint8_t> m_buffer = vector<uint8_t>(64 * 1024);
    size_t          m_messageSize {0};
    size_t          m_ackSize {0};
    size_t          m_sentMessages {0};
};

/**
 * @brief Test server that counts accepted connections and signals when a target is reached.
 *
 * The count is incremented in createConnection(), which the listener calls for
 * every accepted connection.
 */
class AcceptCountingServer : public FastTCPServer
{
public:
    explicit AcceptCountingServer(const string& name)
        : FastTCPServer(name)
    {
    }

    ~AcceptCountingServer() override
    {
        stop();
    }

    atomic<size_t> accepted {0}; ///< Number of accepted connections.
    Semaphore      done;         ///< Posted once targetConnections have been accepted.
    size_t         targetConnections {0};

    shared_ptr<ServerConnection> createConnection(const ServerConnection::Type connectionType, const SocketType connectionSocket, const sockaddr_in* peer) override
    {
        auto connection = FastTCPServer::createConnection(connectionType, connectionSocket, peer);
        if (const auto count = accepted.fetch_add(1) + 1;
            targetConnections != 0 && count >= targetConnections)
        {
            done.post();
        }
        return connection;
    }

protected:
    void socketEventCallback(const shared_ptr<ServerConnection>& connection, const SocketEventType eventType) override
    {
        // The accept-rate test sends no data; just release connections on hangup/error
        // so the level-triggered reactor does not keep re-signaling them.
        if (eventType.m_hangup || eventType.m_error)
        {
            closeConnection(connection);
        }
    }
};

array<uint8_t, messageSize> makeMessage()
{
    array<uint8_t, messageSize> message {};
    for (size_t i = 0; i < messageSize; ++i)
    {
        message[i] = static_cast<uint8_t>('A' + (i % 26));
    }
    return message;
}

} // namespace

namespace sptk {

TEST(FastTcpServerTests, basicEcho)
{
    const auto message = makeMessage();

    EchoServer server("FastTcpServer Echo");
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", testPort));
    ASSERT_TRUE(server.active());

    TCPSocket client;
    client.open(Host("127.0.0.1", testPort));
    client.setOption(IPPROTO_TCP, TCP_NODELAY, 1);

    client.write(message.data(), message.size());

    array<uint8_t, messageSize> echoed {};
    size_t                      received = 0;
    while (received < echoed.size())
    {
        ASSERT_TRUE(client.readyToRead(2s)) << "Timed out waiting for echo";
        received += client.read(echoed.data() + received, echoed.size() - received);
    }

    EXPECT_EQ(0, memcmp(message.data(), echoed.data(), messageSize));

    client.close();
    server.stop();
}

TEST(FastTcpServerTests, multipleListeners)
{
    CountingServer server("FastTcpServer MultiListener");
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", testPort));
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", testPort + 1));

    EXPECT_EQ(2U, server.listenerHosts().size());

    // Connect to both listeners and verify each delivers data to the reactor.
    server.targetBytes = 2 * messageSize;

    const auto message = makeMessage();

    TCPSocket client1;
    client1.open(Host("127.0.0.1", testPort));
    client1.write(message.data(), message.size());

    TCPSocket client2;
    client2.open(Host("127.0.0.1", testPort + 1));
    client2.write(message.data(), message.size());

    ASSERT_TRUE(server.done.wait_for(5s)) << "Did not receive data from both listeners";
    EXPECT_GE(server.bytesReceived.load(), 2 * messageSize);

    client1.close();
    client2.close();
    server.stop();
}

TEST(FastTcpServerTests, sslEcho)
{
    const auto certFile = TestData::SslKeysDirectory() / "mycert.pem";
    if (!filesystem::exists(certFile))
    {
        GTEST_SKIP() << "Certificate file " << certFile << " does not exist.";
    }

    const auto message = makeMessage();

    EchoServer server("FastTcpServer SSL Echo");
    server.setSSLKeys(make_shared<SSLKeys>(certFile, certFile));
    server.addListener(ServerConnection::Type::SSL, Host("127.0.0.1", testPort));
    ASSERT_TRUE(server.active());

    SSLSocket client;
    client.open(Host("127.0.0.1", testPort));

    client.write(message.data(), message.size());

    array<uint8_t, messageSize> echoed {};
    size_t                      received = 0;
    while (received < echoed.size())
    {
        ASSERT_TRUE(client.readyToRead(2s)) << "Timed out waiting for SSL echo";
        received += client.read(echoed.data() + received, echoed.size() - received);
    }

    EXPECT_EQ(0, memcmp(message.data(), echoed.data(), messageSize));

    client.close();
    server.stop();
}

TEST(FastTcpServerTests, throughput)
{
    const auto message = makeMessage();

    // Number of 100-byte messages to push through the server.
    constexpr size_t messageCount = 10'000;
    constexpr size_t ackSize = 10;

    // Echo is 10 first bytes like a short ACK.
    EchoServer server("FastTcpServer Throughput", ackSize);
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", testPort));

    const auto sender = make_shared<TCPSocket>();
    sender->open(Host("127.0.0.1", testPort));
    sender->setOption(IPPROTO_TCP, TCP_NODELAY, 1);

    const auto receiver = sender;

    Stopwatch stopWatch;
    stopWatch.start();

    size_t                  receivedMessageCount = 0;
    Semaphore               receivedAllMessages;
    SocketEvents<TCPSocket> socketEvents("",
                                         [&receivedMessageCount, &receivedAllMessages](const std::shared_ptr<TCPSocket>& socket, SocketEventType eventType)
                                         {
                                             if (eventType.m_hangup)
                                             {
                                                 socket->close();
                                                 return;
                                             }
                                             if (eventType.m_data)
                                             {
                                                 array<uint8_t, ackSize> message;
                                                 if (socket->read(message.data(), ackSize) == ackSize)
                                                 {
                                                     ++receivedMessageCount;
                                                     if (receivedMessageCount == messageCount)
                                                     {
                                                         receivedAllMessages.post();
                                                     }
                                                 }
                                             }
                                         });

    socketEvents.open();
    socketEvents.add(receiver, receiver);

    // Blocking writes throttle naturally as the reactor drains the socket;
    // the server never echoes, so it always makes progress (no deadlock).
    for (size_t i = 0; i < messageCount; ++i)
    {
        sender->write(message.data(), message.size());
    }

    ASSERT_TRUE(receivedAllMessages.wait_for(1000s)) << "Throughput test receiving timed out";
    ASSERT_EQ(messageCount, receivedMessageCount) << "Received " << receivedMessageCount << " messages";

    stopWatch.stop();

    const double milliseconds = stopWatch.milliseconds();
    const double messagesPerSecond = static_cast<double>(messageCount) * 1000.0 / milliseconds;
    const double megabytesPerSecond =
        static_cast<double>(messageCount * messageSize) / 1024.0 / 1024.0 * 1000.0 / milliseconds;

    COUT("FastTcpServer received " << messageCount << " messages of " << messageSize << " bytes in "
                                   << fixed << setprecision(1) << milliseconds << " ms: "
                                   << setprecision(0) << messagesPerSecond << " msg/s ("
                                   << setprecision(1) << megabytesPerSecond << " MB/s)");

    sender->close();
    server.stop();
}

TEST(FastTcpServerTests, acceptRate)
{
    // Number of client connections to establish. Kept below the ephemeral port
    // range, since every connection consumes a distinct client-side port.
    constexpr size_t connectionCount = 20'000;
    // Multiple connector threads so the measurement reflects the server's accept
    // rate rather than a single client's serialized connect loop.
    constexpr size_t connectorThreads = 8;
    // Several listener threads (SO_REUSEPORT) so accepts are not serialized on one thread.
    constexpr uint16_t listenerThreads = 4;

    AcceptCountingServer server("FastTcpServer AcceptRate");
    server.targetConnections = connectionCount;
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", testPort), listenerThreads);

    // Each thread owns its own socket vector to avoid contention.
    vector<vector<unique_ptr<TCPSocket>>> clientsPerThread(connectorThreads);

    // Resolve the server address once; reused for every connect so the measurement
    // is not skewed by per-connection name resolution (Host() calls getaddrinfo()).
    const Host serverHost("127.0.0.1", testPort);

    Stopwatch stopWatch;
    stopWatch.start();

    {
        vector<jthread> connectors;
        connectors.reserve(connectorThreads);
        for (size_t t = 0; t < connectorThreads; ++t)
        {
            connectors.emplace_back(
                [t, &clientsPerThread, &serverHost]
                {
                    constexpr auto share = connectionCount / connectorThreads;
                    auto&          myClients = clientsPerThread[t];
                    myClients.reserve(share);
                    for (size_t i = 0; i < share; ++i)
                    {
                        auto client = make_unique<TCPSocket>();
                        client->open(serverHost);
                        myClients.push_back(std::move(client));
                    }
                });
        }
    } // join all connector threads

    ASSERT_TRUE(server.done.wait_for(30s)) << "Accept test timed out; accepted " << server.accepted.load()
                                           << " of " << connectionCount;
    stopWatch.stop();

    const double milliseconds = stopWatch.milliseconds();
    const double connectionsPerSecond = static_cast<double>(server.accepted.load()) * 1000.0 / milliseconds;

    COUT("FastTcpServer accepted " << server.accepted.load() << " connections in "
                                   << fixed << setprecision(1) << milliseconds << " ms: "
                                   << setprecision(0) << connectionsPerSecond << " conn/s");

    EXPECT_GE(server.accepted.load(), connectionCount);

    for (auto& clients: clientsPerThread)
    {
        for (auto& client: clients)
        {
            client->close();
        }
    }
    server.stop();
}

} // namespace sptk
