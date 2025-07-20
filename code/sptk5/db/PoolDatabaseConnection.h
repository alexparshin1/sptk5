/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/Logger.h>
#include <sptk5/Strings.h>
#include <sptk5/Variant.h>
#include <sptk5/db/DatabaseConnectionString.h>

#include <vector>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

class Query;

using DBHandle = uint8_t*;
using StmtHandle = uint8_t*;
using SStmtHandle = std::shared_ptr<uint8_t>;

/**
 * Database connection type
 */
enum class DatabaseConnectionType : uint16_t
{
    MYSQL = 1,         ///< MySQL
    ORACLE = 2,        ///< Oracle
    POSTGRES = 4,      ///< PostgreSQL
    SQLITE3 = 8,       ///< SQLite3
    FIREBIRD = 16,     ///< Firebird
    GENERIC_ODBC = 32, ///< Generic ODBC
    MSSQL_ODBC = 64,   ///< MS SQL ODBC
    ORACLE_OCI = 128,  ///< Oracle OCILib
};

/**
 * Types of the objects for DatabaseConnection::listObjects method
 */
enum class DatabaseObjectType : uint8_t
{
    TABLES,     ///< Tables
    VIEWS,      ///< Views
    PROCEDURES, ///< Stored procedures
    FUNCTIONS,  ///< Stored functions
    DATABASES   ///< Available databases
};

class SP_EXPORT PoolDatabaseConnectionQueryMethods
{
    friend class Query;

    friend class QueryStatementManagement;

public:
    virtual ~PoolDatabaseConnectionQueryMethods() = default;

protected:
    /**
     * Sets internal CQuery statement handle
     */
    void querySetStmt(Query* query, const SStmtHandle& stmt);

    /**
     * Sets internal CQuery m_prepared flag
     */
    static void querySetPrepared(Query* query, bool isPrepared);

    /**
     * Sets internal CQuery m_active flag
     */
    static void querySetActive(Query* query, bool isActive);

    /**
     * Sets internal CQuery m_eof flag
     */
    static void querySetEof(Query* query, bool isEof);

    // These methods implement the actions requested by CQuery
    /**
     * Retrieves an error (if any) after executing a statement
     */
    virtual String queryError(const Query* query) const = 0;

    /**
     * Allocates an ODBC statement
     */
    virtual void queryAllocStmt(Query* query) = 0;

    /**
     * Deallocates an ODBC statement
     */
    virtual void queryFreeStmt(Query* query) = 0;

    /**
     * Closes an ODBC statement
     */
    virtual void queryCloseStmt(Query* query) = 0;

    /**
     * Prepares a query if supported by database
     */
    virtual void queryPrepare(Query* query) = 0;

    /**
     * Executes a statement
     */
    virtual void queryExecute(Query* query) = 0;

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    virtual size_t queryColCount(Query* query) = 0;

    /**
     * In a dataset returned by a query, retrieves the column attributes
     */
    virtual void queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value) = 0;

    /**
     * In a dataset returned by a query, retrieves the column attributes
     */
    virtual void queryColAttributes(Query* query, int16_t column, int16_t descType, char* buff, int len) = 0;

    /**
     * Binds the parameters to the query
     */
    virtual void queryBindParameters(Query* query) = 0;

    /**
     * Opens the query for reading data from the query's recordset
     */
    virtual void queryOpen(Query* query) = 0;

    /**
     * Reads data from the query's recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.
     */
    virtual void queryFetch(Query* query) = 0;

    /**
     * Returns parameter mark
     *
     * Parameter mark is generated from the parameterIndex.
     * @param paramIndex unsigned, parameter index in SQL starting from 0
     */
    virtual String paramMark(unsigned paramIndex);

    /**
     * Stub function to throw an exception in case if the
     * called method isn't implemented in the derived class
     */
    [[noreturn]] static void notImplemented(const String& methodName);

    /**
     * Attaches (links) query to the database
     */
    bool linkQuery(Query* query);

    /**
     * Unlinks query from the database
     */
    bool unlinkQuery(Query* query);

    /**
     * Close all queries, connected to this connection,
     * free their statements, and empty connected query
     * list.
     */
    void disconnectAllQueries();

private:
    std::map<Query*, SStmtHandle> m_queryList; ///< The list of queries that use this database
};

/**
 * Database connector
 *
 * Implements a thread-safe connection to generDOT_al database. It is used
 * as a base class for actual database driver classes.
 */
class SP_EXPORT PoolDatabaseConnection
    : public PoolDatabaseConnectionQueryMethods
{
    friend class Query;

    friend class QueryStatementManagement;

public:
    /**
     * @brief Destructor
     */
    ~PoolDatabaseConnection() override;

    /**
     * @brief Copy constructor is deleted
     */
    PoolDatabaseConnection(const PoolDatabaseConnection&) = delete;

    /**
     * @brief Move constructor is deleted
     */
    PoolDatabaseConnection(PoolDatabaseConnection&&) noexcept = default;

    /**
     * @brief Copy assignment is deleted
     */
    PoolDatabaseConnection& operator=(const PoolDatabaseConnection&) = delete;

    /**
     * @brief Move assignment is deleted
     */
    PoolDatabaseConnection& operator=(PoolDatabaseConnection&&) noexcept = default;

    /**
     * Opens the database connection
     *
     * If unsuccessful throws an exception.
     * @param newConnectionString  The ODBC connection string
     */
    void open(const String& newConnectionString = "");

    /**
     * Closes the database connection. If unsuccessful throws an exception.
     */
    void close();

    /**
     * Returns true if database is opened
     */
    [[nodiscard]] virtual bool active() const;

    /**
     * Returns the database connection handle
     */
    [[nodiscard]] virtual DBHandle handle() const;

    /**
     * Returns the connection string
     */
    [[nodiscard]] const DatabaseConnectionString& connectionString() const
    {
        return m_connString;
    }

    /**
     * Set connecting string
     * @param connectionString  Connection string
     */
    void connectionString(const DatabaseConnectionString& connectionString);

    /**
     * Returns driver-specific connection string
     */
    [[nodiscard]] virtual String nativeConnectionString() const
    {
        return "";
    }

    /**
     * Returns the connection type
     */
    [[nodiscard]] virtual DatabaseConnectionType connectionType() const
    {
        return m_connType;
    }

    /**
     * Returns the driver description
     */
    [[nodiscard]] virtual String driverDescription() const
    {
        return m_driverDescription;
    }

    /**
     * Begins the transaction
     */
    void beginTransaction();

    /**
     * Commits the transaction
     */
    void commitTransaction();

    /**
     * Rolls back the transaction
     */
    void rollbackTransaction();

    /**
     * Reports true if in transaction
     */
    [[nodiscard]] bool inTransaction() const
    {
        return m_inTransaction;
    }

    /**
     * Lists database objects
     *
     * Not implemented in DatabaseConnection. The derived database class
     * must provide its own implementation
     * @param objectType        Object type to list
     * @param objects           Object list (output)
     */
    virtual void objectList(DatabaseObjectType objectType, Strings& objects) = 0;

    /**
     * @brief Executes bulk inserts of data from vector of rows.
     *
     * Data is inserted the fastest possible way. The rows must have the same number of columns as columnNames.
     * @param tableName         Table name to insert into
     * @param columnNames       List of table columns to populate
     * @param data              Data for bulk insert
     * @return inserted ids (if keyColumnName isn't empty), or empty vector.
     */
    [[nodiscard]] virtual std::vector<uint64_t> bulkInsert(const String& tableName, const String& keyColumnName, const Strings& columnNames,
                                                           const std::vector<VariantVector>& data);

    /**
     * @brief Executes bulk delete of rows by the keys.
     *
     * Data is deleted the fastest possible way.
     * @param tableName         Table name to insert into
     * @param keyColumnName     List of table columns to populate
     * @param keys              Data for bulk insert
     */
    virtual void bulkDelete(const String& tableName, const String& keyColumnName,
                            const VariantVector& keys);

    /**
     * Executes SQL batch file
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchFileName     SQL batch file
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions
     */
    virtual void executeBatchFile(const String& batchFileName, Strings* errors);

    /**
     * @brief Executes SQL batch queries.
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL          SQL batch file
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions
     */
    virtual void executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors);

    [[nodiscard]] String lastAutoIncrementSql(const String& tableName, const String& sequenceName) const;

protected:
    [[nodiscard]] bool getInTransaction() const;

    void setInTransaction(bool inTransaction);

    /**
     * Constructor
     *
     * Protected constructor prevents creating an instance of the
     * DatabaseConnection. Instead, it is possible to create an instance of derived
     * classes.
     * @param connectionString  The connection string
     * @param connectionType    The connection type
     * @param connectTimeout    Connection timeout
     */
    explicit PoolDatabaseConnection(const String& connectionString, DatabaseConnectionType connectionType, std::chrono::seconds connectTimeout);

    /**
     * Opens the database connection.
     *
     * This method should be overwritten in derived classes
     * @param newConnectionString  The ODBC connection string
     */
    virtual void _openDatabase(const String& newConnectionString);

    /**
     * Closes the database connection.
     *
     * This method should be overwritten in derived classes
     */
    virtual void closeDatabase();

    /**
     * Begins the transaction
     *
     * This method should be implemented in derived driver
     */
    virtual void driverBeginTransaction();

    /**
     * Ends the transaction
     *
     * This method should be implemented in derived driver
     * @param commit            Commit if true, rollback if false
     */
    virtual void driverEndTransaction(bool commit);

    /**
     * Throws an exception
     *
     * Before exception is thrown, it is logged into the logfile (if the logfile is defined)
     * @param method            Method name where error has occured
     * @param error             Error text
     */
    [[noreturn]] static void logAndThrow(const String& method, const String& error);

    /**
     * Set the connection type
     */
    virtual void connectionType(DatabaseConnectionType connType)
    {
        m_connType = connType;
    }

    /**
     * Return connection timeout
     * @return connection timeout
     */
    [[nodiscard]] std::chrono::seconds connectTimeout() const
    {
        return m_connectionTimeout;
    }

private:
    DatabaseConnectionString m_connString;            ///< The connection string
    DatabaseConnectionType   m_connType;              ///< The connection type
    String                   m_driverDescription;     ///< Driver description is filled by the particular driver.
    bool                     m_inTransaction {false}; ///< The in-transaction flag
    std::chrono::seconds     m_connectionTimeout;     ///< Connection timeout
};

using SPoolDatabaseConnection = std::shared_ptr<PoolDatabaseConnection>;

/**
 * Escape SQL string for bulk insert
 * @param str                   String to escape
 * @param tsv                   True if output data is TSV (tab-separated values)
 * @return                      Escaped string
 */
SP_EXPORT String escapeSQLString(const String& str, bool tsv = false);

/**
 * @}
 */
} // namespace sptk
