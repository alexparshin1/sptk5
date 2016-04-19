/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CMySQLConnection.h  -  description
                             -------------------
    begin                : Wed Jul 24 2013
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CMYSQLCONNECTION_H__
#define __CMYSQLCONNECTION_H__

#include <sptk5/db/CDatabaseConnection.h>

#if HAVE_MYSQL == 1

#include <sptk5/db/CMySQLStatement.h>

namespace sptk
{

/// @addtogroup Database Database Support
/// @{

class CQuery;
class CMySQLStatement;

/// @brief MySQL database connection
class SP_EXPORT CMySQLConnection: public CDatabaseConnection
{
    friend class CQuery;
    friend class CMySQLStatement;

private:

    MYSQL*  m_connection;                           ///< MySQL database connection

protected:

    /// @brief Begins the transaction
    virtual void driverBeginTransaction() THROWS_EXCEPTIONS;

    /// @brief Ends the transaction
    /// @param commit bool, commit if true, rollback if false
    virtual void driverEndTransaction(bool commit) THROWS_EXCEPTIONS;

    // These methods implement the actions requested by CQuery
    virtual std::string queryError(const CQuery *query) const; ///< Retrieves an error (if any) after executing a statement
    virtual void queryAllocStmt(CQuery *query);      ///< Allocates an MySQL statement
    virtual void queryFreeStmt(CQuery *query);       ///< Deallocates an MySQL statement
    virtual void queryCloseStmt(CQuery *query);      ///< Closes an MySQL statement
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
    /// @brief Returns the MySQL connection object
    MYSQL* connection() const
    {
        return m_connection;
    }

    MYSQL_STMT* createStatement(std::string sql);

    MYSQL_STMT* createStatement();

public:

    /// @brief Constructor
    ///
    /// Typical connection string is something like: "dbname='mydb' host='myhostname' port=5142" and so on.
    /// For more information please refer to:
    /// http://www.postgresql.org/docs/current/interactive/libpq-connect.html
    /// If the connection string is empty then default database with the name equal to user name is used.
    /// @param connectionString std::string, the MySQL connection string
    CMySQLConnection(std::string connectionString = "");

    /// @brief Destructor
    virtual ~CMySQLConnection();

    /// @brief Opens the database connection. If unsuccessful throws an exception.
    /// @param connectionString std::string, the MySQL connection string
    virtual void openDatabase(std::string connectionString = "") THROWS_EXCEPTIONS;

    /// @brief Closes the database connection. If unsuccessful throws an exception.
    virtual void closeDatabase() THROWS_EXCEPTIONS;

    /// @brief Returns true if database is opened
    virtual bool active() const;

    /// @brief Returns the database connection handle
    virtual void* handle() const;

    /// @brief Returns driver-specific connection string
    virtual std::string nativeConnectionString() const;

    /// @brief Returns the MySQL driver description for the active connection
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

#define throwMySQLException(info) throw CDatabaseException(string(info) + ":" + string(mysql_error(m_connection)))

/// @}
}

#endif

extern "C" {
    SP_DRIVER_EXPORT void* mysql_create_connection(const char* connectionString);
    SP_DRIVER_EXPORT void  mysql_destroy_connection(void* connection);
}

#endif
