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

#include <gtest/gtest.h>
#include <sptk5/Base64.h>
#include <sptk5/CommandLine.h>
#include <sptk5/Crypt.h>
#include <sptk5/DirectoryDS.h>
#include <sptk5/JWT.h>
#include <sptk5/Tar.h>
#include <sptk5/db/DatabaseConnectionPool.h>
#include <sptk5/db/DatabaseTests.h>
#include <sptk5/net/HttpConnect.h>
#include <sptk5/net/SSLSocket.h>
#include <sptk5/net/ServerConnection.h>
#include <sptk5/net/TCPServer.h>
#include <sptk5/test/TestRunner.h>
#include <sptk5/threads/Timer.h>
#include <sptk5/wsdl/WSComplexType.h>

#ifdef BUILD_TEST_WS

#include <test/wsdl/TestWebService.h>

#endif

using namespace std;
using namespace sptk;

/**
 * Stub TCP server - testing only
 */
class StubServer
    : public TCPServer
{
public:
    StubServer()
        : TCPServer("test")
    {
    }

protected:
    UServerConnection createConnection(ServerConnection::Type connectionType, SocketType, const sockaddr_in*) override
    {
        return nullptr;
    }
};

// Hints to linker that we need other modules.
// Otherwise, Visual Studio doesn't include any tests
void stub()
{
    const DateTime dateTime;
    const JWT jwt;
    const RegularExpression regexp(".*");
    const CommandLine cmd("", "", "");
    const DirectoryDS dir("");
    const ThreadPool threads(1, std::chrono::milliseconds(), "test", nullptr);
    const Timer timer;
    const MD5 md5;
    const StubServer tcpServer;
    const Tar tar;
    const FieldList fieldList(false);
    const Variant variant;

    SSLSocket socket;
    const HttpConnect connect(socket);

    const string text("The quick brown fox jumps over the lazy dog.ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    const string key("01234567890123456789012345678901");
    const string iv("0123456789012345");

    const Buffer intext(text);
    Buffer outtext;
    Crypt::encrypt(outtext, intext, key, iv);

    Buffer buffer1;
    const Buffer buffer2("xxx");
    Base64::encode(buffer1, buffer2);

    const DatabaseConnectionPool connectionPool("");

#ifdef BUILD_TEST_WS
    const TestWebService setvice;
#endif
}

TestRunner::TestRunner(int& argc, char**& argv)
    : m_argc(argc)
    , m_argv(argv)
{
}

void TestRunner::addDatabaseConnection(const DatabaseConnectionString& connectionString) const
{
    DatabaseTests::tests().addDatabaseConnection(connectionString);
}

static String excludeDatabasePatterns(const std::vector<DatabaseConnectionString>& definedConnections)
{
    map<String, String> excludeDrivers = {
        {"postgresql", "PostgreSQL"},
        {"mysql", "MySQL"},
        {"mssql", "MSSQL"},
        {"oracle", "Oracle"},
        {"sqlite3", "SQLite3"}};

    for (const auto& connection: definedConnections)
    {
        excludeDrivers.erase(connection.driverName());
    }

    Strings excludePatterns;
    for (const auto& [protocol, serverName]: excludeDrivers)
    {
        excludePatterns.push_back("SPTK_" + serverName + "*.*");
    }

    return excludePatterns.join(":");
}

int TestRunner::runAllTests()
{
#ifdef _WIN32
    // Make sure Winsock is initialized
    TCPSocket socket;
#endif

    const String excludeDBDriverPatterns = excludeDatabasePatterns(DatabaseTests::tests().connectionStrings());

    size_t filterArgumentIndex = 0;
    for (int i = 1; i < m_argc; ++i)
    {
        if (strstr(m_argv[i], "--gtest_filter="))
        {
            filterArgumentIndex = i;
            break;
        }
    }

    vector<char*> argv(m_argv, m_argv + m_argc);
    String filter;

    if (!excludeDBDriverPatterns.empty())
    {
        if (filterArgumentIndex != 0)
        {
            filter = argv[filterArgumentIndex];
        }

        if (filter.empty())
        {
            filter = "-" + excludeDBDriverPatterns;
        }
        else
        {
            filter += ":-" + excludeDBDriverPatterns;
        }

        if (filterArgumentIndex == 0)
        {
            argv.push_back(filter.data());
            ++m_argc;
        }
        else
        {
            argv[filterArgumentIndex] = filter.data();
        }
    }

    ::testing::InitGoogleTest(&m_argc, argv.data());

    return RUN_ALL_TESTS();
}
