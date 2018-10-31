/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        PoolDatabaseConnection.h - description                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday October 28 2018                                 ║
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

#ifndef __SPTK_DATABASE_CONNECTION_H__
#define __SPTK_DATABASE_CONNECTION_H__

#include <sptk5/sptk.h>
#include <sptk5/Strings.h>
#include <sptk5/db/DatabaseConnectionString.h>
#include <sptk5/Variant.h>
#include <sptk5/Logger.h>

#include <vector>
#include <mutex>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

class Query;

/**
 * @brief Database connection type
 */
enum DatabaseConnectionType : uint16_t {
    /**
     * Unknown
     */
    DCT_UNKNOWN=0,

    /**
     * MySQL
     */
    DCT_MYSQL=1,

    /**
     * Oracle
     */
    DCT_ORACLE=2,

    /**
     * PostgreSQL
     */
    DCT_POSTGRES=4,

    /**
     * SQLite3
     */
    DCT_SQLITE3=8,

    /**
     * Firebird
     */
    DCT_FIREBIRD=16,

    /**
     * Generic ODBC
     */
    DCT_GENERIC_ODBC=32,

    /**
     * MS SQL ODBC
     */
    DCT_MSSQL_ODBC=64

};

/**
 * @brief Types of the objects for DatabaseConnection::listObjects method
 */
enum DatabaseObjectType : uint8_t
{
    /**
     * Tables
     */
    DOT_TABLES,

    /**
     * Views
     */
    DOT_VIEWS,

    /**
     * Stored procedures
     */
    DOT_PROCEDURES,

    /**
     * Stored functions
     */
    DOT_FUNCTIONS,

    /**
     * Available databases
     */
    DOT_DATABASES
};

/**
 * @brief Column type and size structure
 */
struct QueryColumnTypeSize
{
    /**
     * Column type
     */
    VariantType     type;

    /**
     * Column data size
     */
    size_t          length;

};

/**
 * @brief Vector of column type and size structures
 */
typedef std::vector<QueryColumnTypeSize> QueryColumnTypeSizeVector;

/**
 * @brief Map of column names to column type and size structures
 */
typedef std::map<std::string,QueryColumnTypeSize> QueryColumnTypeSizeMap;

/**
 * @brief Database connector
 *
 * Implements a thread-safe connection to general database. It is used
 * as a base class for actual database driver classes.
 */
class SP_EXPORT PoolDatabaseConnection
{
    typedef std::vector<Query*> CQueryVector;
    friend class Query;

public:
    /**
    * Mutex that protects access to data memebers
    */
    mutable std::mutex    m_mutex;

protected:

    /**
     * The list of queries that use this database
     */
    CQueryVector                m_queryList;

    /**
     * The connection string
     */
    DatabaseConnectionString    m_connString;

    /**
     * The connection type
     */
    DatabaseConnectionType      m_connType;

    /**
     * The in-transaction flag
     */
    bool                        m_inTransaction;

    /**
     * Object name for logs and error messages
     */
    String                      m_objectName;


    /**
     * @brief Attaches (links) query to the database
     */
    bool linkQuery(Query* q);

    /**
     * @brief Unlinks query from the database
     */
    bool unlinkQuery(Query* q);

    /**
     * Sets internal CQuery m_autoPrepare flag
     */
    void querySetAutoPrep(Query* q, bool pf);

    /**
     * Sets internal CQuery statement handle
     */
    void querySetStmt(Query* q, void *stmt);

    /**
     * Sets internal CQuery connection handle
     */
    void querySetConn(Query* q, void *conn);

    /**
     * Sets internal CQuery m_prepared flag
     */
    void querySetPrepared(Query* q, bool pf);

    /**
     * Sets internal CQuery m_active flag
     */
    void querySetActive(Query* q, bool af);

    /**
     * Sets internal CQuery m_eof flag
     */
    void querySetEof(Query* q, bool eof);


    // These methods implement the actions requested by CQuery
    /**
     * Retrieves an error (if any) after executing a statement
     */
    virtual String queryError(const Query* query) const;

    /**
     * Allocates an ODBC statement
     */
    virtual void queryAllocStmt(Query* query);

    /**
     * Deallocates an ODBC statement
     */
    virtual void queryFreeStmt(Query* query);

    /**
     * Closes an ODBC statement
     */
    virtual void queryCloseStmt(Query* query);

    /**
     * Prepares a query if supported by database
     */
    virtual void queryPrepare(Query* query);

    /**
     * Unprepares a query if supported by database
     */
    virtual void queryUnprepare(Query* query);

    /**
     * Executes a statement
     */
    virtual void queryExecute(Query* query);

    /**
     * Executes unprepared statement
     */
    virtual void queryExecDirect(Query* query) {}

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    virtual int  queryColCount(Query* query);

    /**
     * In a dataset returned by a query, retrieves the column attributes
     */
    virtual void queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value);

    /**
     * In a dataset returned by a query, retrieves the column attributes
     */
    virtual void queryColAttributes(Query* query, int16_t column, int16_t descType, char *buff, int len);

    /**
     * Binds the parameters to the query
     */
    virtual void queryBindParameters(Query* query);

    /**
     * Opens the query for reading data from the query' recordset
     */
    virtual void queryOpen(Query* query);

    /**
     * Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.
     */
    virtual void queryFetch(Query* query);


    /**
     * @brief Returns parameter mark
     *
     * Parameter mark is generated from the parameterIndex.
     * @param paramIndex unsigned, parameter index in SQL starting from 0
     */
    virtual String paramMark(unsigned paramIndex);

    /**
     * Driver description is filled by the particular driver.
     */
    String  m_driverDescription;

    /**
     * @brief Constructor
     *
     * Protected constructor prevents creating an instance of the
     * DatabaseConnection. Instead, it is possible to create an instance of derived
     * classes.
     * @param connectionString  The connection string
     */
    explicit PoolDatabaseConnection(const String& connectionString);

    /**
     * Stub function to throw an exception in case if the
     * called method isn't implemented in the derived class
     */
    void notImplemented(const String& methodName) const;

    /**
     * Retrieves internal query handle
     */
    void *queryHandle(Query* query) const;

    /**
     * Sets internal query handle
     */
    void queryHandle(Query* query, void *handle);


    /**
     * @brief Opens the database connection.
     *
     * This method should be overwritten in derived classes
     * @param connectionString  The ODBC connection string
     */
    virtual void _openDatabase(const String& connectionString);

    /**
     * @brief Closes the database connection.
     *
     * This method should be overwritten in derived classes
     */
    virtual void closeDatabase();

    /**
     * @brief Begins the transaction
     *
     * This method should be implemented in derived driver
     */
    virtual void driverBeginTransaction();

    /**
     * @brief Ends the transaction
     *
     * This method should be implemented in derived driver
     * @param commit            Commit if true, rollback if false
     */
    virtual void driverEndTransaction(bool commit);

    /**
     * @brief Throws an exception
     *
     * Before exception is thrown, it is logged into the logfile (if the logfile is defined)
     * @param method            Method name where error has occured
     * @param error             Error text
     */
    void logAndThrow(const String& method, const String& error);

    /**
     * @brief Executes bulk inserts of data from memory buffer
     *
     * Data is inserted the fastest possible way. The server-specific format definition provides extra information
     * about data. If format is empty than default server-specific data format is used.
     * For instance, for PostgreSQL it is TAB-delimited data, with some escaped characters ('\\t', '\\n', '\\r') and "\\N" for NULLs.
     * @param tableName         Table name to insert into
     * @param columnNames       List of table columns to populate
     * @param data              Data for bulk insert
     * @param format            Data format (may be database-specific). The default is TAB-delimited data.
     */
    virtual void _bulkInsert(const String& tableName, const Strings& columnNames, const Strings& data,
                             const String& format);

    /**
     * @brief Executes SQL batch file
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchFileName     SQL batch file
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions
     */
    virtual void _executeBatchFile(const String& batchFileName, Strings* errors);

    /**
     * @brief Executes SQL batch queries
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL          SQL batch file
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions
     */
    virtual void _executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors);

public:

    /**
     * @brief Destructor
     *
     * Closes the database connection and all the connected queries.
     * Releases all the database resources allocated during the connection.
     */
    virtual ~PoolDatabaseConnection();

    /**
     * @brief Opens the database connection
     *
     * If unsuccessful throws an exception.
     * @param connectionString  The ODBC connection string
     */
    void open(const String& connectionString = "");

    /**
     * @brief Closes the database connection. If unsuccessful throws an exception.
     */
    void close();

    /**
     * @brief Returns true if database is opened
     */
    virtual bool active() const;

    /**
     * @brief Returns the database connection handle
     */
    virtual void* handle() const;

    /**
     * @brief Returns the connection string
     */
    virtual const DatabaseConnectionString& connectionString() const
    {
        return m_connString;
    }

    /**
     * @brief Returns driver-specific connection string
     */
    virtual String nativeConnectionString() const = 0;

    /**
     * @brief Returns the connection type
     */
    virtual DatabaseConnectionType connectionType() const
    {
        return m_connType;
    }

    /**
     * @brief Returns the driver description
     */
    virtual String driverDescription() const
    {
        return m_driverDescription;
    }

    /**
     * @brief Begins the transaction
     */
    void beginTransaction();

    /**
     * @brief Commits the transaction
     */
    void commitTransaction();

    /**
     * @brief Rolls back the transaction
     */
    void rollbackTransaction();

    /**
     * @brief Reports true if in transaction
     */
    int inTransaction()
    {
        return m_inTransaction;
    }

    /**
     * @brief Lists database objects
     *
     * Not implemented in DatabaseConnection. The derived database class
     * must provide its own implementation
     * @param objectType        Object type to list
     * @param objects           Object list (output)
     */
    virtual void objectList(DatabaseObjectType objectType, Strings& objects) = 0;

    /**
     * @brief Executes bulk inserts of data from memory buffer
     *
     * Data is inserted the fastest possible way. The server-specific format definition provides extra information
     * about data. If format is empty than default server-specific data format is used.
     * For instance, for PostgreSQL it is TAB-delimited data, with some escaped characters ('\\t', '\\n', '\\r') and "\\N" for NULLs.
     * @param tableName         Table name to insert into
     * @param columnNames       List of table columns to populate
     * @param data              Data for bulk insert
     * @param format            Data format (may be database-specific). The default is TAB-delimited data.
     */
    void bulkInsert(const String& tableName, const Strings& columnNames, const Strings& data, const String& format = "")
    {
        _bulkInsert(tableName, columnNames, data, format);
    }

    /**
     * @brief Executes SQL batch file
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchFileName     SQL batch file
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions
     */
    void executeBatchFile(const String& batchFileName, Strings* errors = NULL)
    {
        _executeBatchFile(batchFileName, errors);
    }

    /**
     * @brief Executes SQL batch queries
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL          SQL batch file
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions
     */
    void executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors = NULL)
    {
        _executeBatchSQL(batchSQL, errors);
    }
};
/**
 * @}
 */
}
#endif
