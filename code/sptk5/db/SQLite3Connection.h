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

#include <sptk5/sptk.h>

#ifdef HAVE_SQLITE3

#include "sptk5/threads/SynchronizedMap.h"


#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/db/QueryParameter.h>
#include <sqlite3.h>

namespace sptk {
/**
 * @addtogroup Database Database Support.
 * @{
 */

/**
 * @brief SQLite3 database.
 *
 * CSQLite3Connection is the thread-safe connection to SQLite3 database.
 */
class SP_EXPORT SQLite3Connection
    : public PoolDatabaseConnection
{
    friend class Query;

public:
    /**
     * @brief Constructor.
     * @param connectionString  The SQLite3 connection string.
     * @param connectTimeout    Connection timeout in seconds.
     */
    explicit SQLite3Connection(const String& connectionString = "", std::chrono::seconds connectTimeout = std::chrono::seconds(60));

    /**
     * @brief Destructor.
     */
    ~SQLite3Connection() override = default;

    /**
     * @brief Returns driver-specific connection string.
     */
    String nativeConnectionString() const override;

    /**
     * @brief Closes the database connection. If unsuccessful, throws an exception.
     */
    void closeDatabase() override;

    /**
     * @brief Returns true if the database is opened.
     */
    bool active() const override;

    /**
     * @brief Returns the database connection handle.
     */
    DBHandle handle() const override;

    /**
     * @brief Returns the SQLite3 driver description for the active connection.
     */
    String driverDescription() const override;

    /**
     * @brief Lists database objects.
     * @param objectType        Object type to list.
     * @param objects           Object list (output).
     */
    void objectList(DatabaseObjectType objectType, Strings& objects) override;

    /**
     * @brief All active connections.
     */
    static SynchronizedMap<SQLite3Connection*, std::shared_ptr<SQLite3Connection>> s_sqlite3Connections;

protected:
    /**
     * @brief Begins the transaction.
     */
    void driverBeginTransaction() override;

    /**
     * @brief Ends the transaction.
     * @param commit            Commit if true, rollback if false.
     */
    void driverEndTransaction(bool commit) override;

    // These methods implement the actions requested by CQuery
    /**
     * @brief Retrieves an error (if any) after executing a statement.
     */
    String queryError(const Query* query) const override;

    /**
     * @brief Allocates an SQLite3 statement.
     */
    void queryAllocStmt(Query* query) override;

    /**
     * @brief Deallocates an SQLite3 statement.
     */
    void queryFreeStmt(Query* query) override;

    /**
     * @brief Closes an SQLite3 statement.
     */
    void queryCloseStmt(Query* query) override;

    /**
     * @brief Prepares a query if supported by the database.
     */
    void queryPrepare(Query* query) override;

    /**
     * @brief Executes a statement.
     */
    void queryExecute(Query* query) override;
    void queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value) override;
    void queryColAttributes(Query* query, int16_t column, int16_t descType, char* buff, int len) override;
    /**
     * @brief Counts columns of the dataset (if any) returned by the query.
     */
    size_t queryColCount(Query* query) override;

    /**
     * @brief Binds the parameters to the query.
     */
    void queryBindParameters(Query* query) override;

    /**
     * @brief Opens the query for reading data from the query's recordset.
     */
    void queryOpen(Query* query) override;

    /**
     * @brief Reads data from the query's recordset into fields and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.
     */
    void queryFetch(Query* query) override;

    /**
     * @brief Returns the SQLite3 connection object.
     */
    sqlite3* connection() const
    {
        return m_connect.get();
    }

    /**
     * @brief Whether the SQLite this is linked against understands RETURNING.
     *
     * The clause arrived in 3.35.0. Asked of the library at run time rather than decided when SPTK
     * was built, because the answer belongs to whatever SQLite is loaded: Enterprise Linux 9 ships
     * 3.34.1 and offers nothing newer in any repository, while every other platform SPTK is built
     * on is well past it.
     */
    [[nodiscard]] bool supportsReturning() const override;

    /**
     * @brief The rowid of the last row inserted on this connection.
     *
     * What InsertQuery uses in place of RETURNING. Read straight from the connection rather than
     * through "SELECT last_insert_rowid()", which would cost a statement to learn the same thing.
     *
     * A trigger that inserts a row of its own moves this: SQLite reports the trigger's insert, not
     * the caller's. There are no triggers in anything SPTK creates, and this is the reason not to
     * add one lightly.
     */
    [[nodiscard]] int64_t lastInsertId() const override;

    /**
     * @brief Opens the database connection. If unsuccessful, throws an exception.
     * @param newConnectionString  The SQLite3 connection string.
     */
    void _openDatabase(const String& newConnectionString) override;

    /**
     * @brief Executes SQL batch file.
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL          SQL batch file.
     * @param errors            If not nullptr, store errors here instead of exceptions.
     */
    void executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors) override;

private:
    using SQLHSTMT = sqlite3_stmt*;

    mutable std::mutex       m_mutex;                     ///< Mutex that protects access to data members.
    std::shared_ptr<sqlite3> m_connect;                   ///< Database connection.
    std::chrono::minutes     m_sessionTimezoneOffset {0}; //< Session timezone offset

    void                 bindParameter(Query* query, uint32_t paramNumber);
    void                 closeAndClean();
    static int           transformDateTimeParameter(sqlite3_stmt* stmt, QueryParameter* param, short paramBindNumber, VariantDataType dataType);
    std::chrono::minutes getSessionTimezoneOffset(); //< Get session timezone offset

    static void setFieldToNull(Field* field, short sqliteFieldType);
};

/**
 * @}
 */
} // namespace sptk

#endif

extern "C" {
SP_DRIVER_EXPORT [[maybe_unused]] void* sqlite3CreateConnection(const char* connectionString, size_t connectionTimeoutSeconds);
SP_DRIVER_EXPORT [[maybe_unused]] void  sqlite3DestroyConnection(void* connection);
}
