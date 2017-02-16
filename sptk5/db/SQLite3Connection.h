/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        SQLite3Connection.h - description                     ║
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

#ifndef __SPTK_SQLITE3CONNECTION_H__
#define __SPTK_SQLITE3CONNECTION_H__

#include <sptk5/sptk.h>
#include <sptk5/sptk.h>

#if HAVE_SQLITE3 == 1

#include <sptk5/db/DatabaseConnection.h>
#include <sqlite3.h>

namespace sptk {
/**
 * @addtogroup Database Database Support
 * @{
 */

class Query;

/**
 * @brief SQLite3 database
 *
 * CSQLite3Connection is thread-safe connection to SQLite3 database.
 */
class SP_EXPORT SQLite3Connection: public DatabaseConnection
{
    friend class Query;

    typedef sqlite3_stmt * SQLHSTMT;
    typedef sqlite3 * SQLHDBC;

private:

    /**
     * The SQLite3 database connection object
     */
    sqlite3 *m_connect;


protected:

    /**
     * @brief Begins the transaction
     */
    virtual void driverBeginTransaction() THROWS_EXCEPTIONS override;

    /**
     * @brief Ends the transaction
     * @param commit bool, commit if true, rollback if false
     */
    virtual void driverEndTransaction(bool commit) THROWS_EXCEPTIONS override;

    // These methods implement the actions requested by CQuery
    /**
     * Retrieves an error (if any) after executing a statement
     */
    virtual std::string queryError(const Query *query) const override;

    /**
     * Allocates an SQLite3 statement
     */
    virtual void queryAllocStmt(Query *query) override;

    /**
     * Deallocates an SQLite3 statement
     */
    virtual void queryFreeStmt(Query *query) override;

    /**
     * Closes an SQLite3 statement
     */
    virtual void queryCloseStmt(Query *query) override;

    /**
     * Prepares a query if supported by database
     */
    virtual void queryPrepare(Query *query) override;

    /**
     * Unprepares a query if supported by database
     */
    virtual void queryUnprepare(Query *query) override;

    /**
     * Executes a statement
     */
    virtual void queryExecute(Query *query) override;

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    virtual int queryColCount(Query *query) override;

    /**
     * Binds the parameters to the query
     */
    virtual void queryBindParameters(Query *query) override;

    /**
     * Opens the query for reading data from the query' recordset
     */
    virtual void queryOpen(Query *query) override;

    /**
     * Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.
     */
    virtual void queryFetch(Query *query) override;


    /**
     * @brief Returns the SQLite3 connection object
     */
    sqlite3 *connection()
    {
        return m_connect;
    }

    /**
     * @brief Converts datatype from SQLite type to SPTK VariantType
     */
    void SQLITEtypeToCType(int sqliteType, VariantType& dataType);

public:

    /**
     * @brief Constructor
     * @param connectionString std::string, the SQLite3 connection string
     */
    SQLite3Connection(std::string connectionString = "");

    /**
     * @brief Destructor
     */
    virtual ~SQLite3Connection();

    /**
     * @brief Returns driver-specific connection string
     */
    std::string nativeConnectionString() const override;

    /**
     * @brief Opens the database connection. If unsuccessful throws an exception.
     * @param connectionString std::string, the SQLite3 connection string
     */
    virtual void openDatabase(std::string connectionString = "") THROWS_EXCEPTIONS override;

    /**
     * @brief Closes the database connection. If unsuccessful throws an exception.
     */
    virtual void closeDatabase() THROWS_EXCEPTIONS override;

    /**
     * @brief Returns true if database is opened
     */
    virtual bool active() const override;

    /**
     * @brief Returns the database connection handle
     */
    virtual void* handle() const override;

    /**
     * @brief Returns the SQLite3 driver description for the active connection
     */
    virtual std::string driverDescription() const override;

    /**
     * @brief Lists database objects
     * @param objectType CDbObjectType, object type to list
     * @param objects Strings&, object list (output)
     */
    virtual void objectList(DatabaseObjectType objectType, Strings& objects) THROWS_EXCEPTIONS override;
};

/**
 * @}
 */
}

#endif

extern "C" {
    SP_DRIVER_EXPORT void* sqlite3_create_connection(const char* connectionString);
    SP_DRIVER_EXPORT void  sqlite3_destroy_connection(void* connection);
}

#endif
