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
#include <sptk5/db/DatabaseConnectionPool.h>

#include <future>
#include <gtest/gtest.h>
#include <sptk5/db/DatabaseTests.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

namespace {

void testConnect(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());

    if (connectionString.empty())
    {
        FAIL() << dbName << " connection string is empty";
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

void testCurrentTimestamp(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());

    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        const auto now = DateTime::Now();
        const auto [dbTime1, dbTime2] = DatabaseTests::testCurrentTimestamp(connectionString);
        const auto diffSeconds = chrono::duration_cast<chrono::seconds>(dbTime1 - dbTime2).count();
        EXPECT_GE(1, diffSeconds);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
}

void testSqlParserPerformance(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());

    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        DatabaseTests::testSqlParserPerformance(connectionString);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
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
        DatabaseTests::testInsertQuery(connectionString, true);
        if (connectionString.driverName() != "mysql")
        {
            DatabaseTests::testInsertQuery(connectionString, false);
        }
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
        DatabaseTests::testBulkInsertErrors(connectionString);
        DatabaseTests::testBatchSQL(connectionString);
    }
    catch (const Exception& e)
    {
        FAIL() << connectionString.toString(false) << ": " << e.what();
    }
}

void testParallelInsert(const String& dbName)
{
    const DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString(dbName.toLowerCase());
    if (connectionString.empty())
    {
        FAIL() << dbName << " connection is not defined";
    }

    try
    {
        DatabaseTests::testParallelBulkInsert(connectionString);
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
namespace sptk {

TEST(PostgreSQLConnectionTests,connect)
{
    testConnect("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,currentTimestamp)
{
    testCurrentTimestamp("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,sqlParserPerformance)
{
    testSqlParserPerformance("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,ddl)
{
    testDDL("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,bulkInsert)
{
    testBulkInsert("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,bulkParallelInsert)
{
    testParallelInsert("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,bulkInsertPerformance)
{
    testBulkInsertPerformance("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,queryParameters)
{
    testQueryParameters("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,dates)
{
    testQueryDateAndTimestamp("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,transaction)
{
    testTransaction("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,select)
{
    testSelect("PostgreSQL");
    testInvalidQuery("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,insertQuery)
{
    testInsertQuery("PostgreSQL");
}

TEST(PostgreSQLConnectionTests,blob)
{
    testBlobInsertAndSelect("PostgreSQL");
}

#endif

//───────────────────────────────── MySQL ────────────────────────────────────────────────

#ifdef HAVE_MYSQL

TEST(MySQLConnectionTests,connect)
{
    testConnect("MySQL");
}

TEST(MySQLConnectionTests,currentTimestamp)
{
    testCurrentTimestamp("MySQL");
}

TEST(MySQLConnectionTests,ddl)
{
    testDDL("MySQL");
}

TEST(MySQLConnectionTests,bulkInsert)
{
    testBulkInsert("MySQL");
}

TEST(MySQLConnectionTests,bulkParallelInsert)
{
    testParallelInsert("MySQL");
}

TEST(MySQLConnectionTests,bulkInsertPerformance)
{
    testBulkInsertPerformance("MySQL");
}

TEST(MySQLConnectionTests,queryParameters)
{
    testQueryParameters("MySQL");
}

/**
 * A TEXT parameter in the middle of a long parameter list.
 *
 * Found in XMQ: a row was updated with twenty parameters, the fourteenth of which was a ~300 byte
 * JSON document in a TEXT column. The server received bytes that were not the document - it refused
 * them as invalid text ("Incorrect string value"), and PostgreSQL, which accepts more, had been
 * storing them quietly. The same value passed on its own, and a value of the same length built on
 * the spot passed too, so the fault is in how the parameter is bound, not in the value.
 */
TEST(MySQLConnectionTests, textParameterAmongManyOthersKeepsItsValue)
{
    const auto connectionString = DatabaseTests::tests().connectionString("mysql");
    if (connectionString.empty())
    {
        GTEST_SKIP() << "MySQL connection is not defined";
    }

    DatabaseConnectionPool pool(connectionString.toString());
    const DatabaseConnection connection = pool.getConnection();
    DatabaseTests::dropTable(connection, "sptk_wide_params");

    {
        Query create(connection,
                     "CREATE TABLE sptk_wide_params (id int NOT NULL PRIMARY KEY,"
                     " s1 varchar(64), s2 varchar(64), s3 varchar(64), s4 varchar(64), s5 varchar(64),"
                     " s6 varchar(64), s7 varchar(64), s8 varchar(64), s9 varchar(64), s10 varchar(1024),"
                     " n1 int, n2 int, n3 int, payload text, n4 int, n5 int, n6 int, n7 int, n8 int)");
        create.exec();
    }

    // The size the failure had: long enough that a wrong length reads past the end of it.
    String document("{\"log_to\":\"xmq_server.log\",\"keep_logs\":7,\"min_log_level\":\"INFO\",\"log_level\":{");
    for (int i = 0; i < 9; ++i)
    {
        document.append("\"subject").append(to_string(i)).append("\":\"INFO\",");
    }
    document.append("\"subject9\":\"INFO\"");
    document.append(String(296 - document.size(), ' '));
    document.append("}");
    ASSERT_EQ(297u, document.size()) << "the document is meant to be the length the failure had";

    // A row to update, so that the statement is an UPDATE and not an INSERT.
    {
        Query insert(connection, "INSERT INTO sptk_wide_params (id, payload) VALUES (:id, :payload)");
        insert.param("id") = 1;
        insert.param("payload") = String("initial");
        insert.exec();
    }

    Query update(connection,
                 "UPDATE sptk_wide_params SET s1 = :s1, s2 = :s2, s3 = :s3, s4 = :s4, s5 = :s5,"
                 " s6 = :s6, s7 = :s7, s8 = :s8, s9 = :s9, s10 = :s10, n1 = :n1, n2 = :n2, n3 = :n3,"
                 " payload = :payload, n4 = :n4, n5 = :n5, n6 = :n6, n7 = :n7, n8 = :n8"
                 " WHERE id = :id");
    for (int i = 1; i <= 10; ++i)
    {
        update.param("s" + to_string(i)) = "value-" + to_string(i);
    }
    for (int i = 1; i <= 8; ++i)
    {
        update.param("n" + to_string(i)) = i;
    }
    update.param("payload") = document;
    update.param("id") = 1;

    EXPECT_NO_THROW(update.exec()) << "the server refused the text parameter as invalid";

    Query read(connection, "SELECT LENGTH(payload) AS len, payload FROM sptk_wide_params WHERE id = 1");
    read.open();
    ASSERT_FALSE(read.eof());
    const auto storedLength = read["len"].asInteger();
    const auto stored = read["payload"].asString();
    read.close();

    EXPECT_EQ(static_cast<int64_t>(document.size()), storedLength) << "the server holds a different number of bytes";

    // The first byte that differs, so a truncated or shifted value says where it went wrong instead
    // of only that it did.
    size_t firstDifference = 0;
    while (firstDifference < stored.size() && firstDifference < document.size() &&
           stored[firstDifference] == document[firstDifference])
    {
        ++firstDifference;
    }
    EXPECT_EQ(document.size(), firstDifference) << "the stored value differs from byte " << firstDifference;

    EXPECT_EQ(document, stored) << "the text parameter did not reach the server unchanged";
}

TEST(MySQLConnectionTests,dates)
{
    testQueryDateAndTimestamp("MySQL");
}

TEST(MySQLConnectionTests,transaction)
{
    testTransaction("MySQL");
}

TEST(MySQLConnectionTests,select)
{
    testSelect("MySQL");
    testInvalidQuery("MySQL");
}

TEST(MySQLConnectionTests,insertQuery)
{
    testInsertQuery("MySQL");
}

TEST(MySQLConnectionTests,blob)
{
    testBlobInsertAndSelect("MySQL");
}

#endif

//───────────────────────────────── Oracle ─────────────────────────────────────────────
#if defined(HAVE_ORACLE_OCI) || defined(HAVE_ORACLE)

TEST(OracleConnectionTests,connect)
{
    testConnect("Oracle");
}

TEST(OracleConnectionTests,currentTimestamp)
{
    testCurrentTimestamp("Oracle");
}

TEST(OracleConnectionTests,ddl)
{
    testDDL("Oracle");
}

TEST(OracleConnectionTests,bulkInsert)
{
    testBulkInsert("Oracle");
}

TEST(OracleConnectionTests,bulkInsertPerformance)
{
    testBulkInsertPerformance("Oracle");
}

TEST(OracleConnectionTests,queryParameters)
{
    testQueryParameters("Oracle");
}

TEST(OracleConnectionTests,dates)
{
    testQueryDateAndTimestamp("Oracle");
}

TEST(OracleConnectionTests,transaction)
{
    testTransaction("Oracle");
}

TEST(OracleConnectionTests,select)
{
    testSelect("Oracle");
    testInvalidQuery("Oracle");
}

TEST(OracleConnectionTests,insertQuery)
{
    testInsertQuery("Oracle");
}

TEST(OracleConnectionTests,blob)
{
    testBlobInsertAndSelect("Oracle");
}

#endif

//───────────────────────────────── MS SQL ─────────────────────────────────────────────
// Gated separately from HAVE_ODBC: ODBC support alone doesn't imply a live MSSQL DSN is
// configured (dsn_mssql) - USE_MSSQL_TESTS must be explicitly opted into via CMake.
#if defined(HAVE_ODBC) && defined(HAVE_MSSQL_TESTS)

TEST(MSSQLConnectionTests,connect)
{
    testConnect("MSSQL");
}

TEST(MSSQLConnectionTests,currentTimestamp)
{
    testCurrentTimestamp("MSSQL");
}

TEST(MSSQLConnectionTests,ddl)
{
    testDDL("MSSQL");
}

TEST(MSSQLConnectionTests,bulkInsert)
{
    testBulkInsert("MSSQL");
}

TEST(MSSQLConnectionTests,bulkParallelInsert)
{
    testParallelInsert("MSSQL");
}

TEST(MSSQLConnectionTests,bulkInsertPerformance)
{
    testBulkInsertPerformance("MSSQL");
}

TEST(MSSQLConnectionTests,queryParameters)
{
    testQueryParameters("MSSQL");
}

TEST(MSSQLConnectionTests,dates)
{
    testQueryDateAndTimestamp("MSSQL");
}

TEST(MSSQLConnectionTests,transaction)
{
    testTransaction("MSSQL");
}

TEST(MSSQLConnectionTests,select)
{
    testSelect("MSSQL");
    testInvalidQuery("MSSQL");
}

TEST(MSSQLConnectionTests,insertQuery)
{
    testInsertQuery("MSSQL");
}

TEST(MSSQLConnectionTests,blob)
{
    testBlobInsertAndSelect("MSSQL");
}

#endif

//───────────────────────────────── SQLite3 ────────────────────────────────────────────
#ifdef HAVE_SQLITE3

TEST(SQLite3ConnectionTests,connect)
{
    testConnect("SQLite3");
}

TEST(SQLite3ConnectionTests,currentTimestamp)
{
    testCurrentTimestamp("SQLite3");
}

TEST(SQLite3ConnectionTests,ddl)
{
    testDDL("SQLite3");
}

TEST(SQLite3ConnectionTests,bulkInsert)
{
    testBulkInsert("SQLite3");
}

TEST(SQLite3ConnectionTests,bulkInsertPerformance)
{
    testBulkInsertPerformance("SQLite3");
}

TEST(SQLite3ConnectionTests,queryParameters)
{
    testQueryParameters("SQLite3");
}

TEST(SQLite3ConnectionTests,dates)
{
    testQueryDateAndTimestamp("SQLite3");
}

TEST(SQLite3ConnectionTests,transaction)
{
    testTransaction("SQLite3");
}

TEST(SQLite3ConnectionTests,select)
{
    testSelect("SQLite3");
}

// insertQuery test isn't defined because SQLite3 doesn't support auto-incremental fields

#endif

} // namespace sptk_test
