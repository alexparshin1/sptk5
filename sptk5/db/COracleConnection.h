/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          COracleConnection.h  -  description
                             -------------------
    begin                : Sat November 17 2012
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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

#ifndef __CORACLECONNECTION_H__
#define __CORACLECONNECTION_H__

#include <sptk5/db/CDatabaseConnection.h>

#if HAVE_ORACLE == 1

#include <sptk5/db/COracleStatement.h>
#include <sptk5/db/COracleEnvironment.h>

namespace sptk
{

/// @addtogroup Database Database Support
/// @{

class CQuery;
class COracleStatement;

/// @brief Oracle database
///
/// COracleConnection is thread-safe connection to Oracle database.
class SP_EXPORT COracleConnection: public CDatabaseConnection
{
    friend class CQuery;

public:
    typedef oracle::occi::Environment   Environment;
    typedef oracle::occi::Connection    Connection;
    typedef oracle::occi::Statement     Statement;
    typedef oracle::occi::Type          Type;

private:

    COracleEnvironment  m_environment;  ///< Oracle connection environment
    Connection*         m_connection;   ///< Oracle database connection
    std::string         m_lastError;    ///< Last error in this connection or query

    /// @brief Translates Oracle native type to CVariant type
    /// @param oracleType oracle::occi::Type, Oracle native type
    /// @returns CVariant type
    static CVariantType OracleTypeToVariantType(Type oracleType);

    /// @brief Translates CVariant type to Oracle native type
    /// @param dataType CVariantType&, CVariant type
    /// @returns Oracle native type
    static Type VariantTypeToOracleType(CVariantType dataType);

protected:

    /// @brief Begins the transaction
    virtual void driverBeginTransaction() throw (CDatabaseException);

    /// @brief Ends the transaction
    /// @param commit bool, commit if true, rollback if false
    virtual void driverEndTransaction(bool commit) throw (CDatabaseException);

    // These methods implement the actions requested by CQuery
    virtual std::string queryError(const CQuery *query) const; ///< Retrieves an error (if any) after executing a statement
    virtual void queryAllocStmt(CQuery *query);      ///< Allocates an Oracle statement
    virtual void queryFreeStmt(CQuery *query);       ///< Deallocates an Oracle statement
    virtual void queryCloseStmt(CQuery *query);      ///< Closes an Oracle statement
    virtual void queryPrepare(CQuery *query);        ///< Prepares a query if supported by database
    virtual void queryUnprepare(CQuery *query);      ///< Unprepares a query if supported by database
    virtual void queryExecute(CQuery *query) {};     ///< Executes a statement
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
    /// @brief Returns the Oracle connection object
    Connection* connection() const
    {
        return m_connection;
    }

    /// @brief Returns the Oracle connection object
    Environment* environment() const
    {
        return m_environment.handle();
    }

    Statement* createStatement(std::string sql)
    {
        return m_connection->createStatement(sql);
    }

public:

    /// @brief Constructor
    ///
    /// Typical connection string is something like: "dbname='mydb' host='myhostname' port=5142" and so on.
    /// For more information please refer to:
    /// http://www.postgresql.org/docs/current/interactive/libpq-connect.html
    /// If the connection string is empty then default database with the name equal to user name is used.
    /// @param connectionString std::string, the Oracle connection string
    COracleConnection(std::string connectionString = "");

    /// @brief Destructor
    virtual ~COracleConnection();

    /// @brief Opens the database connection. If unsuccessful throws an exception.
    /// @param connectionString std::string, the Oracle connection string
    virtual void openDatabase(std::string connectionString = "") throw (CDatabaseException);

    /// @brief Closes the database connection. If unsuccessful throws an exception.
    virtual void closeDatabase() throw (CDatabaseException);

    /// @brief Returns true if database is opened
    virtual bool active() const;

    /// @brief Returns the database connection handle
    virtual void* handle() const;

    /// @brief Returns driver-specific connection string
    virtual std::string nativeConnectionString() const;

    /// @brief Returns the Oracle driver description for the active connection
    virtual std::string driverDescription() const;

    /// @brief Lists database objects
    /// @param objectType CDbObjectType, object type to list
    /// @param objects CStrings&, object list (output)
    virtual void objectList(CDbObjectType objectType, CStrings& objects) throw (CDatabaseException);
};

#define throwOracleException(description) m_lastError = description; throwDatabaseException(m_lastError);

/// @}
}

#endif

extern "C" {
    SP_DRIVER_EXPORT void* oracle_create_connection(const char* connectionString);
    SP_DRIVER_EXPORT void  oracle_destroy_connection(void* connection);
}

#endif
