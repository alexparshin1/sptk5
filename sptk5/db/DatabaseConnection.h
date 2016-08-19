/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        DatabaseConnection.h - description                    ║
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

#ifndef __SPTK_DATABASECONNECTION_H__
#define __SPTK_DATABASECONNECTION_H__

#include <sptk5/sptk.h>
#include <sptk5/Strings.h>
#include <sptk5/db/DatabaseConnectionString.h>
#include <sptk5/threads/SynchronizedCode.h>
#include <sptk5/Variant.h>
#include <sptk5/Logger.h>

#include <vector>

namespace sptk {

/// @addtogroup Database Database Support
/// @{

class Query;

/// @brief Types of the objects for DatabaseConnection::listObjects method
enum DatabaseObjectType
{
    DOT_TABLES,         ///< Tables
    DOT_VIEWS,          ///< Views
    DOT_PROCEDURES      ///< Stored procedures
};

/// @brief Column type and size structure
struct QueryColumnTypeSize
{
    VariantType     type;   ///< Column type
    size_t          length; ///< Column data size
};

/// @brief Vector of column type and size structures
typedef std::vector<QueryColumnTypeSize> QueryColumnTypeSizeVector;

/// @brief Map of column names to column type and size structures
typedef std::map<std::string,QueryColumnTypeSize> QueryColumnTypeSizeMap;

/// @brief Database connector
///
/// Implements a thread-safe connection to general database. It is used
/// as a base class for actual database driver classes.
class SP_EXPORT DatabaseConnection: public Synchronized
{
    typedef std::vector<Query*> CQueryVector;
    friend class Query;

public:
    /// @brief Database connection type
    enum Type {
        DCT_UNKNOWN=0,      ///< Unknown
        DCT_MYSQL=1,        ///< MySQL
        DCT_ORACLE=2,       ///< Oracle
        DCT_POSTGRES=4,     ///< PostgreSQL
        DCT_SQLITE3=8,      ///< SQLite3
        DCT_FIREBIRD=16,    ///< Firebird
        DCT_GENERIC_ODBC=32,///< Generic ODBC
        DCT_MSSQL_ODBC=64   ///< Generic ODBC
    };

protected:

    CQueryVector                m_queryList;      ///< The list of queries that use this database
    DatabaseConnectionString    m_connString;     ///< The connection string
    Type                        m_connType;       ///< The connection type
    bool                        m_inTransaction;  ///< The in-transaction flag
    Logger*                     m_log;            ///< Log for the database events (optional)
    std::string                 m_objectName;     ///< Object name for logs and error messages

    /// @brief Attaches (links) query to the database
    bool linkQuery(Query *q);

    /// @brief Unlinks query from the database
    bool unlinkQuery(Query *q);

protected:
    // These methods get access to CQuery's protected members
    void querySetAutoPrep(Query *q, bool pf);       ///< Sets internal CQuery m_autoPrepare flag
    void querySetStmt(Query *q, void *stmt);        ///< Sets internal CQuery statement handle
    void querySetConn(Query *q, void *conn);        ///< Sets internal CQuery connection handle
    void querySetPrepared(Query *q, bool pf);       ///< Sets internal CQuery m_prepared flag
    void querySetActive(Query *q, bool af);         ///< Sets internal CQuery m_active flag
    void querySetEof(Query *q, bool eof);           ///< Sets internal CQuery m_eof flag

    // These methods implement the actions requested by CQuery
    virtual std::string queryError(const Query *query) const; ///< Retrieves an error (if any) after executing a statement
    virtual void queryAllocStmt(Query *query);      ///< Allocates an ODBC statement
    virtual void queryFreeStmt(Query *query);       ///< Deallocates an ODBC statement
    virtual void queryCloseStmt(Query *query);      ///< Closes an ODBC statement
    virtual void queryPrepare(Query *query);        ///< Prepares a query if supported by database
    virtual void queryUnprepare(Query *query);      ///< Unprepares a query if supported by database
    virtual void queryExecute(Query *query);        ///< Executes a statement
    virtual void queryExecDirect(Query *query) {}   ///< Executes unprepared statement
    virtual int  queryColCount(Query *query);       ///< Counts columns of the dataset (if any) returned by query
    virtual void queryColAttributes(Query *query, int16_t column, int16_t descType, int32_t& value); ///< In a dataset returned by a query, retrieves the column attributes
    virtual void queryColAttributes(Query *query, int16_t column, int16_t descType, char *buff, int len); ///< In a dataset returned by a query, retrieves the column attributes
    virtual void queryBindParameters(Query *query); ///< Binds the parameters to the query
    virtual void queryOpen(Query *query);           ///< Opens the query for reading data from the query' recordset
    virtual void queryFetch(Query *query);          ///< Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.

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
    /// DatabaseConnection. Instead, it is possible to create an instance of derived
    /// classes.
    /// @param connectionString std::string, the connection string
    DatabaseConnection(std::string connectionString);

    /// Stub function to throw an exception in case if the
    /// called method isn't implemented in the derived class
    void notImplemented(const char *methodName) const;

    void *queryHandle(Query *query) const;          ///< Retrieves internal query handle
    void queryHandle(Query *query, void *handle);    ///< Sets internal query handle

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
    virtual ~DatabaseConnection();

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
    virtual const DatabaseConnectionString& connectionString() const
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
    /// Not implemented in DatabaseConnection. The derived database class
    /// must provide its own implementation
    /// @param objectType CDbObjectType, object type to list
    /// @param objects Strings&, object list (output)
    virtual void objectList(DatabaseObjectType objectType, Strings& objects) THROWS_EXCEPTIONS = 0;

    /// @brief Sets a log file for the database operations.
    ///
    /// If the database log is set, the database would log the events in DatabaseConnection and CQuery objects
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
    /// @param columnNames const Strings&, list of table columns to populate
    /// @param data const Strings&, data for bulk insert
    /// @param format std::string, data format (may be database-specific). The default is TAB-delimited data.
    virtual void bulkInsert(std::string tableName, const Strings& columnNames, const Strings& data, std::string format="") THROWS_EXCEPTIONS;

    /// @brief Executes SQL batch file
    ///
    /// Queries are executed in not prepared mode.
    /// Syntax of the SQL batch file is matching the native for the database.
    /// @param batchFile const sptk::Strings&, SQL batch file
    virtual void executeBatchFile(const sptk::Strings& batchFile) THROWS_EXCEPTIONS;
};
/// @}
}
#endif
