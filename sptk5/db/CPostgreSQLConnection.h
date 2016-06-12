/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        CPostgreSQLConnection.h - description                 ║
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

#ifndef __CPOSTGRESQLCONNECTION_H__
#define __CPOSTGRESQLCONNECTION_H__

#include <sptk5/db/CDatabaseConnection.h>

#if HAVE_POSTGRESQL == 1

#include <libpq-fe.h>
#ifndef _WIN32
#include <netinet/in.h>
#endif

namespace sptk
{

/// @addtogroup Database Database Support
/// @{

class CQuery;
class CPostgreSQLStatement;

/// @brief PostgreSQL database
///
/// CPostgreSQLConnection is thread-safe connection to PostgreSQL database.
class SP_EXPORT CPostgreSQLConnection: public CDatabaseConnection
{
    friend class CQuery;

private:

    PGconn* m_connect;  ///< PostgreSQL database connection
    
protected:

    /// @brief Begins the transaction
    virtual void driverBeginTransaction() THROWS_EXCEPTIONS;

    /// @brief Ends the transaction
    /// @param commit bool, commit if true, rollback if false
    virtual void driverEndTransaction(bool commit) THROWS_EXCEPTIONS;

    // These methods implement the actions requested by CQuery
    virtual std::string queryError(const CQuery *query) const; ///< Retrieves an error (if any) after executing a statement
    virtual void queryAllocStmt(CQuery *query);      ///< Allocates an PostgreSQL statement
    virtual void queryFreeStmt(CQuery *query);       ///< Deallocates an PostgreSQL statement
    virtual void queryCloseStmt(CQuery *query);      ///< Closes an PostgreSQL statement
    virtual void queryPrepare(CQuery *query);        ///< Prepares a query if supported by database
    virtual void queryUnprepare(CQuery *query);      ///< Unprepares a query if supported by database
    virtual void queryExecute(CQuery *query) {}      ///< Executes a statement
    virtual void queryExecDirect(CQuery *query);     ///< Executes unprepared statement
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
    /// @brief Returns the PostgreSQL connection object
    PGconn *connection()
    {
        return m_connect;
    }

public:
    /// @brief Converts datatype from PostgreSQL type to SPTK CVariantType
    static void PostgreTypeToCType(int postgreType, CVariantType& dataType);

    /// @brief Converts datatype from SPTK CVariantType to PostgreSQL type
    static void CTypeToPostgreType(CVariantType dataType, Oid& postgreType);

public:

    /// @brief Constructor
    ///
    /// Typical connection string is something like: "dbname='mydb' host='myhostname' port=5142" and so on.
    /// For more information please refer to:
    /// http://www.postgresql.org/docs/current/interactive/libpq-connect.html
    /// If the connection string is empty then default database with the name equal to user name is used.
    /// @param connectionString std::string, the PostgreSQL connection string
    CPostgreSQLConnection(std::string connectionString = "");

    /// @brief Destructor
    virtual ~CPostgreSQLConnection();

    /// @brief Returns driver-specific connection string
    virtual std::string nativeConnectionString() const;

    /// @brief Opens the database connection. If unsuccessful throws an exception.
    /// @param connectionString std::string, the PostgreSQL connection string
    virtual void openDatabase(std::string connectionString = "") THROWS_EXCEPTIONS;

    /// @brief Closes the database connection. If unsuccessful throws an exception.
    virtual void closeDatabase() THROWS_EXCEPTIONS;

    /// @brief Returns true if database is opened
    virtual bool active() const;

    /// @brief Returns the database connection handle
    virtual void* handle() const;

    /// @brief Returns the PostgreSQL driver description for the active connection
    virtual std::string driverDescription() const;

    /// @brief Lists database objects
    /// @param objectType CDbObjectType, object type to list
    /// @param objects CStrings&, object list (output)
    virtual void objectList(CDbObjectType objectType, CStrings& objects) THROWS_EXCEPTIONS;

    /// @brief Executes bulk inserts of data from memory buffer
    ///
    /// Data is inserted the fastest possible way. The server-specific format definition provides extra information
    /// about data. If format is empty than default server-specific data format is used.
    /// For instance, for PostgreSQL it is TAB-delimited data, with some escaped characters ('\\t', '\\n', '\\r') and "\\N" for NULLs.
    /// @param tableName std::string, table name to insert into
    /// @param columnNames const CStrings&, list of table columns to populate
    /// @param data const CStrings&, data for bulk insert
    virtual void bulkInsert(std::string tableName, const CStrings& columnNames, const CStrings& data, std::string format="") THROWS_EXCEPTIONS;

    /// @brief Executes SQL batch file
    ///
    /// Queries are executed in not prepared mode.
    /// Syntax of the SQL batch file is matching the native for the database.
    /// @param batchFile std::string, SQL batch file
    /// @param columnNames const CStrings&, list of table columns to populate
    /// @param data const CStrings&, data for bulk insert
    /// @param format std::string, data format (may be database-specific). The default is TAB-delimited data.
    virtual void executeBatchFile(std::string batchFile) THROWS_EXCEPTIONS;
};

/// @}
}

#endif

extern "C" {
    SP_DRIVER_EXPORT void* postgresql_create_connection(const char* connectionString);
    SP_DRIVER_EXPORT void  postgresql_destroy_connection(void* connection);
}

#endif
