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

#include <memory>
#include <sptk5/db/PoolDatabaseConnection.h>

namespace sptk {
/**
 * @addtogroup Database Database Support.
 * @{SPTK_OracleConnection.
 */
class DatabaseConnectionPool;

/**
 * @brief Wrapper for CDatabase connection that automatically handles connection create and release.
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
     * Releases connection to connection pool.
     */
    virtual ~AutoDatabaseConnection();

    AutoDatabaseConnection& operator=(const AutoDatabaseConnection&) = delete;
    AutoDatabaseConnection& operator=(AutoDatabaseConnection&&) = delete;

    /**
     * @brief Returns database connection acquired from the connection pool.
     */
    [[nodiscard]] WPoolDatabaseConnection connection() const;

    /**
     * @brief Opens the database connection.
     *
     * If unsuccessful, throws an exception.
     * @param connectionString  The ODBC connection string.
     */
    void open(const String& connectionString = "") const;

    /**
     * @brief Closes the database connection.
     */
    void close() const;

    /**
     * @brief Returns true if the database is opened.
     */
    [[nodiscard]] bool active() const;

    /**
     * @brief Returns the connection string.
     */
    [[nodiscard]] const DatabaseConnectionString& connectionString() const;

    /**
     * @brief Returns the connection type.
     */
    [[nodiscard]] DatabaseConnectionType connectionType() const;

    /**
     * @brief Returns the driver description.
     */
    [[nodiscard]] String driverDescription() const;

    /**
     * @brief Begins the transaction.
     */
    void beginTransaction() const;

    /**
     * @brief Commits the transaction.
     */
    void commitTransaction() const;

    /**
     * @brief Rolls back the transaction.
     */
    void rollbackTransaction() const;

    /**
     * @brief Lists database objects.
     *
     * Not implemented in DatabaseConnection. The derived database class
     * must provide its own implementation.
     * @param objectType        Object type to list.
     * @param objects           Object list (output).
     */
    void objectList(DatabaseObjectType objectType, Strings& objects) const;

    /**
     * @brief Executes bulk inserts of data from the vector of rows.
     *
     * Data is inserted the fastest possible way. The autoincrement ids inserted during the operation,
     * returned in the inserted ids vector.
     * @param tableName         Table name to insert into.
     * @param autoIncrementColumnName The key column that should be auto-incremental, or empty string.
     * @param columnNames       List of table columns to populate.
     * @param data              Data for bulk insert.
     * @param insertedIds       Optional vector of inserted autoincrement ids (if keyColumnName isn't empty).
     * @param groupSize         Number of records inserted at once.
     */
    void bulkInsert(const String& tableName, const String& autoIncrementColumnName, const Strings& columnNames,
                    std::vector<VariantVector>& data, std::vector<int64_t>& insertedIds, size_t groupSize = 100) const;

    /**
     * @brief Executes bulk inserts of data from the vector of rows.
     *
     * Data is inserted the fastest possible way.
     * @param tableName         Table name to insert into.
     * @param columnNames       List of table columns to populate.
     * @param data              Data for bulk insert.
     * @param groupSize         Number of records inserted at once.
     */
    void bulkInsert(const String& tableName, const Strings& columnNames,
                    std::vector<VariantVector>& data, size_t groupSize = 50) const;

    /**
     * @brief Executes bulk delete of rows by the keys.
     * @param tableName         Table name to insert into.
     * @param keyColumnName     List of table columns to populate.
     * @param keys              Data for bulk insert.
     */
    void bulkDelete(const String& tableName, const String& keyColumnName, const VariantVector& keys) const;

    /**
     * @brief Executes SQL batch file.
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchFileName     SQL batch file.
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions.
     */
    [[maybe_unused]] void executeBatchFile(const String& batchFileName, Strings* errors = nullptr) const;

    /**
     * @brief Executes SQL batch queries.
     *
     * The queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL          SQL batch file.
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions.
     */
    void executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors = nullptr) const;

    [[nodiscard]] String tableSequenceName(const String& tableName, const String& /*sequenceName*/ = "") const;

    [[nodiscard]] String lastAutoIncrementSql(const String& tableName, const String& /*sequenceName*/ = "") const;

private:
    DatabaseConnectionPool& m_connectionPool;           ///< Database connection pool.
    WPoolDatabaseConnection m_connection;               ///< Database connection.
    static const String     s_invalidConnectionMessage; ///< Error message for inactive connection.

    SPoolDatabaseConnection acquireConnection() const;
};

using DatabaseConnection = std::shared_ptr<AutoDatabaseConnection>;

/**
 * @}
 */
} // namespace sptk
