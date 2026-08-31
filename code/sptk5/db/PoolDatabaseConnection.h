/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/Variant.h>
#include <sptk5/db/DatabaseConnectionString.h>

#include <vector>

namespace sptk {
/**
 * @addtogroup Database Database Support.
 * @{
 */

class Query;

using DBHandle = uint8_t*;
using StmtHandle = uint8_t*;
using SStmtHandle = std::shared_ptr<uint8_t>;

/**
 * @brief Database connection type.
 */
enum class DatabaseConnectionType : uint8_t
{
    MYSQL = 1,         ///< MySQL.
    ORACLE = 2,        ///< Oracle.
    POSTGRES = 4,      ///< PostgreSQL.
    SQLITE3 = 8,       ///< SQLite3.
    FIREBIRD = 16,     ///< Firebird.
    GENERIC_ODBC = 32, ///< Generic ODBC.
    MSSQL_ODBC = 64,   ///< MS SQL ODBC.
    ORACLE_OCI = 128,  ///< Oracle OCILib.
};

/**
 * @brief Types of the objects for the DatabaseConnection::listObjects method.
 */
enum class DatabaseObjectType : uint8_t
{
    TABLES,     ///< Tables.
    VIEWS,      ///< Views.
    PROCEDURES, ///< Stored procedures.
    FUNCTIONS,  ///< Stored functions.
    DATABASES   ///< Available databases.
};

class SP_EXPORT PoolDatabaseConnectionQueryMethods
{
    friend class Query;

    friend class QueryStatementManagement;

public:
    virtual ~PoolDatabaseConnectionQueryMethods() = default;

protected:
    /**
     * @brief Sets internal CQuery statement handle.
     */
    void querySetStmt(Query* query, const SStmtHandle& stmt);

    /**
     * @brief Sets internal CQuery m_prepared flag.
     */
    static void querySetPrepared(Query* query, bool isPrepared);

    /**
     * @brief Sets internal CQuery m_active flag.
     */
    static void querySetActive(Query* query, bool isActive);

    /**
     * @brief Sets internal CQuery m_eof flag.
     */
    static void querySetEof(Query* query, bool isEof);

    // These methods implement the actions requested by Query

    /**
     * @brief Retrieves an error (if any) after executing a statement.
     */
    virtual String queryError(const Query* query) const = 0;

    /**
     * @brief Allocates an ODBC statement.
     */
    virtual void queryAllocStmt(Query* query) = 0;

    /**
     * @brief Deallocates an ODBC statement.
     */
    virtual void queryFreeStmt(Query* query) = 0;

    /**
     * @brief Closes an ODBC statement.
     */
    virtual void queryCloseStmt(Query* query) = 0;

    /**
     * @brief Prepares a query if supported by the database.
     */
    virtual void queryPrepare(Query* query) = 0;

    /**
     * @brief Executes a statement.
     */
    virtual void queryExecute(Query* query) = 0;

    /**
     * @brief Counts columns of the dataset (if any) returned by the query.
     */
    virtual size_t queryColCount(Query* query) = 0;

    /**
     * @brief In a dataset returned by a query, retrieves the column attributes.
     */
    virtual void queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value) = 0;

    /**
     * @brief In a dataset returned by a query, retrieves the column attributes.
     */
    virtual void queryColAttributes(Query* query, int16_t column, int16_t descType, char* buff, int len) = 0;

    /**
     * @brief Binds the parameters to the query.
     */
    virtual void queryBindParameters(Query* query) = 0;

    /**
     * @brief Opens the query for reading data from the query's recordset.
     */
    virtual void queryOpen(Query* query) = 0;

    /**
     * @brief Reads data from the query's recordset into fields and advances to the next row. After reading the last row sets the EOF (end of the file, or no more data) flag.
     */
    virtual void queryFetch(Query* query) = 0;

    /**
     * @brief Returns parameter mark.
     *
     * @brief Parameter mark is generated from the parameterIndex.
     * @param paramIndex unsigned, parameter index in SQL starting from 0.
     */
    virtual std::string paramMark(unsigned paramIndex);

    /**
     * @brief Stub function to throw an exception in case if the called method isn't implemented in the derived class.
     */
    [[noreturn]] static void notImplemented(const String& methodName);

    /**
     * @brief Attaches (links) the query to the database.
     */
    bool linkQuery(Query* query);

    /**
     * @brief Unlinks the query from the database.
     */
    bool unlinkQuery(Query* query);

    /**
     * @brief Close all queries connected to this connection, free their statements, and the empty connected query list.
     */
    void disconnectAllQueries();

private:
    std::map<Query*, SStmtHandle> m_queryList; ///< The list of queries that use this database.
};

/**
 * @brief Database connector.
 *
 * Implements a connection to a generic database.
 * It is used as a base class for actual database driver classes.
 */
class SP_EXPORT PoolDatabaseConnection
    : public PoolDatabaseConnectionQueryMethods
    , public std::enable_shared_from_this<PoolDatabaseConnection>
{
    friend class Query;

    friend class QueryStatementManagement;

public:
    /**
     * @brief Destructor.
     */
    ~PoolDatabaseConnection() override;

    /**
     * @brief Copy constructor is deleted.
     */
    PoolDatabaseConnection(const PoolDatabaseConnection&) = delete;

    /**
     * @brief Move constructor.
     */
    PoolDatabaseConnection(PoolDatabaseConnection&&) noexcept = default;

    /**
     * @brief Copy assignment is deleted.
     */
    PoolDatabaseConnection& operator=(const PoolDatabaseConnection&) = delete;

    /**
     * @brief Move assignment.
     */
    PoolDatabaseConnection& operator=(PoolDatabaseConnection&&) noexcept = default;

    /**
     * @brief Opens the database connection.
     *
     * If unsuccessful, throws an exception.
     * @param newConnectionString  The ODBC connection string.
     */
    void open(const String& newConnectionString = "");

    /**
     * @brief Closes the database connection. If unsuccessful, throws an exception.
     */
    void close();

    /**
     * @brief Returns true if the database is opened.
     */
    [[nodiscard]] virtual bool active() const;

    /**
     * @brief Returns the database connection handle.
     */
    [[nodiscard]] virtual DBHandle handle() const;

    /**
     * @brief Returns the connection string.
     */
    [[nodiscard]] const DatabaseConnectionString& connectionString() const
    {
        return m_connString;
    }

    /**
     * @brief Set the connection string.
     * @param connectionString  Connection string.
     */
    void connectionString(const DatabaseConnectionString& connectionString);

    /**
     * @brief Returns driver-specific connection string.
     */
    [[nodiscard]] virtual String nativeConnectionString() const
    {
        return "";
    }

    /**
     * @brief Returns the connection type.
     */
    [[nodiscard]] virtual DatabaseConnectionType connectionType() const
    {
        return m_connType;
    }

    /**
     * @brief Whether this connection's server understands a RETURNING clause on INSERT.
     *
     * Asked rather than assumed from the connection type, because one type is not one capability:
     * SQLite gained RETURNING in 3.35, and Enterprise Linux 9 ships 3.34.1 and has nothing newer in
     * any repository. Until this was asked, InsertQuery appended RETURNING to every SQLite insert
     * and every insert that wanted its generated id back failed there with a syntax error.
     *
     * Answered by the driver, which is the only part that knows what its library can do. False here
     * so that a driver saying nothing is assumed not to support it - the safe direction, since the
     * caller then uses lastInsertId() and gets a correct answer either way.
     */
    [[nodiscard]] virtual bool supportsReturning() const
    {
        return false;
    }

    /**
     * @brief The generated key of the last row this connection inserted.
     *
     * The way back when supportsReturning() is false. Per connection and per statement: it must be
     * read before anything else is inserted on the same connection, which holding the connection
     * for the duration of a query already arranges.
     *
     * @throws sptk::Exception when the driver has no way to answer.
     */
    [[nodiscard]] virtual int64_t lastInsertId() const
    {
        throw Exception("This driver cannot report the id of the last inserted row");
    }

    /**
     * @brief Returns the driver description.
     */
    [[nodiscard]] virtual String driverDescription() const
    {
        return m_driverDescription;
    }

    /**
     * @brief Begins the transaction.
     */
    void beginTransaction();

    /**
     * @brief Commits the transaction.
     */
    void commitTransaction();

    /**
     * @brief Rolls back the transaction.
     */
    void rollbackTransaction();

    /**
     * @brief Reports true if in transaction.
     */
    [[nodiscard]] bool inTransaction() const
    {
        return m_inTransaction;
    }

    /**
     * @brief Lists database objects.
     *
     * Not implemented in DatabaseConnection. The derived database class must provide its own implementation.
     * @param objectType        Object type to list.
     * @param objects           Object list (output).
     */
    virtual void objectList(DatabaseObjectType objectType, Strings& objects) = 0;

    /**
     * @brief Executes bulk inserts of data from the vector of rows.
     *
     * Data is inserted the fastest possible way. The rows must have the same number of columns as columnNames.
     * @param tableName         Table name to insert into.
     * @param autoIncrementColumnName The optional column name for autoincrement column, or empty string.
     * @param columnNames       List of table columns to populate.
     * @param data              Data for bulk insert.
     * @param groupSize         The number of records inserted at a time.
     * @param insertedIds       Output vector for the inserted autoincrement ids (if keyColumnName isn't empty).
     */
    virtual void bulkInsert(const String& tableName, const String& autoIncrementColumnName, const Strings& columnNames,
                            std::vector<VariantVector>& data, size_t groupSize, std::vector<int64_t>& insertedIds)
    {
        bulkInsert(tableName, autoIncrementColumnName, columnNames, data, groupSize, &insertedIds);
    }

    /**
     * @brief Executes bulk inserts of data from the vector of rows.
     *
     * Data is inserted the fastest possible way. The rows must have the same number of columns as columnNames.
     * @param tableName         Table name to insert into.
     * @param columnNames       List of table columns to populate.
     * @param data              Data for bulk insert.
     * @param groupSize         The number of records inserted at a time.
     */
    virtual void bulkInsert(const String& tableName, const Strings& columnNames,
                            std::vector<VariantVector>& data, size_t groupSize)
    {
        bulkInsert(tableName, "", columnNames, data, groupSize, nullptr);
    }

    /**
     * @brief Executes bulk delete of rows by the keys.
     *
     * Data is deleted the fastest possible way.
     * @param tableName         Table name to insert into.
     * @param keyColumnName     List of table columns to populate.
     * @param keys              Data for bulk insert.
     */
    virtual void bulkDelete(const String& tableName, const String& keyColumnName,
                            const VariantVector& keys);

    /**
     * @brief Executes SQL batch file.
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchFileName     SQL batch file.
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions.
     */
    virtual void executeBatchFile(const String& batchFileName, Strings* errors);

    /**
     * @brief Executes SQL batch queries.
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL          SQL batch file.
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions.
     */
    virtual void executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors);

    [[nodiscard]] virtual String tableSequenceName(const String& tableName);
    [[nodiscard]] virtual String lastAutoIncrementSql(const String& tableName);

protected:
    [[nodiscard]] bool getInTransaction() const;

    void setInTransaction(bool inTransaction);

    /**
     * @brief Constructor.
     *
     * Protected constructor prevents creating an instance of the DatabaseConnection. Instead, it is possible to create
     * an instance of derived classes.
     * @param connectionString  The connection string.
     * @param connectionType    The connection type.
     * @param connectTimeout    Connection timeout.
     */
    explicit PoolDatabaseConnection(const String& connectionString, DatabaseConnectionType connectionType, std::chrono::seconds connectTimeout);

    /**
     * @brief Opens the database connection.
     *
     * This method should be overwritten in derived classes.
     * @param newConnectionString  The ODBC connection string.
     */
    virtual void _openDatabase(const String& newConnectionString);

    /**
     * @brief Closes the database connection.
     *
     * This method should be overwritten in derived classes.
     */
    virtual void closeDatabase();

    /**
     * @brief Begins the transaction.
     *
     * This method should be implemented in the derived driver.
     */
    virtual void driverBeginTransaction();

    /**
     * @brief Ends the transaction.
     *
     * This method should be implemented in the derived driver.
     * @param commit            Commit if true, rollback if false.
     */
    virtual void driverEndTransaction(bool commit);

    /**
     * @brief Throws an exception.
     *
     * Before the exception is thrown, it is logged into the logfile (if the logfile is defined).
     * @param method            Method name where error has occured.
     * @param error             Error text.
     */
    [[noreturn]] static void logAndThrow(const String& method, const String& error);

    /**
     * @brief Set the connection type.
     */
    virtual void connectionType(DatabaseConnectionType connType)
    {
        m_connType = connType;
    }

    /**
     * @brief Return connection timeout.
     * @return connection timeout.
     */
    [[nodiscard]] std::chrono::seconds connectTimeout() const
    {
        return m_connectionTimeout;
    }

private:
    DatabaseConnectionString m_connString;            ///< The connection string.
    DatabaseConnectionType   m_connType;              ///< The connection type.
    String                   m_driverDescription;     ///< Driver description is filled by the particular driver.
    bool                     m_inTransaction {false}; ///< The in-transaction flag.
    std::chrono::seconds     m_connectionTimeout;     ///< Connection timeout.

    /**
     * @brief Executes bulk inserts of data from the vector of rows.
     *
     * Data is inserted the fastest possible way. The rows must have the same number of columns as columnNames.
     * @param tableName         Table name to insert into.
     * @param autoIncrementColumnName The optional column name for autoincrement column, or empty string.
     * @param columnNames       List of table columns to populate.
     * @param data              Data for bulk insert.
     * @param groupSize         The number of records inserted at a time.
     * @param insertedIds       Optional (output) vector for the inserted autoincrement ids (if keyColumnName isn't empty).
     */
    virtual void bulkInsert(const String& tableName, const String& autoIncrementColumnName, const Strings& columnNames,
                            std::vector<VariantVector>& data, size_t groupSize, std::vector<int64_t>* insertedIds);
};

using SPoolDatabaseConnection = std::shared_ptr<PoolDatabaseConnection>;
using WPoolDatabaseConnection = std::weak_ptr<PoolDatabaseConnection>;

/**
 * @brief Escape SQL string for bulk insert.
 * @param str                   String to escape.
 * @param tsv                   True if output data is TSV (tab-separated values).
 * @return                      Escaped string.
 */
SP_EXPORT String escapeSQLString(const String& str, bool tsv = false);

/**
 * @}
 */
} // namespace sptk
