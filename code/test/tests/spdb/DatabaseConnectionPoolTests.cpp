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

#include "sptk5/db/DatabaseTests.h"

#include <gtest/gtest.h>
#include <queue>
#include <sptk5/cutils>
#include <sptk5/db/DatabaseConnectionPool.h>

using namespace std;
using namespace sptk;
namespace sptk {

TEST(DatabaseConnectionPoolTests,connectString)
{
    try
    {
        const DatabaseConnectionPool connectionPool("xsql://server1/db1");
        FAIL() << "MUST FAIL, incorrect server type";
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }

    try
    {
        const DatabaseConnectionPool connectionPool("mysql://server1/db1");
        COUT(connectionPool.toString(false));
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(DatabaseConnectionPoolTests,createConnections)
{
    try
    {
        constexpr size_t               maxConnections = 10;
        const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
        DatabaseConnectionPool         connectionPool(connectionString.toString(), maxConnections);

        queue<DatabaseConnection> connections;
        for (size_t i = 0; i < maxConnections; ++i)
        {
            const auto connection = connectionPool.getConnection();
            connections.push(connection);
        }

        EXPECT_EQ(maxConnections, connections.size());

        auto expectedTotalConnections = maxConnections;
        auto expectedAvailableConnections = 0;

        EXPECT_EQ(expectedTotalConnections, connectionPool.totalConnections());
        EXPECT_EQ(expectedAvailableConnections, connectionPool.availableConnections());

        while (!connections.empty())
        {
            connections.pop();
            ++expectedAvailableConnections;
            EXPECT_EQ(expectedTotalConnections, connectionPool.totalConnections());
            EXPECT_EQ(expectedAvailableConnections, connectionPool.availableConnections());
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
}

TEST(DatabaseConnectionPoolTests,reuseConnections)
{
    try
    {
        constexpr size_t               maxConnections = 10;
        const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
        DatabaseConnectionPool         connectionPool(connectionString.toString(), maxConnections);

        for (auto i = 0; i < 2; ++i)
        {
            queue<DatabaseConnection> connections;
            for (size_t j = 0; j < maxConnections; ++j)
            {
                const auto connection = connectionPool.getConnection();
                connections.push(connection);
            }

            EXPECT_EQ(maxConnections, connections.size());

            auto expectedTotalConnections = maxConnections;
            auto expectedAvailableConnections = 0;

            EXPECT_EQ(expectedTotalConnections, connectionPool.totalConnections());
            EXPECT_EQ(expectedAvailableConnections, connectionPool.availableConnections());

            while (!connections.empty())
            {
                connections.pop();
                ++expectedAvailableConnections;
                EXPECT_EQ(expectedTotalConnections, connectionPool.totalConnections());
                EXPECT_EQ(expectedAvailableConnections, connectionPool.availableConnections());
            }
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
}

TEST(DatabaseConnectionPoolTests,createConnectionsTimeout)
{
    try
    {
        constexpr size_t               maxConnections = 10;
        const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
        DatabaseConnectionPool         connectionPool(connectionString.toString(), maxConnections, 100ms);

        queue<DatabaseConnection> connections;
        for (size_t i = 0; i < maxConnections; ++i)
        {
            const auto connection = connectionPool.getConnection();
            connections.push(connection);
        }

        Stopwatch sw;
        sw.start();
        const auto connection = connectionPool.getConnection();
        sw.stop();

        EXPECT_EQ(nullptr, connection->connection().lock());
        EXPECT_GE(sw.milliseconds(), 100);
        EXPECT_LE(sw.milliseconds(), 150);
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
}

} // namespace sptk_test
