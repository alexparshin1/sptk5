/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        ODBCConnection.h - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_ODBCCONNECTION_H__
#define __SPTK_ODBCCONNECTION_H__

#include <sptk5/sptk.h>

#if HAVE_ODBC == 1

#include <sptk5/db/ODBCEnvironment.h>
#include <sptk5/db/PoolDatabaseConnection.h>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * @brief ODBC database
 *
 * CODBCConnection is thread-safe connection to ODBC database.
 */
class SP_DRIVER_EXPORT ODBCConnection: public PoolDatabaseConnection
{
    friend class Query;

    /**
     * The ODBC connection object
     */
    ODBCConnectionBase *m_connect;


    /**
     * @brief Retrieves an error (if any) after statement was executed
     * @param stmt SQLHSTMT, the statement that had an error
     */
    String queryError(SQLHSTMT stmt) const;

protected:

    /**
     * @brief Begins the transaction
     */
    void driverBeginTransaction() override;

    /**
     * @brief Ends the transaction
     * @param commit bool, commit if true, rollback if false
     */
    void driverEndTransaction(bool commit) override;

    // These methods implement the actions requested by CQuery
    /**
     * Retrieves an error (if any) after executing a statement
     */
    String queryError(const Query* query) const override;

    /**
     * Allocates an ODBC statement
     */
    void queryAllocStmt(Query *query) override;

    /**
     * Deallocates an ODBC statement
     */
    void queryFreeStmt(Query *query) override;

    /**
     * Closes an ODBC statement
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
    int queryColCount(Query *query) override;

    /**
     * In a dataset returned by a query, retrieves the column attributes
     */
    void queryColAttributes(Query *query, int16_t column, int16_t descType, int32_t& value) override;

    /**
     * In a dataset returned by a query, retrieves the column attributes
     */
    void queryColAttributes(Query *query, int16_t column, int16_t descType, char *buff, int len) override;

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
     * Converts the native ODBC type into SPTK data type
     */
    static void ODBCtypeToCType(int odbcType, int32_t &ctype, VariantType& dataType);

    /**
     * Returns the ODBC connection object
     */
    ODBCConnectionBase *connection()
    {
        return m_connect;
    }

    /**
     * List all data sources (user and system)
     * @param dsns
     */
    void listDataSources(Strings& dsns);

    /**
     * @brief Opens the database connection. If unsuccessful throws an exception.
     * @param connectionString  The ODBC connection string
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

public:

    /**
     * @brief Constructor
     * @param connectionString  The ODBC connection string
     */
    explicit ODBCConnection(const String& connectionString = "");

    /**
     * @brief Destructor
     */
    ~ODBCConnection() override;

    /**
     * @brief Returns driver-specific connection string
     */
    String nativeConnectionString() const override;

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
    void* handle() const override;

    /**
     * @brief Returns the ODBC connection string for the active connection
     */
    virtual String connectString() const;

    /**
     * @brief Returns the ODBC driver description for the active connection
     */
    String driverDescription() const override;

    /**
     * @brief Lists database objects
     * @param objectType        Object type to list
     * @param objects           Object list (output)
     */
    void objectList(DatabaseObjectType objectType, Strings& objects) override;

};


/**
 * @}
 */
}
#endif

extern "C" {
    SP_DRIVER_EXPORT void* odbc_create_connection(const char* connectionString);
    SP_DRIVER_EXPORT void  odbc_destroy_connection(void* connection);
}

#endif
