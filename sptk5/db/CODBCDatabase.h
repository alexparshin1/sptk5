/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CODBCDatabase.h  -  description
                             -------------------
    begin                : Fri Oct 03 2003
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CODBCDATABASE_H__
#define __CODBCDATABASE_H__

#include <sptk5/sptk.h>

#if HAVE_ODBC == 1

#include <sptk5/db/CODBC.h>
#include <sptk5/db/CDatabase.h>

namespace sptk {

/// @addtogroup Database Database Support
/// @{

class CODBCConnection;
class CQuery;

/// @brief ODBC database
///
/// CODBCDatabase is thread-safe connection to ODBC database.
class SP_EXPORT CODBCDatabase : public CDatabase {
    friend class CQuery;

private:

    CODBCConnection  *m_connect;   ///< The ODBC connection object

    /// @brief Retrieves an error (if any) after statement was executed
    /// @param stmt SQLHSTMT, the statement that had an error
    std::string queryError(SQLHSTMT stmt) const;

protected:

    /// @brief Begins the transaction
    virtual void driverBeginTransaction() throw(CException);

    /// @brief Ends the transaction
    /// @param commit bool, commit if true, rollback if false
    virtual void driverEndTransaction(bool commit) throw(CException);

    // These methods implement the actions requested by CQuery
    virtual std::string queryError(const CQuery *query) const; ///< Retrieves an error (if any) after executing a statement
    virtual void queryAllocStmt(CQuery *query);     ///< Allocates an ODBC statement
    virtual void queryFreeStmt(CQuery *query);      ///< Deallocates an ODBC statement
    virtual void queryCloseStmt(CQuery *query);     ///< Closes an ODBC statement
    virtual void queryPrepare(CQuery *query);       ///< Prepares a query if supported by database
    virtual void queryUnprepare(CQuery *query);     ///< Unprepares a query if supported by database
    virtual void queryExecute(CQuery *query);       ///< Executes a statement
    virtual int  queryColCount(CQuery *query);      ///< Counts columns of the dataset (if any) returned by query
    virtual void queryColAttributes(CQuery *query,int16_t column,int16_t descType,int32_t& value); ///< In a dataset returned by a query, retrieves the column attributes
    virtual void queryColAttributes(CQuery *query,int16_t column,int16_t descType,char *buff,int len); ///< In a dataset returned by a query, retrieves the column attributes
    virtual void queryBindParameters(CQuery *query); ///< Binds the parameters to the query
    virtual void queryOpen(CQuery *query);           ///< Opens the query for reading data from the query' recordset
    virtual void queryFetch(CQuery *query);          ///< Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.

    static  void ODBCtypeToCType(int odbcType,int32_t &ctype,CVariantType& dataType); ///< Converts the native ODBC type into SPTK data type

    /// Returns the ODBC connection object
    CODBCConnection *connection() {
        return m_connect;
    }

public:

    /// Constructor
    /// @param connectionString std::string, the ODBC connection string
    CODBCDatabase(std::string connectionString = "");

    /// Destructor
    virtual ~CODBCDatabase();

    /// Opens the database connection. If unsuccessful throws an exception.
    /// @param connectionString std::string, the ODBC connection string
    virtual void openDatabase(std::string connectionString = "") throw(CException);

    /// Closes the database connection. If unsuccessful throws an exception.
    virtual void closeDatabase() throw(CException);

    /// Retruns true if database is opened
    virtual bool active() const;

    /// Returns the database connection handle
    virtual void* handle() const;

    /// Returns the ODBC connection string for the active connection
    virtual std::string connectString() const;

    /// Returns the ODBC driver description for the active connection
    virtual std::string driverDescription() const;

    /// @brief Lists database objects
    /// @param objectType CDbObjectType, object type to list
    /// @param objects CStrings&, object list (output)
    virtual void objectList(CDbObjectType objectType,CStrings& objects) throw(std::exception);
};
/// @}
}
#endif

#endif
