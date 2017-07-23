/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        ODBCConnection.h - description                        ║
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

#ifndef __SPTK_ODBCCONNECTION_H__
#define __SPTK_ODBCCONNECTION_H__

#include <sptk5/sptk.h>

#if HAVE_ODBC == 1

#include <sptk5/db/ODBCEnvironment.h>
#include <sptk5/db/DatabaseConnection.h>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

class ODBCConnection;
class Query;

/**
 * @brief ODBC database
 *
 * CODBCConnection is thread-safe connection to ODBC database.
 */
class SP_DRIVER_EXPORT ODBCConnection: public DatabaseConnection
{
    friend class Query;

private:

    /**
     * The ODBC connection object
     */
    ODBCConnectionBase *m_connect;


    /**
     * @brief Retrieves an error (if any) after statement was executed
     * @param stmt SQLHSTMT, the statement that had an error
     */
    std::string queryError(SQLHSTMT stmt) const;

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
     * Allocates an ODBC statement
     */
    virtual void queryAllocStmt(Query *query);

    /**
     * Deallocates an ODBC statement
     */
    virtual void queryFreeStmt(Query *query);

    /**
     * Closes an ODBC statement
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
    virtual void queryExecute(Query *query);

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    virtual int queryColCount(Query *query);

    /**
     * In a dataset returned by a query, retrieves the column attributes
     */
    virtual void queryColAttributes(Query *query, int16_t column, int16_t descType, int32_t& value);

    /**
     * In a dataset returned by a query, retrieves the column attributes
     */
    virtual void queryColAttributes(Query *query, int16_t column, int16_t descType, char *buff, int len);

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

public:

    /**
     * @brief Constructor
     * @param connectionString std::string, the ODBC connection string
     */
    ODBCConnection(const std::string& connectionString = "");

    /**
     * @brief Destructor
     */
    virtual ~ODBCConnection();

    /**
     * @brief Returns driver-specific connection string
     */
    virtual std::string nativeConnectionString() const;

    /**
     * @brief Opens the database connection. If unsuccessful throws an exception.
     * @param connectionString std::string, the ODBC connection string
     */
    virtual void openDatabase(const std::string& connectionString = "") THROWS_EXCEPTIONS override;

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
     * @brief Returns the ODBC connection string for the active connection
     */
    virtual std::string connectString() const;

    /**
     * @brief Returns the ODBC driver description for the active connection
     */
    virtual std::string driverDescription() const;

    /**
     * @brief Lists database objects
     * @param objectType CDbObjectType, object type to list
     * @param objects Strings&, object list (output)
     */
    virtual void objectList(DatabaseObjectType objectType, Strings& objects) THROWS_EXCEPTIONS override;

    /**
     * @brief Executes SQL batch file
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL const sptk::Strings&, SQL batch file
     * @param errors Strings*, Errors during execution. If provided, then errors are stored here, instead of exceptions
     */
    virtual void executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors=NULL) THROWS_EXCEPTIONS override;
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
