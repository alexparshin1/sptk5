/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DatabaseTests.cpp - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_DATABASE_TESTS_H__
#define __SPTK_DATABASE_TESTS_H__

#include "DatabaseConnectionString.h"
#include "AutoDatabaseConnection.h"

namespace sptk {

/**
 * Common operations used by database-related unit tests
 */
class DatabaseTests
{
    /**
     * Connection strings for which tests will be executed
     */
    std::map<String, DatabaseConnectionString> m_connectionStrings;

    /**
     * Get number of rows in table
     * @param db            Database connection
     * @param table         Database table
     * @return number of rows in table
     */
    size_t countRowsInTable(DatabaseConnection& db, const String& table);

    /**
     * Test transactions
     * @param db            Database connection
     * @param commit        If true then commit the transaction
     */
    void testTransaction(DatabaseConnection db, bool commit);

    /**
     * Connect to database and create test table
     * @return
     */
    void createTestTable(DatabaseConnection db);

public:
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
    std::vector<DatabaseConnectionString> connectionStrings() const;

    /**
     * Get connection string for database type (driver name).
     * @param driverName        Driver name
     * @return connection string
     */
    DatabaseConnectionString connectionString(const String& driverName) const;

    /**
     * Test database connection
     * @param connectionString Database connection string
     */
    void testConnect(const DatabaseConnectionString& connectionString);

    /**
     * Test SELECT statements
     * @param connectionString Database connection string
     */
    void testSelect(const DatabaseConnectionString& connectionString);

    /**
     * Test basic DDL statements
     * @param connectionString Database connection string
     */
    void testDDL(const DatabaseConnectionString& connectionString);

    /**
     * Test parametrized queries
     * @param connectionString Database connection string
     */
    void testQueryParameters(const DatabaseConnectionString& connectionString);

    /**
     * Test transaction
     * @param connectionString Database connection string
     */
    void testTransaction(const DatabaseConnectionString& connectionString);

    /**
     * Test bulk insert operation
     * @param connectionString Database connection string
     */
    void testBulkInsert(const DatabaseConnectionString& connectionString);

    /**
     * Test bulk insert operation performance
     * @param connectionString  Database connection string
     * @param recordCount       Records to insert during test
     */
    void testBulkInsertPerformance(const DatabaseConnectionString& connectionString, size_t recordCount);
};

/**
 * Global database tests collection
 */
extern DatabaseTests databaseTests;

}

#endif