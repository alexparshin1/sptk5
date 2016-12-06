/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        PostgreSQLConnection.h - description                  ║
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

#ifndef __SPTK_POSTGRESQLCONNECTION_H__
#define __SPTK_POSTGRESQLCONNECTION_H__

#include <sptk5/db/DatabaseConnection.h>

#if HAVE_POSTGRESQL == 1

#include <libpq-fe.h>
#ifndef _WIN32
#include <netinet/in.h>
#endif

namespace sptk
{

/**
 * @addtogroup Database Database Support
 * @{
 */

class Query;
class CPostgreSQLStatement;

/**
 * @brief PostgreSQL database
 *
 * CPostgreSQLConnection is thread-safe connection to PostgreSQL database.
 */
class SP_EXPORT PostgreSQLConnection: public DatabaseConnection
{
    friend class Query;

private:

    /**
     * PostgreSQL database connection
     */
    PGconn* m_connect;


protected:

    /**
     * @brief Begins the transaction
     */
    virtual void driverBeginTransaction() THROWS_EXCEPTIONS;

    /**
     * @brief Ends the transaction
     * @param commit bool, commit if true, rollback if false
     */
    virtual void driverEndTransaction(bool commit) THROWS_EXCEPTIONS;

    // These methods implement the actions requested by CQuery
    /**
     * Retrieves an error (if any) after executing a statement
     */
    virtual std::string queryError(const Query *query) const;

    /**
     * Allocates an PostgreSQL statement
     */
    virtual void queryAllocStmt(Query *query);

    /**
     * Deallocates an PostgreSQL statement
     */
    virtual void queryFreeStmt(Query *query);

    /**
     * Closes an PostgreSQL statement
     */
    virtual void queryCloseStmt(Query *query);

    /**
     * Prepares a query if supported by database
     */
    virtual void queryPrepare(Query *query);

    /**
     * Unprepares a query if supported by database
     */
    virtual void queryUnprepare(Query *query);

    /**
     * Executes a statement
     */
    virtual void queryExecute(Query *query) {}

    /**
     * Executes unprepared statement
     */
    virtual void queryExecDirect(Query *query);

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    virtual int  queryColCount(Query *query);

    /**
     * Binds the parameters to the query
     */
    virtual void queryBindParameters(Query *query);

    /**
     * Opens the query for reading data from the query' recordset
     */
    virtual void queryOpen(Query *query);

    /**
     * Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.
     */
    virtual void queryFetch(Query *query);


    /**
     * @brief Returns parameter mark
     *
     * Parameter mark is generated from the parameterIndex.
     * @param paramIndex unsigned, parameter index in SQL starting from 0
     */
    virtual std::string paramMark(unsigned paramIndex);

public:
    /**
     * @brief Returns the PostgreSQL connection object
     */
    PGconn *connection()
    {
        return m_connect;
    }

public:
    /**
     * @brief Converts datatype from PostgreSQL type to SPTK VariantType
     */
    static void PostgreTypeToCType(int postgreType, VariantType& dataType);

    /**
     * @brief Converts datatype from SPTK VariantType to PostgreSQL type
     */
    static void CTypeToPostgreType(VariantType dataType, Oid& postgreType);

public:

    /**
     * @brief Constructor
     *
     * Typical connection string is something like: "dbname='mydb' host='myhostname' port=5142" and so on.
     * For more information please refer to:
     * http://www.postgresql.org/docs/current/interactive/libpq-connect.html
     * If the connection string is empty then default database with the name equal to user name is used.
     * @param connectionString std::string, the PostgreSQL connection string
     */
    PostgreSQLConnection(std::string connectionString = "");

    /**
     * @brief Destructor
     */
    virtual ~PostgreSQLConnection();

    /**
     * @brief Returns driver-specific connection string
     */
    virtual std::string nativeConnectionString() const;

    /**
     * @brief Opens the database connection. If unsuccessful throws an exception.
     * @param connectionString std::string, the PostgreSQL connection string
     */
    virtual void openDatabase(std::string connectionString = "") THROWS_EXCEPTIONS;

    /**
     * @brief Closes the database connection. If unsuccessful throws an exception.
     */
    virtual void closeDatabase() THROWS_EXCEPTIONS;

    /**
     * @brief Returns true if database is opened
     */
    virtual bool active() const;

    /**
     * @brief Returns the database connection handle
     */
    virtual void* handle() const;

    /**
     * @brief Returns the PostgreSQL driver description for the active connection
     */
    virtual std::string driverDescription() const;

    /**
     * @brief Lists database objects
     * @param objectType CDbObjectType, object type to list
     * @param objects Strings&, object list (output)
     */
    virtual void objectList(DatabaseObjectType objectType, Strings& objects) THROWS_EXCEPTIONS override;

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
    virtual void bulkInsert(std::string tableName, const Strings& columnNames, const Strings& data, std::string format="") THROWS_EXCEPTIONS;

    /**
     * @brief Executes SQL batch file
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL const sptk::Strings&, SQL batch file
     */
    virtual void executeBatchSQL(const sptk::Strings& batchSQL) THROWS_EXCEPTIONS override;
};

/**
 * @}
 */
}

#endif

extern "C" {
    SP_DRIVER_EXPORT void* postgresql_create_connection(const char* connectionString);
    SP_DRIVER_EXPORT void  postgresql_destroy_connection(void* connection);
}

#endif
