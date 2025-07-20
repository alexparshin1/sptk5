/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/db/DatabaseConnectionPool.h>

#include <future>
#include <gtest/gtest.h>
#include <sptk5/db/DatabaseTests.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

TEST(SPTK_DatabaseConnectionPool, connectString)
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

namespace {

void testConnect(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());

    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        DatabaseTests::testConnect(connectionString);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }

    const auto invalidConnectionStringStr = DatabaseConnectionString(dbName.toLowerCase() + "://localhost:1234/xyz");
    EXPECT_THROW(DatabaseTests::testConnect(invalidConnectionStringStr), DatabaseException);
}

void testDDL(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());

    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        DatabaseTests::testDDL(connectionString);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
}

void verifyInvalidKeywordQueryThrows(const DatabaseConnection& databaseConnection);
void verifyInvalidTableQueryThrows(const DatabaseConnection& databaseConnection);
void testInvalidQuery(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());
    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();

    verifyInvalidTableQueryThrows(databaseConnection);
    verifyInvalidKeywordQueryThrows(databaseConnection);
}

void verifyInvalidTableQueryThrows(const DatabaseConnection& databaseConnection)
{
    Query query(databaseConnection, "SELECT * FROM xx");
    EXPECT_THROW(query.exec(), DatabaseException);
}

void verifyInvalidKeywordQueryThrows(const DatabaseConnection& databaseConnection)
{
    Query query(databaseConnection, "UNSELECT * FROM xx");
    EXPECT_THROW(query.exec(), DatabaseException);
}

void testInsertQuery(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());
    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        DatabaseTests::testInsertQuery(connectionString);
        DatabaseTests::testInsertQueryDirect(connectionString);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
}

void testBlobInsertAndSelect(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());
    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }
    try
    {
        DatabaseTests::testBLOB(connectionString);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
}

void testBulkInsert(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());
    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        DatabaseTests::testBulkInsert(connectionString);
        DatabaseTests::testBatchSQL(connectionString);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
}

void testBulkInsertPerformance(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());
    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        constexpr auto recordCount = 1024;
        DatabaseTests::testBulkInsertPerformance(connectionString, recordCount);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
}

void testQueryParameters(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());
    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        DatabaseTests::testQueryParameters(connectionString);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
}

void testQueryDateAndTimestamp(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());
    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        DatabaseTests::testQueryInsertDate(connectionString);
        DatabaseTests::testQueryInsertDateTime(connectionString);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
}

void testTransaction(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());
    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        DatabaseTests::testTransaction(connectionString);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
}

void testSelect(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());
    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        DatabaseTests::testSelect(connectionString);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
}

} // namespace

//───────────────────────────────── PostgreSQL ───────────────────────────────────────────
#ifdef HAVE_POSTGRESQL

TEST(SPTK_PostgreSQLConnection, connect)
{
    testConnect("PostgreSQL");
}

TEST(SPTK_PostgreSQLConnection, DDL)
{
    testDDL("PostgreSQL");
}

TEST(SPTK_PostgreSQLConnection, bulkInsert)
{
    testBulkInsert("PostgreSQL");
}

TEST(SPTK_PostgreSQLConnection, bulkInsertPerformance)
{
    testBulkInsertPerformance("PostgreSQL");
}

TEST(SPTK_PostgreSQLConnection, queryParameters)
{
    testQueryParameters("PostgreSQL");
}

TEST(SPTK_PostgreSQLConnection, dates)
{
    testQueryDateAndTimestamp("PostgreSQL");
}

TEST(SPTK_PostgreSQLConnection, transaction)
{
    testTransaction("PostgreSQL");
}

TEST(SPTK_PostgreSQLConnection, select)
{
    testSelect("PostgreSQL");
    testInvalidQuery("PostgreSQL");
}

TEST(SPTK_PostgreSQLConnection, insertQuery)
{
    testInsertQuery("PostgreSQL");
}

TEST(SPTK_PostgreSQLConnection, BLOB)
{
    testBlobInsertAndSelect("PostgreSQL");
}

#endif

//───────────────────────────────── MySQL ────────────────────────────────────────────────

#ifdef HAVE_MYSQL

TEST(SPTK_MySQLConnection, connect)
{
    testConnect("MySQL");
}

TEST(SPTK_MySQLConnection, DDL)
{
    testDDL("MySQL");
}

TEST(SPTK_MySQLConnection, bulkInsert)
{
    testBulkInsert("MySQL");
}

TEST(SPTK_MySQLConnection, bulkInsertPerformance)
{
    testBulkInsertPerformance("MySQL");
}

TEST(SPTK_MySQLConnection, queryParameters)
{
    testQueryParameters("MySQL");
}

TEST(SPTK_MySQLConnection, dates)
{
    testQueryDateAndTimestamp("MySQL");
}

TEST(SPTK_MySQLConnection, transaction)
{
    testTransaction("MySQL");
}

TEST(SPTK_MySQLConnection, select)
{
    testSelect("MySQL");
    testInvalidQuery("MySQL");
}

TEST(SPTK_MySQLConnection, insertQuery)
{
    testInsertQuery("MySQL");
}

TEST(SPTK_MySQLConnection, BLOB)
{
    testBlobInsertAndSelect("MySQL");
}

#endif

//───────────────────────────────── Oracle ─────────────────────────────────────────────
#ifdef HAVE_ORACLE

TEST(SPTK_OracleConnection, connect)
{
    testConnect("Oracle");
}

TEST(SPTK_OracleConnection, DDL)
{
    testDDL("Oracle");
}

TEST(SPTK_OracleConnection, bulkInsert)
{
    testBulkInsert("Oracle");
}

TEST(SPTK_OracleConnection, bulkInsertPerformance)
{
    testBulkInsertPerformance("Oracle");
}

TEST(SPTK_OracleConnection, queryParameters)
{
    testQueryParameters("Oracle");
}

TEST(SPTK_OracleConnection, dates)
{
    testQueryDateAndTimestamp("Oracle");
}

TEST(SPTK_OracleConnection, transaction)
{
    testTransaction("Oracle");
}

TEST(SPTK_OracleConnection, select)
{
    testSelect("Oracle");
    testInvalidQuery("Oracle");
}

TEST(SPTK_OracleConnection, insertQuery)
{
    testInsertQuery("Oracle");
}

TEST(SPTK_OracleConnection, BLOB)
{
    testBlobInsertAndSelect("Oracle");
}

#endif

//───────────────────────────────── Oracle OCILib ──────────────────────────────────────
#ifdef HAVE_ORACLE_OCI

TEST(SPTK_OracleOciConnection, connect)
{
    testConnect("Oracle");
}

TEST(SPTK_OracleOciConnection, DDL)
{
    testDDL("Oracle");
}

TEST(SPTK_OracleOciConnection, bulkInsert)
{
    testBulkInsert("Oracle");
}

TEST(SPTK_OracleOciConnection, bulkInsertPerformance)
{
    testBulkInsertPerformance("Oracle");
}

TEST(SPTK_OracleOciConnection, queryParameters)
{
    testQueryParameters("Oracle");
}

TEST(SPTK_OracleOciConnection, dates)
{
    testQueryDateAndTimestamp("Oracle");
}

TEST(SPTK_OracleOciConnection, transaction)
{
    testTransaction("Oracle");
}

TEST(SPTK_OracleOciConnection, select)
{
    testSelect("Oracle");
    testInvalidQuery("Oracle");
}

TEST(SPTK_OracleOciConnection, insertQuery)
{
    testInsertQuery("Oracle");
}

TEST(SPTK_OracleOciConnection, BLOB)
{
    testBlobInsertAndSelect("Oracle");
}

#endif

//───────────────────────────────── MS SQL ─────────────────────────────────────────────
#ifdef HAVE_ODBC

TEST(SPTK_MSSQLConnection, connect)
{
    testConnect("MSSQL");
}

TEST(SPTK_MSSQLConnection, DDL)
{
    testDDL("MSSQL");
}

TEST(SPTK_MSSQLConnection, bulkInsert)
{
    testBulkInsert("MSSQL");
}

TEST(SPTK_MSSQLConnection, bulkInsertPerformance)
{
    testBulkInsertPerformance("MSSQL");
}

TEST(SPTK_MSSQLConnection, queryParameters)
{
    testQueryParameters("MSSQL");
}

TEST(SPTK_MSSQLConnection, dates)
{
    testQueryDateAndTimestamp("MSSQL");
}

TEST(SPTK_MSSQLConnection, transaction)
{
    testTransaction("MSSQL");
}

TEST(SPTK_MSSQLConnection, select)
{
    testSelect("MSSQL");
    testInvalidQuery("MSSQL");
}

TEST(SPTK_MSSQLConnection, insertQuery)
{
    testInsertQuery("MSSQL");
}

TEST(SPTK_MSSQLConnection, BLOB)
{
    testBlobInsertAndSelect("MSSQL");
}

#endif

//───────────────────────────────── SQLite3 ────────────────────────────────────────────
#ifdef HAVE_SQLITE3

TEST(SPTK_SQLite3Connection, connect)
{
    testConnect("SQLite3");
}

TEST(SPTK_SQLite3Connection, DDL)
{
    testDDL("SQLite3");
}

TEST(SPTK_SQLite3Connection, bulkInsert)
{
    testBulkInsert("SQLite3");
}

TEST(SPTK_SQLite3Connection, bulkInsertPerformance)
{
    testBulkInsertPerformance("SQLite3");
}

TEST(SPTK_SQLite3Connection, queryParameters)
{
    testQueryParameters("SQLite3");
}

TEST(SPTK_SQLite3Connection, transaction)
{
    testTransaction("SQLite3");
}

TEST(SPTK_SQLite3Connection, select)
{
    testSelect("SQLite3");
}

// insertQuery test isn't defined because SQLite3 doesn't support auto-incremental fields

#endif
