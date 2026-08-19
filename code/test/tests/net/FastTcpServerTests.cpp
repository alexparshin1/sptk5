/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <gtest/gtest.h>
#include <sptk5/cutils>
#include <sptk5/net/FastTCPServer.h>
#include <sptk5/net/SSLSocket.h>

#include "test/TestData.h"

#include <filesystem>
#include <iomanip>
#include <sptk5/threads/Semaphore.h>
#include <unordered_map>
#include <vector>

#ifndef _WIN32
#include <netinet/tcp.h>
#endif

using namespace std;
using namespace sptk;

namespace {

/// @brief Ports of this file's test servers, taken from the kernel on first use.
/// @remarks See TestData::freePort() for why hard-coded numbers are not reliable here.
/// The multi-listener test needs two ports; they are asked for separately, because a port
/// next to a free one is not itself guaranteed to be free.
uint16_t testPort()
{
    static const uint16_t port = TestData::freePort();
    return port;
}

uint16_t secondTestPort()
{
    static const uint16_t port = TestData::freePort();
    return port;
}

constexpr size_t messageSize = 100;

/**
 * @brief Test server that echoes back every message it receives.
 * @remarks The server works with messages of messageSize bytes each.
 * @remarks If ackSize is defined, it echoes back the first ackSize bytes.
 */
class EchoServer : public FastTCPServer
{
public:
    explicit EchoServer(const string& name, const size_t messageSize, const size_t ackSize)
        : FastTCPServer(name)
        , m_messageSize(messageSize)
        , m_ackSize(ackSize)
    {
    }

    ~EchoServer() override
    {
        stop();
    }

    SServerConnection createConnection(const ServerConnection::Type connectionType, const SocketType connectionSocket, const sockaddr_in* peer) override
    {
        auto              connection = FastTCPServer::createConnection(connectionType, connectionSocket, peer);
        const scoped_lock lock(m_connectionsMutex);
        m_connections.push_back(connection);
        return connection;
    }

protected:
    void socketEventCallback(const shared_ptr<ServerConnection>& connection, const SocketEventType eventType) override
    {
        if (eventType.m_hangup || eventType.m_error)
        {
            {
                const scoped_lock lock(m_pendingMutex);
                m_pending.erase(connection.get());
            }
            closeConnection(connection);
            return;
        }

        if (!eventType.m_data)
        {
            return;
        }

        const auto        socket = connection->getSocket();
        const scoped_lock lock(m_pendingMutex);
        auto&             pending = m_pending[connection.get()];

        // Socket::read() is a single recv() and may return a short read, so drain whatever the
        // socket has into a per-connection buffer and advance by the bytes actually read. Assuming
        // each read yields a whole message desynchronizes the accounting from the socket and
        // strands a partial message there, which a level-triggered poll then re-reports forever.
        auto available = socket->socketBytes();
        while (available > 0)
        {
            const auto offset = pending.size();
            pending.resize(offset + available);
            const auto bytes = socket->read(pending.data() + offset, available);
            pending.resize(offset + bytes);
            if (bytes == 0)
            {
                break; // Peer closed the connection.
            }
            available -= bytes;
        }

        // Echo only whole messages; a trailing partial one stays buffered for the next event.
        size_t processed = 0;
        while (pending.size() - processed >= m_messageSize)
        {
            const auto echoSize = m_ackSize ? min(m_ackSize, m_messageSize) : m_messageSize;
            socket->write(pending.data() + processed, echoSize);
            processed += m_messageSize;
            ++m_sentMessages;
        }
        pending.erase(pending.begin(), pending.begin() + static_cast<ptrdiff_t>(processed));
    }

private:
    size_t                    m_messageSize {0};
    size_t                    m_ackSize {0};
    size_t                    m_sentMessages {0};
    mutex                     m_connectionsMutex;
    vector<SServerConnection> m_connections;
    mutex                     m_pendingMutex; ///< Protects m_pending.
    std::unordered_map<ServerConnection*, vector<uint8_t>>
        m_pending; ///< Bytes received but not yet forming a whole message, per connection.
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

    EchoServer server("FastTcpServer Echo", 100, 100);
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", testPort()));
    ASSERT_TRUE(server.active());

    TCPSocket client;
    client.open(Host("127.0.0.1", testPort()));
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
    EchoServer server("FastTcpServer MultiListener", 100, 100);
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", testPort()));
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", secondTestPort()));

    EXPECT_EQ(2U, server.listenerHosts().size());

    const auto message = makeMessage();

    TCPSocket client1;
    client1.open(Host("127.0.0.1", testPort()));
    client1.write(message.data(), message.size());

    TCPSocket client2;
    client2.open(Host("127.0.0.1", secondTestPort()));
    client2.write(message.data(), message.size());

    Buffer buffer;
    auto   echoBytes = client1.read(buffer, messageSize);
    EXPECT_EQ(messageSize, echoBytes);

    echoBytes = client2.read(buffer, messageSize);
    EXPECT_EQ(messageSize, echoBytes);

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

    EchoServer server("FastTcpServer SSL Echo", 100, 100);
    server.setSSLKeys(make_shared<SSLKeys>(certFile, certFile));
    server.addListener(ServerConnection::Type::SSL, Host("127.0.0.1", testPort()));
    ASSERT_TRUE(server.active());

    SSLSocket client;
    client.open(Host("127.0.0.1", testPort()));

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
    constexpr size_t messageCount = 1000;
    constexpr size_t ackSize = 10;

    // Echo is 10 first bytes like a short ACK.
    EchoServer server("FastTcpServer Throughput", 100, ackSize);
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", testPort()));

    const auto sender = make_shared<TCPSocket>();
    sender->open(Host("127.0.0.1", testPort()));
    sender->setOption(IPPROTO_TCP, TCP_NODELAY, 1);

    const auto& receiver = sender;

    Stopwatch stopWatch;
    stopWatch.start();

    size_t               receivedMessageCount = 0;
    Semaphore            receivedAllMessages;
    SocketEvents<Socket> socketEvents("",
                                      [&receivedMessageCount, &receivedAllMessages](const std::weak_ptr<Socket>& socketPtr, const SocketEventType eventType)
                                      {
                                          const auto socket = socketPtr.lock();
                                          if (eventType.m_hangup)
                                          {
                                              socket->close();
                                              return;
                                          }
                                          if (eventType.m_data)
                                          {
                                              array<uint8_t, ackSize> ack;
                                              if (socket->socketBytes() >= ackSize)
                                              {
                                                  if (socket->read(ack.data(), ackSize) == ackSize)
                                                  {
                                                      ++receivedMessageCount;
                                                      if (receivedMessageCount == messageCount)
                                                      {
                                                          receivedAllMessages.post();
                                                      }
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

    const auto milliseconds = stopWatch.milliseconds();
    const auto messagesPerSecond = static_cast<double>(messageCount) * 1000.0 / milliseconds;
    const auto megabytesPerSecond = static_cast<double>(messageCount * messageSize) / 1024.0 / 1024.0 * 1000.0 / milliseconds;

    COUT("FastTcpServer received " << messageCount << " messages of " << messageSize << " bytes in "
                                   << fixed << setprecision(1) << milliseconds << " ms: "
                                   << setprecision(0) << messagesPerSecond << " msg/s ("
                                   << setprecision(1) << megabytesPerSecond << " MB/s)");

    sender->close();
    server.stop();
}

/**
 * @brief Test how fast FastTcpServerTests can accept connections.
 *
 * On Linux, before running the test, make sure you have a lot of ephemeral ports (~40000).
 *    cat /proc/sys/net/ipv4/ip_local_port_range
 *
 * Temporary setting (until reboot) the good number of ephemeral points:
 *    sudo echo "1024 65535" > /proc/sys/net/ipv4/ip_local_port_range
 */
TEST(FastTcpServerTests, acceptRate)
{
    constexpr size_t   connectionCount = 1000;
    constexpr size_t   connectorThreads = 8;
    constexpr uint16_t listenerThreads = 4;

    EchoServer server("FastTcpServer AcceptRate", 100, 100);
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", testPort()), listenerThreads);

    // Each thread owns its own socket vector to avoid contention.
    vector<vector<unique_ptr<TCPSocket>>> clientsPerThread(connectorThreads);

    // Resolve the server address once; reused for every connect so the measurement
    // is not skewed by per-connection name resolution (Host() calls getaddrinfo()).
    const Host serverHost("127.0.0.1", testPort());

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

    stopWatch.stop();

    const auto milliseconds = stopWatch.milliseconds();
    const auto connectionsPerSecond = static_cast<double>(connectionCount) * 1000.0 / milliseconds;

    COUT("FastTcpServer accepted " << connectionCount << " connections in "
                                   << fixed << setprecision(1) << milliseconds << " ms: "
                                   << setprecision(0) << connectionsPerSecond << " conn/s");

    for (const auto& clients: clientsPerThread)
    {
        for (const auto& client: clients)
        {
            client->close();
        }
    }
    server.stop();
}

/**
 * @brief Test how fast FastTcpServerTests can accept connections.
 *
 * On Linux, before running the test, make sure you have a lot of ephemeral ports (~40000).
 *    cat /proc/sys/net/ipv4/ip_local_port_range
 *
 * To temporary set (until reboot) the good number of ephemeral points:
 *    sudo echo "1024 65535" > /proc/sys/net/ipv4/ip_local_port_range
 */
TEST(FastTcpServerTests, latency)
{
    constexpr size_t   connectionCount = 1000;
    constexpr size_t   connectorThreads = 4;
    constexpr uint16_t listenerThreads = 4;

    EchoServer server("FastTcpServer AcceptRate", 5, 5);
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", testPort()), listenerThreads);

    // Each thread owns its own socket vector to avoid contention.
    vector<vector<unique_ptr<TCPSocket>>> clientsPerThread(connectorThreads);

    const Host serverHost("127.0.0.1", testPort());

    Stopwatch stopWatch;
    stopWatch.start();

    atomic_size_t totalLatencyMcs = 0;
    {
        vector<jthread> connectors;
        connectors.reserve(connectorThreads);
        for (size_t t = 0; t < connectorThreads; ++t)
        {
            connectors.emplace_back(
                [t, &clientsPerThread, &serverHost, &totalLatencyMcs]
                {
                    vector<uint8_t> message(128);
                    constexpr auto  share = connectionCount / connectorThreads;
                    auto&           myClients = clientsPerThread[t];
                    myClients.reserve(share);
                    for (size_t i = 0; i < share; ++i)
                    {
                        auto client = make_unique<TCPSocket>();
                        auto started = chrono::high_resolution_clock::now();
                        client->open(serverHost);
                        client->write("Hello");
                        if (client->readyToRead(1s))
                        {
                            client->read(message.data(), 5);
                        }
                        auto       completed = chrono::high_resolution_clock::now();
                        const auto latencyMcs = duration_cast<chrono::microseconds>(completed - started).count();
                        totalLatencyMcs += latencyMcs;
                        myClients.push_back(std::move(client));
                    }
                });
        }
    } // join all connector threads

    COUT("Average latency is " << fixed << setprecision(1) << totalLatencyMcs.load() / connectionCount << " mcs");

    stopWatch.stop();

    const auto milliseconds = stopWatch.milliseconds();
    const auto connectionsPerSecond = static_cast<double>(connectionCount) * 1000.0 / milliseconds;

    COUT("FastTcpServer accepted " << connectionCount << " connections in "
                                   << fixed << setprecision(1) << milliseconds << " ms: "
                                   << setprecision(0) << connectionsPerSecond << " conn/s");

    for (const auto& clients: clientsPerThread)
    {
        for (const auto& client: clients)
        {
            client->close();
        }
    }
    server.stop();
}

} // namespace sptk
