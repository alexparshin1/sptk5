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

#include <mutex>
#include <sptk5/db/PoolDatabaseConnection.h>

#ifdef HAVE_POSTGRESQL

#include <libpq-fe.h>

#ifndef _WIN32

#include <list>
#include <netinet/in.h>

#endif

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

class PostgreSQLStatement;

/**
 * @brief PostgreSQL database
 *
 * CPostgreSQLConnection is thread-safe connection to PostgreSQL database.
 */
class SP_EXPORT PostgreSQLConnection
    : public PoolDatabaseConnection
{
    friend class Query;

public:
    enum class TimestampFormat : uint8_t
    {
        UNKNOWN,
        DOUBLE,
        INT64
    };

    /**
     * @brief Returns the PostgreSQL connection object
     */
    PGconn* connection()
    {
        return m_connect;
    }

    /**
     * @brief Converts datatype from PostgreSQL type to SPTK VariantType
     */
    static void postgreTypeToVariantType(PostgreSQLDataType postgreType, VariantDataType& dataType);

    /**
     * @brief Converts datatype from SPTK VariantType to PostgreSQL type
     */
    static void variantTypeToPostgreType(VariantDataType dataType, PostgreSQLDataType& postgreType, const String& paramName);

    /**
     * @brief Opens the database connection. If unsuccessful throws an exception.
     * @param newConnectionString  The PostgreSQL connection string
     */
    void _openDatabase(const String& newConnectionString) override;

    /**
     * @brief Executes SQL batch file
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL          SQL batch file
     * @param errors            If not nullptr, store errors here instead of exceptions
     */
    void executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors) override;

    /**
     * @brief Constructor
     *
     * Typical connection string is something like: "dbname='mydb' host='myhostname' port=5142" and so on.
     * For more information please refer to:
     * http://www.postgresql.org/docs/current/interactive/libpq-connect.html
     * If the connection string is empty then default database with the name equal to user name is used.
     * @param connectionString  The PostgreSQL connection string
     */
    explicit PostgreSQLConnection(const String& connectionString = "", std::chrono::seconds connectTimeout = std::chrono::seconds(60));

    /**
     * @brief Destructor
     */
    ~PostgreSQLConnection() override;

    /**
     * @brief Returns driver-specific connection string
     */
    String nativeConnectionString() const override;

    /**
     * @brief Closes the database connection. If unsuccessful throws an exception.
     */
    void closeDatabase() override;

    /**
     * @brief Returns true if database is opened
     */
    bool active() const override;

    /**
     * @brief Returns the database connection handle
     */
    DBHandle handle() const override;

    /**
     * @brief Returns the PostgreSQL driver description for the active connection
     */
    String driverDescription() const override;

    /**
     * @brief Lists database objects
     * @param objectType        Object type to list
     * @param objects           Object list (output)
     */
    void objectList(DatabaseObjectType objectType, Strings& objects) override;

    /**
     * @brief All active connections
     */
    static std::map<PostgreSQLConnection*, std::shared_ptr<PostgreSQLConnection>> s_postgresqlConnections;

protected:
    static Strings extractStatements(const Strings& sqlBatch);

    /**
     * @brief Begins the transaction
     */
    void driverBeginTransaction() override;

    /**
     * @brief Ends the transaction
     * @param commit            Rollback if false
     */
    void driverEndTransaction(bool commit) override;

    // These methods implement the actions requested by Query

    /**
     * Retrieves an error (if any) after executing a statement
     */
    String queryError(const Query* query) const override;

    /**
     * Allocates an PostgreSQL statement
     */
    void queryAllocStmt(Query* query) override;

    /**
     * Deallocates an PostgreSQL statement
     */
    void queryFreeStmt(Query* query) override;

    /**
     * Closes an PostgreSQL statement
     */
    void queryCloseStmt(Query* query) override;

    /**
     * Prepares a query if supported by database
     */
    void queryPrepare(Query* query) override;

    /**
     * Executes a statement
     */
    void queryExecute(Query*) override
    {
        // Not needed for PG driver
    }

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    size_t queryColCount(Query* query) override;

    /**
     * Binds the parameters to the query
     */
    void queryBindParameters(Query* query) override;

    /**
     * Opens the query for reading data from the query' recordset
     */
    void queryOpen(Query* query) override;

    /**
     * Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.
     */
    void queryFetch(Query* query) override;

    /**
     * @brief Returns parameter mark
     *
     * Parameter mark is generated from the parameterIndex.
     * @param paramIndex        Parameter index in SQL starting from 0
     */
    String paramMark(unsigned paramIndex) override;

    void queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value) override;
    void queryColAttributes(Query* query, int16_t column, int16_t descType, char* buff, int len) override;
    void queryExecDirect(const Query* query);

private:
    mutable std::mutex m_mutex;                                    ///< Mutex that protects access to data members
    PGconn* m_connect {nullptr};                                   ///< PostgreSQL database connection
    TimestampFormat m_timestampsFormat {TimestampFormat::UNKNOWN}; ///< Connection timestamp format
};

/**
 * @}
 */
} // namespace sptk

#endif

extern "C" {
SP_DRIVER_EXPORT [[maybe_unused]] void* postgresqlCreateConnection(const char* connectionString, size_t connectionTimeoutSeconds);
SP_DRIVER_EXPORT [[maybe_unused]] void postgresqlDestroyConnection(void* connection);
}
