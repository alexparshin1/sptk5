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

#include "sptk5/StreamLogEngine.h"
#include "sptk5/wsdl/WSConnection.h"
#include <gtest/gtest.h>
#include <sptk5/net/FastTCPServer.h>

using namespace sptk;

namespace {
// Minimal concrete FastTCPServer used only to satisfy the WSConnection constructor.
class StubServer : public FastTCPServer
{
public:
    using FastTCPServer::FastTCPServer;

protected:
    void socketEventCallback(const std::shared_ptr<ServerConnection>&, SocketEventType) override
    {
        // Don't need to use events in that test class.
    }
};
} // namespace

// Test fixture that constructs a WSConnection using minimal stubs.
class WSConnectionTests : public ::testing::Test
{
public:
    WSConnectionTests() = default;

    StubServer              tcpServer {"test"};
    std::stringstream       logStream;
    sockaddr_in             addr {};
    WSServices              services {nullptr}; // requires default-constructible - replace with a test double if needed
    StreamLogEngine         logEngine {logStream};
    WSConnection::Options   options {{".", ".", "."}, false};
    std::shared_ptr<Thread> workerThread {};

    // Create WSConnection under test. If your real constructor requires more setup,
    // adapt this fixture to provide appropriate test doubles.
    std::unique_ptr<WSConnection> makeConnection()
    {
        return std::make_unique<WSConnection>(tcpServer, &addr, services, logEngine, options, workerThread);
    }
};

namespace sptk {

TEST_F(WSConnectionTests, addsContentLengthForGetWhenMissing)
{
    // Arrange
    const std::unique_ptr<WSConnection> conn(makeConnection());
    HttpHeaders                         headers;

    // Simulate GET with no Content-Length and no Connection header
    const String requestType = "GET";

    // Act
    const bool closeConnection = WSConnection::reviewHeaders(requestType, headers);

    // Assert
    const auto it = headers.find("Content-Length");
    ASSERT_NE(it, headers.end());
    EXPECT_EQ(it->second, "0");
    // No Connection header at all: HTTP/1.1 says the connection is persistent, so there is
    // nothing to close. This used to expect true, which is how the caller came to close every
    // connection a client wanted to keep.
    EXPECT_FALSE(closeConnection);
}

TEST_F(WSConnectionTests, closesConnectionWhenHeaderIsClose)
{
    // Arrange
    const std::unique_ptr<WSConnection> conn(makeConnection());
    HttpHeaders                         headers;
    headers["Content-Length"] = "123";
    headers["Connection"] = "close";

    const String requestType = "POST";

    // Act
    const bool closeConnection = WSConnection::reviewHeaders(requestType, headers);

    // Assert
    // Content-Length should remain unchanged
    const auto itLen = headers.find("Content-Length");
    ASSERT_NE(itLen, headers.end());
    EXPECT_EQ(itLen->second, "123");

    // The client asked for the connection to be closed, so that is what is reported. The header
    // is left in place, since it is the reason.
    EXPECT_TRUE(closeConnection);
    const auto itConn = headers.find("Connection");
    ASSERT_NE(itConn, headers.end());
    EXPECT_EQ(itConn->second.toLowerCase(), "close");
}

TEST_F(WSConnectionTests, erasesConnectionWhenNotClose)
{
    // Arrange
    const std::unique_ptr<WSConnection> conn(makeConnection());
    HttpHeaders                         headers;
    headers["Connection"] = "keep-alive";

    const String requestType = "POST";

    // Act
    const bool closeConnection = WSConnection::reviewHeaders(requestType, headers);

    // Assert
    // "keep-alive" is a request to keep the connection, and the hop-by-hop header is consumed here.
    EXPECT_FALSE(closeConnection);
    const auto itConn = headers.find("Connection");
    EXPECT_EQ(itConn, headers.end());
}

TEST_F(WSConnectionTests, closesConnectionWhateverTheCaseOfClose)
{
    const std::unique_ptr<WSConnection> conn(makeConnection());
    HttpHeaders                         headers;
    headers["Connection"] = "Close";

    // Header values are case-insensitive, and this spelling used to take the other branch.
    EXPECT_TRUE(WSConnection::reviewHeaders("POST", headers));
}

} // namespace sptk
