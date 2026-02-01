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

#include "sptk5/StreamLogEngine.h"
#include "sptk5/wsdl/WSConnection.h"
#include <gtest/gtest.h>

using namespace sptk;

// Test fixture that constructs a WSConnection using minimal stubs.
// If your WSConnection constructor requires concrete server / services / logger types,
// replace the dummy values below with lightweight test doubles from your test helpers.
class WSConnectionTest : public ::testing::Test
{
public:
    WSConnectionTest() = default;

protected:
    TCPServer               tcpServer {"test"};
    std::stringstream       logStream;
    sockaddr_in             addr {};
    WSServices              services {nullptr}; // requires default-constructible - replace with a test double if needed
    StreamLogEngine         logEngine {logStream};
    WSConnection::Options   options {{".", ".", "."}, false};
    std::shared_ptr<Thread> workerThread {};

    // Create WSConnection under test. If your real constructor requires more setup,
    // adapt this fixture to provide appropriate test doubles.
    WSConnection* makeConnection()
    {
        return new WSConnection(tcpServer, &addr, services, logEngine, options, workerThread);
    }

    void TearDown() override
    {
        // Nothing by default
    }
};

TEST_F(WSConnectionTest, AddsContentLengthForGetWhenMissing)
{
    // Arrange
    std::unique_ptr<WSConnection> conn(makeConnection());
    HttpHeaders                   headers;

    // Simulate GET with no Content-Length and no Connection header
    const String requestType = "GET";

    // Act
    bool closeConnection = conn->reviewHeaders(requestType, headers);

    // Assert
    auto it = headers.find("Content-Length");
    ASSERT_NE(it, headers.end());
    EXPECT_EQ(it->second, "0");
    // When Connection header absent, implementation returns true (not "close")
    EXPECT_TRUE(closeConnection);
}

TEST_F(WSConnectionTest, KeepsConnectionWhenHeaderIsClose)
{
    // Arrange
    std::unique_ptr<WSConnection> conn(makeConnection());
    HttpHeaders                   headers;
    headers["Content-Length"] = "123";
    headers["Connection"] = "close";

    const String requestType = "POST";

    // Act
    bool closeConnection = conn->reviewHeaders(requestType, headers);

    // Assert
    // Content-Length should remain unchanged
    auto itLen = headers.find("Content-Length");
    ASSERT_NE(itLen, headers.end());
    EXPECT_EQ(itLen->second, "123");

    // Since Connection == "close", method returns false and does not erase the header
    EXPECT_FALSE(closeConnection);
    auto itConn = headers.find("Connection");
    ASSERT_NE(itConn, headers.end());
    EXPECT_EQ(itConn->second.toLowerCase(), "close");
}

TEST_F(WSConnectionTest, ErasesConnectionWhenNotClose)
{
    // Arrange
    std::unique_ptr<WSConnection> conn(makeConnection());
    HttpHeaders                   headers;
    headers["Connection"] = "keep-alive";

    const String requestType = "POST";

    // Act
    bool closeConnection = conn->reviewHeaders(requestType, headers);

    // Assert
    // Implementation treats anything not equal to "close" as closeConnection == true and erases header
    EXPECT_TRUE(closeConnection);
    auto itConn = headers.find("Connection");
    EXPECT_EQ(itConn, headers.end());
}
