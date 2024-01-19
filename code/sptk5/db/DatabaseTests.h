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

#pragma once

#include "AutoDatabaseConnection.h"
#include "DatabaseConnectionString.h"
#include "Transaction.h"

namespace sptk {

/**
 * Common operations used by database-related unit tests
 */
class SP_EXPORT DatabaseTests
{
public:
    struct Row {
        int id;
        int64_t ssid;
        String name;
        double price;
        DateTime ts;
    };

    /**
     * Constructor
     */
    DatabaseTests();

    /**
     * Add database connection to future tests.
     * Only one connection string is allowed per database type (driver name).
     * @param connectionString Database connection string
     */
    void addDatabaseConnection(const DatabaseConnectionString& connectionString);

    /**
     * Get list of added database connections
     * @return list of added database connections
     */
    [[nodiscard]] std::vector<DatabaseConnectionString> connectionStrings() const;

    /**
     * Get connection string for database type (driver name).
     * @param driverName        Driver name
     * @return connection string
     */
    [[nodiscard]] DatabaseConnectionString connectionString(const String& driverName) const;

    /**
     * Test database connection
     * @param connectionString Database connection string
     */
    static void testConnect(const DatabaseConnectionString& connectionString);

    /**
     * Test SELECT statements
     * @param connectionString Database connection string
     */
    static void testSelect(const DatabaseConnectionString& connectionString);

    /**
     * Test SELECT statements
     * @param connectionString Database connection string
     */
    static void testSelect(DatabaseConnectionPool& connectionPool);

    /**
     * Test basic DDL statements
     * @param connectionString Database connection string
     */
    static void testDDL(const DatabaseConnectionString& connectionString);

    /**
     * Test query inserting date
     * @param connectionString
     */
    static void testQueryInsertDate(const DatabaseConnectionString& connectionString);

    /**
     * Test query inserting date
     * @param connectionString
     */
    static void testQueryInsertDateTime(const DatabaseConnectionString& connectionString);

    /**
     * Test parametrized queries
     * @param connectionString Database connection string
     */
    static void testQueryParameters(const DatabaseConnectionString& connectionString);

    /**
     * Test InsertQuery returning created value
     * @param connectionString
     */
    static void testInsertQuery(const DatabaseConnectionString& connectionString);

    /**
     * Test InsertQuery returning created value
     * @param connectionString
     */
    static void testInsertQueryDirect(const DatabaseConnectionString& connectionString);

    /**
     * Test transaction
     * @param connectionString Database connection string
     */
    static void testTransaction(const DatabaseConnectionString& connectionString);

    /**
     * Test bulk insert operation
     * @param connectionString Database connection string
     */
    static void testBulkInsert(const DatabaseConnectionString& connectionString);

    /**
     * Test bulk insert operation performance
     * @param connectionString  Database connection string
     * @param recordCount       Records to insert during test
     */
    static void testBulkInsertPerformance(const DatabaseConnectionString& connectionString, size_t recordCount);

    /**
     * Test batch SQL
     * @param connectionString  Database connection string
     */
    static void testBatchSQL(const DatabaseConnectionString& connectionString);


    /**
     * Test BLOB insert and select
     * @param connectionString  Database connection string
     */
    static void testBLOB(const DatabaseConnectionString& connectionString);

    /**
     * Global database tests collection
     */
    [[nodiscard]] static DatabaseTests& tests();

private:
    /**
     * Global database tests collection
     */
    static DatabaseTests _databaseTests;

    /**
     * Connection strings for which tests will be executed
     */
    std::map<String, DatabaseConnectionString> m_connectionStrings;

    /**
     * Get number of rows in table
     * @param databaseConnection                Database connection
     * @param table             Database table
     * @return number of rows in table
     */
    static size_t countRowsInTable(const DatabaseConnection& databaseConnection, const String& table);

    /**
     * Test transactions
     * @param databaseConnection                Database connection
     * @param commit            If true then commit the transaction
     */
    static void testTransaction(const DatabaseConnection& databaseConnection, bool commit);

    /**
     * Connect to database and create test table
     * @param databaseConnection                Database connection
     * @param autoPrepare       If true then use auto-prepared queries (default)
     * @param withBlob          If true then add BLOB field 'data' to the table
     */
    static void createTestTable(const DatabaseConnection& databaseConnection, bool autoPrepare = true, bool withBlob = false);

    /**
     * Connect to database and create test table with serial id
     * @param databaseConnection                Database connection
     */
    static void createTestTableWithSerial(const DatabaseConnection& databaseConnection);

    /**
     * Create Oracle autoincrement for a column
     * @param databaseConnection                Database connection
     * @param tableName         Table name
     * @param columnName        Column name
     */
    static void createOracleAutoIncrement(const DatabaseConnection& databaseConnection, const String& tableName, const String& columnName);
    static void createTempTable(const DatabaseConnectionString& connectionString, const DatabaseConnection& databaseConnection);
    static Buffer createClob();
    static void insertDataIntoTempTable(Buffer& clob, Query& insert);
    static void verifyInsertedData(const DatabaseConnection& databaseConnection, const Buffer& clob);
    static void verifyInsertedRow(const Row& row, const Buffer& clob, Query& select);
    static void verifyTableNoBlobs(const DatabaseConnection& databaseConnection);
    static void verifyBatchInsertedData(Query& selectData, const Strings& expectedResults);
    static size_t insertRecordsInTransaction(const DatabaseConnection& databaseConnection);
    static void verifyInvalidTransactionStateCommitThrows(Transaction& transaction);
    static void verifyInvalidTransactionStateRollbackThrows(Transaction& transaction);
};

} // namespace sptk
