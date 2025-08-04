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

#include <memory>
#include <sptk5/db/PoolDatabaseConnection.h>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{SPTK_OracleConnection
 */
class DatabaseConnectionPool;

/**
 * @brief Wrapper for CDatabase connection that automatically handles connection create and release
 */
class SP_EXPORT AutoDatabaseConnection
{
public:
    /**
     * @brief Constructor.
     * Automatically gets connection from the connection pool.
     * @param connectionPool    Database connection pool.
     */
    explicit AutoDatabaseConnection(DatabaseConnectionPool& connectionPool);

    AutoDatabaseConnection(const AutoDatabaseConnection&) = delete;
    AutoDatabaseConnection(AutoDatabaseConnection&&) = default;

    /**
     * @brief Destructor.
     * Releases connection to connection pool
     */
    virtual ~AutoDatabaseConnection();

    AutoDatabaseConnection& operator=(const AutoDatabaseConnection&) = delete;
    AutoDatabaseConnection& operator=(AutoDatabaseConnection&&) = delete;

    /**
     * @brief Returns database connection acquired from the connection pool.
     */
    [[nodiscard]] PoolDatabaseConnection* connection() const;

    /**
     * @brief Opens the database connection.
     *
     * If unsuccessful throws an exception.
     * @param connectionString  The ODBC connection string.
     */
    void open(const String& connectionString = "") const
    {
        if (!m_connection)
            throw Exception("DB driver not loaded");

        m_connection->open(connectionString);
    }

    /**
     * @brief Closes the database connection. If the connection was not successful, throws an exception.
     */
    void close() const
    {
        m_connection->close();
    }

    /**
     * @brief Returns true if the database is opened.
     */
    [[nodiscard]] bool active() const
    {
        return m_connection != nullptr && m_connection->active();
    }

    /**
     * @brief Returns the connection string.
     */
    [[nodiscard]] const DatabaseConnectionString& connectionString() const
    {
        return m_connection->connectionString();
    }

    /**
     * @brief Returns the connection type.
     */
    [[nodiscard]] DatabaseConnectionType connectionType() const
    {
        return m_connection->connectionType();
    }

    /**
     * @brief Returns the driver description.
     */
    [[nodiscard]] String driverDescription() const
    {
        return m_connection->driverDescription();
    }

    /**
     * @brief Begins the transaction.
     */
    void beginTransaction() const
    {
        m_connection->beginTransaction();
    }

    /**
     * @brief Commits the transaction.
     */
    void commitTransaction() const
    {
        m_connection->commitTransaction();
    }

    /**
     * @brief Rolls back the transaction.
     */
    void rollbackTransaction() const
    {
        m_connection->rollbackTransaction();
    }

    /**
     * @brief Lists database objects.
     *
     * Not implemented in DatabaseConnection. The derived database class
     * must provide its own implementation.
     * @param objectType        Object type to list.
     * @param objects           Object list (output).
     */
    void objectList(DatabaseObjectType objectType, Strings& objects) const
    {
        m_connection->objectList(objectType, objects);
    }

    /**
     * @brief Executes bulk inserts of data from the vector of rows.
     *
     * Data is inserted the fastest possible way.
     * @param tableName         Table name to insert into.
     * @param autoIncrementColumnName The key column that should be auto-incremental, or empty string.
     * @param columnNames       List of table columns to populate.
     * @param data              Data for bulk insert.
     * @param groupSize         Number of records inserted at once.
     * @param insertedIds       Optional vector of inserted autoincrement ids (if keyColumnName isn't empty).
     */
    virtual void bulkInsert(const String& tableName, const String& autoIncrementColumnName, const Strings& columnNames,
                            std::vector<VariantVector>& data, size_t groupSize = 100, std::vector<int64_t>* insertedIds = nullptr) const
    {
        m_connection->bulkInsert(tableName, autoIncrementColumnName, columnNames, data, groupSize, insertedIds);
    }

    /**
     * @brief Executes bulk delete of rows by the keys.
     * @param tableName         Table name to insert into.
     * @param keyColumnName     List of table columns to populate.
     * @param keys              Data for bulk insert.
     */
    void bulkDelete(const String& tableName, const String& keyColumnName, const VariantVector& keys) const
    {
        m_connection->bulkDelete(tableName, keyColumnName, keys);
    }

    /**
     * @brief Executes SQL batch file.
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchFileName     SQL batch file.
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions.
     */
    [[maybe_unused]] void executeBatchFile(const String& batchFileName, Strings* errors = nullptr) const
    {
        m_connection->executeBatchFile(batchFileName, errors);
    }

    /**
     * @brief Executes SQL batch queries.
     *
     * The queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL          SQL batch file.
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions.
     */
    void executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors = nullptr) const
    {
        m_connection->executeBatchSQL(batchSQL, errors);
    }

    [[nodiscard]] String tableSequenceName(const String& tableName, const String& /*sequenceName*/ = "") const
    {
        return m_connection->tableSequenceName(tableName);
    }

    [[nodiscard]] String lastAutoIncrementSql(const String& tableName, const String& /*sequenceName*/ = "") const
    {
        return m_connection->lastAutoIncrementSql(tableName);
    }

private:
    DatabaseConnectionPool& m_connectionPool;       ///< Database connection pool
    SPoolDatabaseConnection m_connection {nullptr}; ///< Database connection
};

using DatabaseConnection = std::shared_ptr<AutoDatabaseConnection>;

/**
 * @}
 */
} // namespace sptk
