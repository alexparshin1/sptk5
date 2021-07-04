/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/db/PoolDatabaseConnection.h>

#if HAVE_MYSQL == 1

#include <sptk5/db/MySQLStatement.h>
#include <mutex>

#ifdef _WIN32
#define ULONG_CAST (unsigned long)
#else
#define ULONG_CAST
#endif

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * @brief MySQL database connection
 */
class SP_EXPORT MySQLConnection
    : public PoolDatabaseConnection
{
    friend class Query;

    friend class MySQLStatement;

public:
    /**
     * @brief Returns the MySQL connection object
     */
    MYSQL* connection() const
    {
        return m_connection.get();
    }

    /**
     * @brief Opens the database connection. If unsuccessful throws an exception.
     * @param connectionString  The MySQL connection string
     */
    void _openDatabase(const String& connectionString) override;

    /**
     * @brief Executes SQL batch file
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL          SQL batch file
     * @param errors            If not nullptr, store errors here instead of exceptions
     */
    void _executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors) override;

    /**
     * @brief Constructor
     *
     * Typical connection string is something like: "dbname='mydb' host='myhostname' port=5142" and so on.
     * For more information please refer to:
     * http://www.postgresql.org/docs/current/interactive/libpq-connect.html
     * If the connection string is empty then default database with the name equal to user name is used.
     * @param connectionString  The MySQL connection string
     */
    explicit MySQLConnection(const String& connectionString = "");

    MySQLConnection(const MySQLConnection&) = delete;

    MySQLConnection(MySQLConnection&&) = delete;

    MySQLConnection& operator=(const MySQLConnection&) = delete;

    MySQLConnection& operator=(MySQLConnection&&) = delete;

    virtual ~MySQLConnection() = default;

    /**
     * @brief Closes the database connection. If unsuccessful throws an exception.
     */
    void closeDatabase() override;

    /**
     * @brief Returns true if database is opened
     */
    bool active() const override;

    /**
     * @brief Returns the database connection handle
     */
    DBHandle handle() const override;

    /**
     * @brief Returns the MySQL driver description for the active connection
     */
    String driverDescription() const override;

    /**
     * @brief Lists database objects
     * @param objectType        Object type to list
     * @param objects           Object list (output)
     */
    void objectList(DatabaseObjectType objectType, Strings& objects) override;

protected:

    /**
     * @brief Begins the transaction
     */
    void driverBeginTransaction() override;

    /**
     * @brief Ends the transaction
     * @param commit            Rollback if false
     */
    void driverEndTransaction(bool commit) override;

    // These methods implement the actions requested by CQuery
    /**
     * Retrieves an error (if any) after executing a statement
     */
    String queryError(const Query* query) const override;

    /**
     * Allocates an MySQL statement
     */
    void queryAllocStmt(Query* query) override;

    /**
     * Deallocates an MySQL statement
     */
    void queryFreeStmt(Query* query) override;

    /**
     * Closes an MySQL statement
     */
    void queryCloseStmt(Query* query) override;

    /**
     * Prepares a query if supported by database
     */
    void queryPrepare(Query* query) override;

    /**
     * Unprepares a query if supported by database
     */
    void queryUnprepare(Query* query) override;

    /**
     * Executes a statement
     */
    void queryExecute(Query* query) override;

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    int queryColCount(Query* query) override;

    /**
     * Binds the parameters to the query
     */
    void queryBindParameters(Query* query) override;

    /**
     * Opens the query for reading data from the query' recordset
     */
    void queryOpen(Query* query) override;

    /**
     * Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.
     */
    void queryFetch(Query* query) override;


    /**
     * @brief Returns parameter mark
     *
     * Parameter mark is generated from the parameterIndex.
     * @param paramIndex        Parameter index in SQL starting from 0
     */
    String paramMark(unsigned paramIndex) override;

private:

    std::shared_ptr<MYSQL> m_connection; ///< MySQL database connection
    mutable std::mutex m_mutex;      ///< Mutex that protects access to data members

    /**
     * @brief Init connection to MySQL server
     */
    void initConnection();

    /**
     * @brief Execute MySQL command
     */
    void executeCommand(const String& command);
};

#define throwMySQLException(info) throw DatabaseException(string(info) + ":" + string(mysql_error(m_connection.get())))

/**
 * @}
 */
}

#endif

extern "C" {
SP_DRIVER_EXPORT void* mysql_create_connection(const char* connectionString);
SP_DRIVER_EXPORT void mysql_destroy_connection(void* connection);
}
