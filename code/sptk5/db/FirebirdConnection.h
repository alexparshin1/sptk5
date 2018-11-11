/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        FirebirdConnection.h - description                    ║
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

#ifndef __SPTK_FIREBIRDCONNECTION_H__
#define __SPTK_FIREBIRDCONNECTION_H__

#include <sptk5/db/PoolDatabaseConnection.h>

#if HAVE_FIREBIRD == 1

#include <ibase.h>
#include "AutoDatabaseConnection.h"

namespace sptk
{

/**
 * @addtogroup Database Database Support
 * @{
 */

class FirebirdStatement;

/**
 * @brief Firebird database connection
 */
class SP_EXPORT FirebirdConnection: public PoolDatabaseConnection
{
    friend class Query;
    friend class FirebirdStatement;

    /**
     * Database connection handle
     */
    isc_db_handle   m_connection;

    /**
     * Database transaction handle
     */
    isc_tr_handle   m_transaction;

    /**
     * Connection status on last checkStatus
     */
    std::string     m_lastStatus;

protected:

    /**
     * @brief Opens the database connection. If unsuccessful throws an exception.
     * @param connectionString  The Firebird connection string
     */
    void _openDatabase(const String& connectionString) override;

    void checkStatus(const ISC_STATUS* status_vector, const char* file, int line);

    /**
     * @brief Begins the transaction
     */
    void driverBeginTransaction() override;

    /**
     * @brief Ends the transaction
     * @param commit            Commit if true, rollback if false
     */
    void driverEndTransaction(bool commit) override;
    // These methods implement the actions requested by Query

    /**
     * Retrieves an error (if any) after executing a statement
     */
    String queryError(const Query *query) const override;

    /**
     * Allocates an Firebird statement
     */
    void queryAllocStmt(Query *query) override;

    /**
     * Deallocates an Firebird statement
     */
    void queryFreeStmt(Query *query) override;

    /**
     * Closes an Firebird statement
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
     * @param paramIndex        Parameter index in SQL starting from 0
     */
    String paramMark(unsigned paramIndex) override;

public:

    /**
     * @brief Returns the Firebird connection object
     */
    isc_db_handle connection()
    {
        return m_connection;
    }

    isc_stmt_handle* createStatement(std::string sql);

    isc_stmt_handle* createStatement();

    /**
     * @brief Constructor
     *
     * Typical connection string is something like: "dbname='mydb' host='myhostname' port=5142" and so on.
     * For more information please refer to:
     * http://www.postgresql.org/docs/current/interactive/libpq-connect.html
     * If the connection string is empty then default database with the name equal to user name is used.
     * @param connectionString  The Firebird connection string
     */
    explicit FirebirdConnection(const String& connectionString = "");

    /**
     * @brief Destructor
     */
    ~FirebirdConnection() override;

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
     * @brief Returns driver-specific connection string
     */
    String nativeConnectionString() const override;

    /**
     * @brief Returns the Firebird driver description for the active connection
     */
    String driverDescription() const override;

    /**
     * @brief Lists database objects
     * @param objectType DatabaseObjectType, object type to list
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
    SP_DRIVER_EXPORT void* firebird_create_connection(const char* connectionString);
    SP_DRIVER_EXPORT void  firebird_destroy_connection(void* connection);
}

#endif
