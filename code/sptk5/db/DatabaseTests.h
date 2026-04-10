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

#pragma once

#include "AutoDatabaseConnection.h"
#include "DatabaseConnectionString.h"
#include "Transaction.h"

namespace sptk {
/**
 * @brief Common operations used by database-related unit tests.
 */
class SP_EXPORT DatabaseTests
{
public:
    struct Row
    {
        int      id;
        int64_t  ssid;
        String   name;
        double   price;
        DateTime ts;
    };

    /**
     * @brief Constructor.
     */
    DatabaseTests();

    /**
     * @brief Add the database connection to future tests.
     * Only one connection string is allowed per database type (driver name).
     * @param connectionString Database connection string.
     */
    void addDatabaseConnection(const DatabaseConnectionString& connectionString);

    /**
     * @brief Get the list of added database connections.
     * @return list of added database connections.
     */
    [[nodiscard]] std::vector<DatabaseConnectionString> connectionStrings() const;

    /**
     * @brief Get the connection string for the database type (driver name).
     * @param driverName        Driver name.
     * @return connection string.
     */
    [[nodiscard]] DatabaseConnectionString connectionString(const String& driverName) const;

    /**
     * @brief Test database connection.
     * @param connectionString Database connection string.
     */
    static void                           testConnect(const DatabaseConnectionString& connectionString);
    static void                           dropTable(const DatabaseConnection& databaseConnection, const String& tableName);
    static std::tuple<DateTime, DateTime> testCurrentTimestamp(const DatabaseConnectionString& connectionString);

    /**
     * @brief Test SQL parser performance.
     * @param connectionString Database connection string.
     */
    static void testSqlParserPerformance(const DatabaseConnectionString& connectionString);

    /**
     * @brief Test SELECT statements.
     * @param connectionString Database connection string.
     */
    static void testSelect(const DatabaseConnectionString& connectionString);

    /**
     * @brief Test SELECT statements.
     * @param connectionPool    Database connection pool.
     */
    static void testSelect(DatabaseConnectionPool& connectionPool);

    /**
     * @brief Test basic DDL statements.
     * @param connectionString Database connection string.
     */
    static void testDDL(const DatabaseConnectionString& connectionString);

    /**
     * @brief Test query inserting date.
     * @param connectionString  Connection string.
     */
    static void testQueryInsertDate(const DatabaseConnectionString& connectionString);

    /**
     * @brief Test query inserting date.
     * @param connectionString  Connection string.
     */
    static void testQueryInsertDateTime(const DatabaseConnectionString& connectionString);

    /**
     * @brief Test parametrized queries.
     * @param connectionString Database connection string.
     */
    static void testQueryParameters(const DatabaseConnectionString& connectionString);

    /**
     * @brief Test InsertQuery returning the created value.
     * @param connectionString  Connection string.
     * @param autoPrepare       Auto-prepare insert queries
     */
    static void testInsertQuery(const DatabaseConnectionString& connectionString, bool autoPrepare);

    /**
     * @brief Test InsertQuery returning the created value.
     * @param connectionString  Connection string.
     */
    static void testInsertQueryDirect(const DatabaseConnectionString& connectionString);

    /**
     * @brief Test transaction.
     * @param connectionString Database connection string.
     */
    static void testTransaction(const DatabaseConnectionString& connectionString);

    /**
     * @brief Test bulk insert operation.
     * @param connectionString Database connection string.
     */
    static void testBulkInsert(const DatabaseConnectionString& connectionString);
    static void testBulkInsertErrors(const DatabaseConnectionString& connectionString);

    /**
     * @brief Test bulk insert operation performance.
     * @param connectionString  Database connection string.
     * @param recordCount       Records to insert during test.
     */
    static void testBulkInsertPerformance(const DatabaseConnectionString& connectionString, size_t recordCount);

    /**
     * @brief Test batch SQL.
     * @param connectionString  Database connection string.
     */
    static void testBatchSQL(const DatabaseConnectionString& connectionString);

    /**
     * @brief Test BLOB insert and select.
     * @param connectionString  Database connection string.
     */
    static void testBLOB(const DatabaseConnectionString& connectionString);

    /**
     * @brief Global database tests collection.
     */
    [[nodiscard]] static DatabaseTests& tests();

    static void testParallelBulkInsert(const DatabaseConnectionString& connectionString);

private:
    /**
     * @brief Global database tests collection.
     */
    static DatabaseTests _databaseTests;

    /**
     * @brief Connection strings for which tests will be executed.
     */
    std::map<String, DatabaseConnectionString> m_connectionStrings;

    /**
     * @brief Get the number of rows in the table.
     * @param databaseConnection                Database connection.
     * @param table             Database table.
     * @return number of rows in the table.
     */
    static size_t countRowsInTable(const DatabaseConnection& databaseConnection, const String& table);

    /**
     * @brief Test transactions.
     * @param databaseConnection                Database connection.
     * @param commit            If true, then commit the transaction.
     */
    static void testTransaction(const DatabaseConnection& databaseConnection, bool commit);

    /**
     * @brief Connect to the database and create the test table.
     * @param databaseConnection    Database connection.
     * @param autoPrepare           If true, then use auto-prepared queries (default).
     * @param withBlob              If true, then add BLOB field 'data' to the table.
     */
    static void createTestTable(const DatabaseConnection& databaseConnection, bool autoPrepare = true, bool withBlob = false);

    /**
     * @brief Connect to the database and create the test table with serial id.
     * @param databaseConnection    Database connection.
     * @param autoPrepareQueries    Auto-prepare insert queries.
     */
    static void createTestTableWithSerial(const DatabaseConnection& databaseConnection, bool autoPrepareQueries);

    static void   createTempTable(const DatabaseConnectionString& connectionString, const DatabaseConnection& databaseConnection);
    static Buffer createClob();
    static void   insertDataIntoTempTable(Buffer& clob, Query& insert);
    static void   verifyInsertedData(const DatabaseConnection& databaseConnection, const Buffer& clob);
    static void   verifyInsertedRow(const Row& row, const Buffer& clob, Query& select);
    static void   verifyTableNoBlobs(const DatabaseConnection& databaseConnection);
    static void   verifyBatchInsertedData(Query& selectData, const Strings& expectedResults);
    static size_t insertRecordsInTransaction(const DatabaseConnection& databaseConnection);
    static void   invalidTransactionStateThrows(Transaction& transaction);
    static String serialColumnDefinition(DatabaseConnectionType connectionType);
};

} // namespace sptk
