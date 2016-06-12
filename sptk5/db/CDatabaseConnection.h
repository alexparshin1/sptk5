/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        CDatabaseConnection.h - description                   ║
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

#ifndef __CDATABASECONNECTION_H__
#define __CDATABASECONNECTION_H__

#include <sptk5/sptk.h>
#include <sptk5/CStrings.h>
#include <sptk5/db/CDatabaseConnectionString.h>
#include <sptk5/threads/CSynchronizedCode.h>
#include <sptk5/Variant.h>
#include <sptk5/Logger.h>

#include <vector>

namespace sptk {

/// @addtogroup Database Database Support
/// @{

class CQuery;

/// @brief Types of the objects for CDatabaseConnection::listObjects method
enum CDbObjectType
{
    DOT_TABLES,         ///< Tables
    DOT_VIEWS,          ///< Views
    DOT_PROCEDURES      ///< Stored procedures
};

/// @brief Column type and size structure
struct CColumnTypeSize
{
    VariantType    type;   ///< Column type
    size_t          length; ///< Column data size
};

/// @brief Vector of column type and size structures
typedef std::vector<CColumnTypeSize> CColumnTypeSizeVector;

/// @brief Map of column names to column type and size structures
typedef std::map<std::string,CColumnTypeSize> CColumnTypeSizeMap;

/// @brief Database connector
///
/// Implements a thread-safe connection to general database. It is used
/// as a base class for actual database driver classes.
class SP_EXPORT CDatabaseConnection: public CSynchronized
{
    typedef std::vector<CQuery*> CQueryVector;
    friend class CQuery;

public:
    /// @brief Database connection type
    enum Type {
        DCT_UNKNOWN=0,      ///< Unknown
        DCT_MYSQL=1,        ///< MySQL
        DCT_ORACLE=2,       ///< Oracle
        DCT_POSTGRES=4,     ///< PostgreSQL
        DCT_SQLITE3=8,      ///< SQLite3
        DCT_ODBC=16,        ///< Generic ODBC
        DCT_FIREBIRD=32     ///< Firebird
    };

protected:

    CQueryVector                m_queryList;      ///< The list of queries that use this database
    CDatabaseConnectionString   m_connString;     ///< The connection string
    Type                        m_connType;       ///< The connection type
    bool                        m_inTransaction;  ///< The in-transaction flag
    Logger*                     m_log;            ///< Log for the database events (optional)
    std::string                 m_objectName;     ///< Object name for logs and error messages

    /// @brief Attaches (links) query to the database
    bool linkQuery(CQuery *q);

    /// @brief Unlinks query from the database
    bool unlinkQuery(CQuery *q);

protected:
    // These methods get access to CQuery's protected members
    void querySetAutoPrep(CQuery *q, bool pf);       ///< Sets internal CQuery m_autoPrepare flag
    void querySetStmt(CQuery *q, void *stmt);        ///< Sets internal CQuery statement handle
    void querySetConn(CQuery *q, void *conn);        ///< Sets internal CQuery connection handle
    void querySetPrepared(CQuery *q, bool pf);       ///< Sets internal CQuery m_prepared flag
    void querySetActive(CQuery *q, bool af);         ///< Sets internal CQuery m_active flag
    void querySetEof(CQuery *q, bool eof);           ///< Sets internal CQuery m_eof flag

    // These methods implement the actions requested by CQuery
    virtual std::string queryError(const CQuery *query) const; ///< Retrieves an error (if any) after executing a statement
    virtual void queryAllocStmt(CQuery *query);      ///< Allocates an ODBC statement
    virtual void queryFreeStmt(CQuery *query);       ///< Deallocates an ODBC statement
    virtual void queryCloseStmt(CQuery *query);      ///< Closes an ODBC statement
    virtual void queryPrepare(CQuery *query);        ///< Prepares a query if supported by database
    virtual void queryUnprepare(CQuery *query);      ///< Unprepares a query if supported by database
    virtual void queryExecute(CQuery *query);        ///< Executes a statement
    virtual void queryExecDirect(CQuery *query) {}   ///< Executes unprepared statement
    virtual int  queryColCount(CQuery *query);       ///< Counts columns of the dataset (if any) returned by query
    virtual void queryColAttributes(CQuery *query, int16_t column, int16_t descType, int32_t& value); ///< In a dataset returned by a query, retrieves the column attributes
    virtual void queryColAttributes(CQuery *query, int16_t column, int16_t descType, char *buff, int len); ///< In a dataset returned by a query, retrieves the column attributes
    virtual void queryBindParameters(CQuery *query); ///< Binds the parameters to the query
    virtual void queryOpen(CQuery *query);           ///< Opens the query for reading data from the query' recordset
    virtual void queryFetch(CQuery *query);          ///< Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.

    /// @brief Returns parameter mark
    ///
    /// Parameter mark is generated from the parameterIndex.
    /// @param paramIndex unsigned, parameter index in SQL starting from 0
    virtual std::string paramMark(unsigned paramIndex);

protected:

    std::string m_driverDescription; ///< Driver description is filled by the particular driver.

    /// @brief Constructor
    ///
    /// Protected constructor prevents creating an instance of the
    /// CDatabaseConnection. Instead, it is possible to create an instance of derived
    /// classes.
    /// @param connectionString std::string, the connection string
    CDatabaseConnection(std::string connectionString);

    /// Stub function to throw an exception in case if the
    /// called method isn't implemented in the derived class
    void notImplemented(const char *methodName) const;

    void *queryHandle(CQuery *query) const;          ///< Retrieves internal query handle
    void queryHandle(CQuery *query, void *handle);    ///< Sets internal query handle

    /// @brief Opens the database connection.
    ///
    /// This method should be overwritten in derived classes
    /// @param connectionString std::string, the ODBC connection string
    virtual void openDatabase(std::string connectionString = "") THROWS_EXCEPTIONS;

    /// @brief Closes the database connection.
    ///
    /// This method should be overwritten in derived classes
    virtual void closeDatabase() THROWS_EXCEPTIONS;

    /// @brief Begins the transaction
    ///
    /// This method should be implemented in derived driver
    virtual void driverBeginTransaction() THROWS_EXCEPTIONS;

    /// @brief Ends the transaction
    ///
    /// This method should be implemented in derived driver
    /// @param commit bool, commit if true, rollback if false
    virtual void driverEndTransaction(bool commit) THROWS_EXCEPTIONS;

    /// @brief Throws an exception
    ///
    /// Before exception is thrown, it is logged into the logfile (if the logfile is defined)
    /// @param method std::string, method name where error has occured
    /// @param error std::string, error text
    void logAndThrow(std::string method, std::string error) THROWS_EXCEPTIONS;

public:
    /// @brief Destructor
    ///
    /// Closes the database connection and all the connected queries.
    /// Releases all the database resources allocated during the connection.
    virtual ~CDatabaseConnection();

    /// @brief Opens the database connection
    ///
    /// If unsuccessful throws an exception.
    /// @param connectionString std::string, the ODBC connection string
    void open(std::string connectionString = "") THROWS_EXCEPTIONS;

    /// @brief Closes the database connection. If unsuccessful throws an exception.
    void close() THROWS_EXCEPTIONS;

    /// @brief Returns true if database is opened
    virtual bool active() const;

    /// @brief Returns the database connection handle
    virtual void* handle() const;

    /// @brief Returns the connection string
    virtual const CDatabaseConnectionString& connectionString() const
    {
        return m_connString;
    }

    /// @brief Returns driver-specific connection string
    virtual std::string nativeConnectionString() const = 0;

    /// @brief Returns the connection type
    virtual Type connectionType() const
    {
        return m_connType;
    }

    /// @brief Returns the driver description
    virtual std::string driverDescription() const
    {
        return m_driverDescription;
    }

    /// @brief Begins the transaction
    void beginTransaction() THROWS_EXCEPTIONS;

    /// @brief Commits the transaction
    void commitTransaction() THROWS_EXCEPTIONS;

    /// @brief Rolls back the transaction
    void rollbackTransaction() THROWS_EXCEPTIONS;

    /// @brief Reports true if in transaction
    int inTransaction()
    {
        return m_inTransaction;
    }

    /// @brief Lists database objects
    ///
    /// Not implemented in CDatabaseConnection. The derived database class
    /// must provide its own implementation
    /// @param objectType CDbObjectType, object type to list
    /// @param objects CStrings&, object list (output)
    virtual void objectList(CDbObjectType objectType, CStrings& objects) THROWS_EXCEPTIONS = 0;

    /// @brief Sets a log file for the database operations.
    ///
    /// If the database log is set, the database would log the events in CDatabaseConnection and CQuery objects
    /// into this log. To stop the logging, set the logFile parameter to NULL, or deactivate the log.
    /// @param logFile Logger *, the log file object to use.
    void logFile(Logger *logFile);

    /// @brief Returns a log file for the database operations.
    /// @returns current log file ptr, ot NULL if log file isn't set
    Logger *logFile();

    /// @brief Executes bulk inserts of data from memory buffer
    ///
    /// Data is inserted the fastest possible way. The server-specific format definition provides extra information
    /// about data. If format is empty than default server-specific data format is used.
    /// For instance, for PostgreSQL it is TAB-delimited data, with some escaped characters ('\\t', '\\n', '\\r') and "\\N" for NULLs.
    /// @param tableName std::string, table name to insert into
    /// @param columnNames const CStrings&, list of table columns to populate
    /// @param data const CStrings&, data for bulk insert
    /// @param format std::string, data format (may be database-specific). The default is TAB-delimited data.
    virtual void bulkInsert(std::string tableName, const CStrings& columnNames, const CStrings& data, std::string format="") THROWS_EXCEPTIONS;

    /// @brief Executes SQL batch file
    ///
    /// Queries are executed in not prepared mode.
    /// Syntax of the SQL batch file is matching the native for the database.
    /// @param batchFile std::string, SQL batch file
    virtual void executeBatchFile(std::string batchFile) THROWS_EXCEPTIONS;
};
/// @}
}
#endif
