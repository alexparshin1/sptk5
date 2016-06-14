/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        FirebirdConnection.h - description                    ║
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

#ifndef __SPTK_FIREBIRDCONNECTION_H__
#define __SPTK_FIREBIRDCONNECTION_H__

#include <sptk5/db/DatabaseConnection.h>

#if HAVE_FIREBIRD == 1

#include <ibase.h>

namespace sptk
{

/// @addtogroup Database Database Support
/// @{

class CQuery;
class CFirebirdStatement;

/// @brief Firebird database connection
class SP_EXPORT CFirebirdConnection: public CDatabaseConnection
{
    friend class CQuery;
    friend class CFirebirdStatement;

protected:
    
    isc_db_handle   m_connection;               ///< Database connection handle
    isc_tr_handle   m_transaction;              ///< Database transaction handle
    std::string     m_lastStatus;               ///< Connection status on last checkStatus
    
    void checkStatus(const ISC_STATUS* status_vector, const char* file, int line) THROWS_EXCEPTIONS;
    
    /// @brief Begins the transaction
    virtual void driverBeginTransaction() THROWS_EXCEPTIONS;

    /// @brief Ends the transaction
    /// @param commit bool, commit if true, rollback if false
    virtual void driverEndTransaction(bool commit) THROWS_EXCEPTIONS;

    // These methods implement the actions requested by CQuery
    virtual std::string queryError(const CQuery *query) const; ///< Retrieves an error (if any) after executing a statement
    virtual void queryAllocStmt(CQuery *query);      ///< Allocates an Firebird statement
    virtual void queryFreeStmt(CQuery *query);       ///< Deallocates an Firebird statement
    virtual void queryCloseStmt(CQuery *query);      ///< Closes an Firebird statement
    virtual void queryPrepare(CQuery *query);        ///< Prepares a query if supported by database
    virtual void queryUnprepare(CQuery *query);      ///< Unprepares a query if supported by database
    virtual void queryExecute(CQuery *query);        ///< Executes a statement
    virtual int  queryColCount(CQuery *query);       ///< Counts columns of the dataset (if any) returned by query
    virtual void queryBindParameters(CQuery *query); ///< Binds the parameters to the query
    virtual void queryOpen(CQuery *query);           ///< Opens the query for reading data from the query' recordset
    virtual void queryFetch(CQuery *query);          ///< Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.

    /// @brief Returns parameter mark
    ///
    /// Parameter mark is generated from the parameterIndex.
    /// @param paramIndex unsigned, parameter index in SQL starting from 0
    virtual std::string paramMark(unsigned paramIndex);

public:
    /// @brief Returns the Firebird connection object
    isc_db_handle connection()
    {
        return m_connection;
    }

    isc_stmt_handle* createStatement(std::string sql);

    isc_stmt_handle* createStatement();

public:

    /// @brief Constructor
    ///
    /// Typical connection string is something like: "dbname='mydb' host='myhostname' port=5142" and so on.
    /// For more information please refer to:
    /// http://www.postgresql.org/docs/current/interactive/libpq-connect.html
    /// If the connection string is empty then default database with the name equal to user name is used.
    /// @param connectionString std::string, the Firebird connection string
    CFirebirdConnection(std::string connectionString = "");

    /// @brief Destructor
    virtual ~CFirebirdConnection();

    /// @brief Opens the database connection. If unsuccessful throws an exception.
    /// @param connectionString std::string, the Firebird connection string
    virtual void openDatabase(std::string connectionString = "") THROWS_EXCEPTIONS;

    /// @brief Closes the database connection. If unsuccessful throws an exception.
    virtual void closeDatabase() THROWS_EXCEPTIONS;

    /// @brief Returns true if database is opened
    virtual bool active() const;

    /// @brief Returns the database connection handle
    virtual void* handle() const;

    /// @brief Returns driver-specific connection string
    virtual std::string nativeConnectionString() const;

    /// @brief Returns the Firebird driver description for the active connection
    virtual std::string driverDescription() const;

    /// @brief Lists database objects
    /// @param objectType CDbObjectType, object type to list
    /// @param objects Strings&, object list (output)
    virtual void objectList(CDbObjectType objectType, Strings& objects) THROWS_EXCEPTIONS;
};

/// @}
}

#endif

extern "C" {
    SP_DRIVER_EXPORT void* firebird_create_connection(const char* connectionString);
    SP_DRIVER_EXPORT void  firebird_destroy_connection(void* connection);
}

#endif
