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

#include "sptk5/db/DatabaseConnectionPool.h"
#include "sptk5/db/DatabaseTests.h"

#include <gtest/gtest.h>
#include <sptk5/cutils>
#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;
namespace sptk {

TEST(BulkInsertTests,escapeSqlString)
{
    const String sourceString = "Hello, 'World'.\n\rLet's go\n";
    const String escapedString = escapeSQLString(sourceString, false);
    EXPECT_STREQ("Hello, ''World''.\\n\\rLet''s go\\n", escapedString.c_str());
}

TEST(BulkInsertTests,escapeSqlStringPerformance)
{
    constexpr auto maxCount = 100000;
    constexpr auto mcsInSecond = 1E6;
    const String   sourceString = "Hello, 'World'.\n\rLet's go\n";
    Stopwatch      stopWatch;
    stopWatch.start();
    for (size_t i = 0; i < maxCount; ++i)
    {
        escapeSQLString(sourceString, false);
    }
    stopWatch.stop();
    COUT("Escaped " << maxCount << " SQLs "
                    << " for " << stopWatch.seconds() << " sec, "
                    << fixed << setprecision(2) << maxCount / stopWatch.seconds() / mcsInSecond << "M op/sec" << endl);
}

TEST(PoolDatabaseConnectionTests,handleBulkInsertFailures)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
    if (connectionString.empty())
    {
        GTEST_SKIP() << "postgresql connection is not defined";
    }

    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    try
    {
        databaseConnection->open();
    }
    catch (const Exception& e)
    {
        GTEST_SKIP() << "postgresql connection is not available: " << e.what();
    }

    auto dropTestTable = [&databaseConnection]()
    {
        try
        {
            Query dropTable(databaseConnection, "DROP TABLE gtest_temp_table");
            dropTable.exec();
        }
        catch (const Exception&)
        {
        }
    };

    dropTestTable();

    Query createTable(databaseConnection, "CREATE TABLE gtest_temp_table(id SERIAL PRIMARY KEY, name CHAR(40) NULL, position_name CHAR(20) NULL, hire_date CHAR(12) NULL)");
    ASSERT_NO_THROW(createTable.exec());

    vector<VariantVector> data;
    data.push_back(VariantVector({"Alex", "Programmer", "01-JAN-2014"}));
    data.push_back(VariantVector({"David", "CEO", "01-JAN-2015", "EXTRA_COLUMN_TO_FAIL"}));
    const Strings   columnNames({"name", "position_name", "hire_date"});
    vector<int64_t> insertedIds;

    EXPECT_THROW(databaseConnection->bulkInsert("gtest_temp_table", "id", columnNames, data, insertedIds, 100), Exception);

    const auto pooledConnection = databaseConnection->connection().lock();
    ASSERT_TRUE(pooledConnection != nullptr);
    EXPECT_FALSE(pooledConnection->inTransaction());

    Query countRows(databaseConnection, "SELECT COUNT(*) FROM gtest_temp_table");
    EXPECT_EQ(countRows.scalar().asInteger(), 0);

    dropTestTable();
}

TEST(PoolDatabaseConnectionTests,handleBulkDeleteFailures)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
    if (connectionString.empty())
    {
        GTEST_SKIP() << "postgresql connection is not defined";
    }

    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    try
    {
        databaseConnection->open();
    }
    catch (const Exception& e)
    {
        GTEST_SKIP() << "postgresql connection is not available: " << e.what();
    }

    auto dropTestTable = [&databaseConnection]()
    {
        try
        {
            Query dropTable(databaseConnection, "DROP TABLE gtest_temp_table");
            dropTable.exec();
        }
        catch (const Exception&)
        {
        }
    };

    dropTestTable();

    Query createTable(databaseConnection, "CREATE TABLE gtest_temp_table(id SERIAL PRIMARY KEY, name CHAR(40) NULL)");
    ASSERT_NO_THROW(createTable.exec());

    Query insertRows(databaseConnection, "INSERT INTO gtest_temp_table(name) VALUES('Alex'),('David')");
    ASSERT_NO_THROW(insertRows.exec());

    VariantVector keys;
    keys.emplace_back(1);
    keys.emplace_back(2);

    EXPECT_THROW(databaseConnection->bulkDelete("gtest_temp_table", "missing_key", keys), Exception);

    const auto pooledConnection = databaseConnection->connection().lock();
    ASSERT_TRUE(pooledConnection != nullptr);
    EXPECT_FALSE(pooledConnection->inTransaction());

    Query countRows(databaseConnection, "SELECT COUNT(*) FROM gtest_temp_table");
    EXPECT_EQ(countRows.scalar().asInteger(), 2);

    dropTestTable();
}

} // namespace sptk_test
