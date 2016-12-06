/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        MySQLConnection.h - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_MYSQLCONNECTION_H__
#define __SPTK_MYSQLCONNECTION_H__

#include <sptk5/db/DatabaseConnection.h>

#if HAVE_MYSQL == 1

#include <sptk5/db/MySQLStatement.h>

namespace sptk
{

/**
 * @addtogroup Database Database Support
 * @{
 */

class Query;
class CMySQLStatement;

/**
 * @brief MySQL database connection
 */
class SP_EXPORT MySQLConnection: public DatabaseConnection
{
    friend class Query;
    friend class MySQLStatement;

private:

    /**
     * MySQL database connection
     */
    MYSQL*  m_connection;


protected:

    /**
     * @brief Begins the transaction
     */
    void driverBeginTransaction() THROWS_EXCEPTIONS override;

    /**
     * @brief Ends the transaction
     * @param commit bool, commit if true, rollback if false
     */
    void driverEndTransaction(bool commit) THROWS_EXCEPTIONS override;

    // These methods implement the actions requested by CQuery
    /**
     * Retrieves an error (if any) after executing a statement
     */
    std::string queryError(const Query *query) const override;

    /**
     * Allocates an MySQL statement
     */
    void queryAllocStmt(Query *query) override;

    /**
     * Deallocates an MySQL statement
     */
    void queryFreeStmt(Query *query) override;

    /**
     * Closes an MySQL statement
     */
    void queryCloseStmt(Query *query) override;

    /**
     * Prepares a query if supported by database
     */
    void queryPrepare(Query *query) override;

    /**
     * Unprepares a query if supported by database
     */
    void queryUnprepare(Query *query) override;

    /**
     * Executes a statement
     */
    void queryExecute(Query *query) override;

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    int  queryColCount(Query *query) override;

    /**
     * Binds the parameters to the query
     */
    void queryBindParameters(Query *query) override;

    /**
     * Opens the query for reading data from the query' recordset
     */
    void queryOpen(Query *query) override;

    /**
     * Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.
     */
    void queryFetch(Query *query) override;


    /**
     * @brief Returns parameter mark
     *
     * Parameter mark is generated from the parameterIndex.
     * @param paramIndex unsigned, parameter index in SQL starting from 0
     */
    std::string paramMark(unsigned paramIndex) override;

public:
    /**
     * @brief Returns the MySQL connection object
     */
    MYSQL* connection() const
    {
        return m_connection;
    }

    MYSQL_STMT* createStatement(std::string sql);

    MYSQL_STMT* createStatement();

public:

    /**
     * @brief Constructor
     *
     * Typical connection string is something like: "dbname='mydb' host='myhostname' port=5142" and so on.
     * For more information please refer to:
     * http://www.postgresql.org/docs/current/interactive/libpq-connect.html
     * If the connection string is empty then default database with the name equal to user name is used.
     * @param connectionString std::string, the MySQL connection string
     */
    MySQLConnection(std::string connectionString = "");

    /**
     * @brief Destructor
     */
    virtual ~MySQLConnection();

    /**
     * @brief Opens the database connection. If unsuccessful throws an exception.
     * @param connectionString std::string, the MySQL connection string
     */
    void openDatabase(std::string connectionString = "") THROWS_EXCEPTIONS override;

    /**
     * @brief Closes the database connection. If unsuccessful throws an exception.
     */
    void closeDatabase() THROWS_EXCEPTIONS override;

    /**
     * @brief Returns true if database is opened
     */
    bool active() const override;

    /**
     * @brief Returns the database connection handle
     */
    void* handle() const override;

    /**
     * @brief Returns driver-specific connection string
     */
    std::string nativeConnectionString() const override;

    /**
     * @brief Returns the MySQL driver description for the active connection
     */
    std::string driverDescription() const override;

    /**
     * @brief Lists database objects
     * @param objectType CDbObjectType, object type to list
     * @param objects Strings&, object list (output)
     */
    void objectList(DatabaseObjectType objectType, Strings& objects) THROWS_EXCEPTIONS override;

    /**
     * @brief Executes bulk inserts of data from memory buffer
     *
     * Data is inserted the fastest possible way. The server-specific format definition provides extra information
     * about data. If format is empty than default server-specific data format is used.
     * For instance, for PostgreSQL it is TAB-delimited data, with some escaped characters ('\\t', '\\n', '\\r') and "\\N" for NULLs.
     * @param tableName std::string, table name to insert into
     * @param columnNames const Strings&, list of table columns to populate
     * @param data const Strings&, data for bulk insert
     */
    void bulkInsert(std::string tableName, const Strings& columnNames, const Strings& data, std::string format="") THROWS_EXCEPTIONS override;

    /**
     * @brief Executes SQL batch file
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL const sptk::Strings&, SQL batch file
     */
    void executeBatchSQL(const sptk::Strings& batchSQL) THROWS_EXCEPTIONS override;
};

#define throwMySQLException(info) throw DatabaseException(string(info) + ":" + string(mysql_error(m_connection)))

/**
 * @}
 */
}

#endif

extern "C" {
    SP_DRIVER_EXPORT void* mysql_create_connection(const char* connectionString);
    SP_DRIVER_EXPORT void  mysql_destroy_connection(void* connection);
}

#endif
