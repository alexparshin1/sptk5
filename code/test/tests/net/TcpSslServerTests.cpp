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

#include <sptk5/cutils>
#include <sptk5/net/FastTCPServer.h>
#include <sptk5/net/SSLServerConnection.h>
#include <sptk5/net/SocketReader.h>

#include "test/TestData.h"

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

namespace {

/// @brief Ports of this file's echo server, taken from the kernel on first use.
/// @remarks See TestData::freePort() for why hard-coded numbers are not reliable here.
uint16_t testTcpEchoServerPort()
{
    static const uint16_t port = TestData::freePort();
    return port;
}

uint16_t testSslEchoServerPort()
{
    static const uint16_t port = TestData::freePort();
    return port;
}

/**
 * @brief Connection that carries a per-connection SocketReader for the event-driven echo server.
 *
 * The reactor delivers events level-triggered, so the reader must persist between events: bytes it
 * buffers (a partially received line) have already left the kernel socket and would be lost if a
 * fresh reader were created for the next event.
 */
class FastEchoConnection final : public ServerConnection
{
public:
    FastEchoConnection(const Type type, const sockaddr_in* peer)
        : ServerConnection(type, peer)
    {
    }

    std::shared_ptr<SocketReader> reader;
};

/**
 * @brief Event-driven, line-based echo server built on FastTCPServer.
 *
 * Echoes every received line back to the client with a trailing newline.
 */
class FastEchoServer final : public FastTCPServer
{
public:
    explicit FastEchoServer(const string& name)
        : FastTCPServer(name)
    {
    }

    ~FastEchoServer() override
    {
        stop();
    }

    SServerConnection createConnection(const ServerConnection::Type connectionType, const SocketType connectionSocket,
                                       const sockaddr_in* peer) override
    {
        const auto socket = createConnectionSocket(connectionType, connectionSocket);

        auto connection = make_shared<FastEchoConnection>(connectionType, peer);
        connection->setSocket(socket);
        connection->reader = make_shared<SocketReader>(socket);

        return connection;
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

        const auto echoConnection = dynamic_pointer_cast<FastEchoConnection>(connection);
        if (!echoConnection)
        {
            return;
        }

        const auto& reader = echoConnection->reader;
        Buffer      data;
        try
        {
            // Drain every complete line currently available: the reactor will not re-signal for
            // lines already buffered out of the kernel socket. A partial line throws here (no data
            // yet) and is left buffered for the next event.
            while (connection->getSocket()->active() && reader->readyToRead(chrono::milliseconds(0)))
            {
                if (reader->readLine(data) == 0)
                {
                    break;
                }
                string str(data.c_str());
                str += "\n";
                connection->getSocket()->write(str);
            }
        }
        catch (const Exception&)
        {
            // No more data ready, or the client went away mid-line.
        }

        if (!connection->getSocket()->active())
        {
            closeConnection(connection);
        }
    }
};

} // namespace

TEST(FastTcpServerTests, sslMinimal)
{
    Buffer buffer;

    try
    {
        FastEchoServer echoServer("TestServer");

        const auto keysDirectory = TestData::SslKeysDirectory();
        const auto keys = make_shared<SSLKeys>(keysDirectory / "mycert.pem", keysDirectory / "mycert.pem");
        if (!filesystem::exists(keys->certificateFileName()))
        {
            GTEST_SKIP() << "Certificate file " << keys->certificateFileName() << " does not exist.";
        }

        echoServer.setSSLKeys(keys);

        echoServer.addListener(ServerConnection::Type::TCP, {"localhost", testTcpEchoServerPort()});
        echoServer.addListener(ServerConnection::Type::SSL, {"localhost", testSslEchoServerPort()});
        this_thread::sleep_for(100ms);

        auto         socket = make_shared<SSLSocket>();
        SocketReader socketReader(socket);

        try
        {
            socket->open({"localhost", testSslEchoServerPort()});
        }
        catch (Exception& e)
        {
            FAIL() << e.what();
        }

        const Strings rows({"Hello, World!",
                            "This is a test of TCPServer class.",
                            "Using simple echo server to verify data flow.",
                            "The session is terminated when this row is received"});

        int rowCount = 0;
        for (const auto& row: rows)
        {
            socket->write(row + "\n");
            buffer.bytes(0);
            if (socketReader.readyToRead(3s))
            {
                socketReader.readLine(buffer);
            }
            EXPECT_STREQ(row.c_str(), buffer.c_str());
            ++rowCount;
        }
        EXPECT_EQ(4, rowCount);

        socket->close();

        echoServer.stop();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}
