/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CPostgreSQLDatabase.h  -  description
                             -------------------
    begin                : Mon Sep 17 2007
    copyright            : (C) 2007-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CPOSTGRESQLDATABASE_H__
#define __CPOSTGRESQLDATABASE_H__

#include <sptk5/db/CDatabaseDriver.h>

#if HAVE_POSTGRESQL == 1

#include <libpq-fe.h>

namespace sptk
{

/// @addtogroup Database Database Support
/// @{

class CQuery;
class CPostgreSQLStatement;

/// @brief PostgreSQL database
///
/// CPostgreSQLDatabase is thread-safe connection to PostgreSQL database.
class SP_EXPORT CPostgreSQLDatabase: public CDatabaseDriver
{
    friend class CQuery;

private:

    PGconn* m_connect;  ///< PostgreSQL database connection

protected:

    /// @brief Begins the transaction
    virtual void driverBeginTransaction() throw (CException);

    /// @brief Ends the transaction
    /// @param commit bool, commit if true, rollback if false
    virtual void driverEndTransaction(bool commit) throw (CException);

    // These methods implement the actions requested by CQuery
    virtual std::string queryError(const CQuery *query) const; ///< Retrieves an error (if any) after executing a statement
    virtual void queryAllocStmt(CQuery *query);      ///< Allocates an PostgreSQL statement
    virtual void queryFreeStmt(CQuery *query);       ///< Deallocates an PostgreSQL statement
    virtual void queryCloseStmt(CQuery *query);      ///< Closes an PostgreSQL statement
    virtual void queryPrepare(CQuery *query);        ///< Prepares a query if supported by database
    virtual void queryUnprepare(CQuery *query);      ///< Unprepares a query if supported by database
    virtual void queryExecute(CQuery *query) {};     ///< Executes a statement
    virtual int queryColCount(CQuery *query);        ///< Counts columns of the dataset (if any) returned by query
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
    CPostgreSQLDatabase(std::string connectionString = "");

    /// @brief Destructor
    virtual ~CPostgreSQLDatabase();

    /// @brief Opens the database connection. If unsuccessful throws an exception.
    /// @param connectionString std::string, the PostgreSQL connection string
    virtual void openDatabase(std::string connectionString = "") throw (CException);

    /// @brief Closes the database connection. If unsuccessful throws an exception.
    virtual void closeDatabase() throw (CException);

    /// @brief Returns true if database is opened
    virtual bool active() const;

    /// @brief Returns the database connection handle
    virtual void* handle() const;

    /// @brief Returns the PostgreSQL driver description for the active connection
    virtual std::string driverDescription() const;

    /// @brief Lists database objects
    /// @param objectType CDbObjectType, object type to list
    /// @param objects CStrings&, object list (output)
    virtual void objectList(CDbObjectType objectType, CStrings& objects) throw (std::exception);
};

/// @}
}

#endif

#endif
