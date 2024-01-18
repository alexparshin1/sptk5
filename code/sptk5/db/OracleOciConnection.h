/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include "PoolDatabaseConnection.h"
#include "ocilib.hpp"

#ifdef HAVE_ORACLE_OCI

#include <mutex>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * @brief Oracle (OCI) database connection
 */
class SP_EXPORT OracleOciConnection
    : public PoolDatabaseConnection
{
    friend class Query;
    friend class OracleOciStatement;

public:
    /**
     * @brief Constructor
     *
     * Typical connection string is something like: "dbname='mydb' host='myhostname' port=5142" and so on.
     * For more information please refer to:
     * http://www.postgresql.org/docs/current/interactive/libpq-connect.html
     * If the connection string is empty then default database with the name equal to user name is used.
     * @param connectionString  The OracleOci connection string
     */
    explicit OracleOciConnection(const String& connectionString = "", std::chrono::seconds connectTimeout = std::chrono::minutes(1));

    OracleOciConnection(const OracleOciConnection&) = delete;
    OracleOciConnection(OracleOciConnection&&) = delete;
    OracleOciConnection& operator=(const OracleOciConnection&) = delete;
    OracleOciConnection& operator=(OracleOciConnection&&) = delete;

    ~OracleOciConnection() override;

    /**
     * @brief Returns the OracleOci connection object
     */
    [[nodiscard]] ocilib::Connection* connection() const;

    /**
     * @brief Opens the database connection. If unsuccessful throws an exception.
     * @param connectionString  The OracleOci connection string
     */
    void _openDatabase(const String& connectionString) override;

    /**
     * @brief Closes the database connection. If unsuccessful throws an exception.
     */
    void closeDatabase() override;

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
     * @brief Returns true if database is opened
     */
    [[nodiscard]] bool active() const override;

    /**
     * @brief Returns the database connection handle
     */
    [[nodiscard]] DBHandle handle() const override;

    /**
     * @brief Returns the OracleOci driver description for the active connection
     */
    [[nodiscard]] String driverDescription() const override;

    /**
     * @brief Lists database objects
     * @param objectType        Object type to list
     * @param objects           Object list (output)
     */
    void objectList(DatabaseObjectType objectType, Strings& objects) override;

    [[nodiscard]] static VariantDataType OracleOciTypeToVariantType(ocilib::DataType oracleType, int scale);
    [[nodiscard]] static ocilib::DataType VariantTypeToOracleOciType(VariantDataType dataType);

    /**
     * @brief All active connections
     */
    static std::map<OracleOciConnection*, std::shared_ptr<OracleOciConnection>> s_oracleOciConnections;

protected:
    /**
     * @brief Begins the transaction
     */
    void driverBeginTransaction() override;

    /**
     * @brief Ends the transaction
     * @param commit            Rollback if false
     */
    void driverEndTransaction(bool commit) override;

    // These methods implement the actions requested by CQuery
    /**
     * Retrieves an error (if any) after executing a statement
     */
    [[nodiscard]] String queryError(const Query* query) const override;

    /**
     * Allocates an OracleOci statement
     */
    void queryAllocStmt(Query* query) override;

    /**
     * Deallocates an OracleOci statement
     */
    void queryFreeStmt(Query* query) override;

    /**
     * Closes an OracleOci statement
     */
    void queryCloseStmt(Query* query) override;

    /**
     * Prepares a query if supported by database
     */
    void queryPrepare(Query* query) override;

    /**
     * Executes a statement
     */
    void queryExecute(Query* query) override;

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    [[nodiscard]] size_t queryColCount(Query* query) override;

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
    void queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value) override;
    void queryColAttributes(Query* query, int16_t column, int16_t descType, char* buff, int len) override;
    /**
     * @brief Returns parameter mark
     *
     * Parameter mark is generated from the parameterIndex.
     * @param paramIndex        Parameter index in SQL starting from 0
     */
    [[nodiscard]] String paramMark(unsigned paramIndex) override;

private:
    std::shared_ptr<ocilib::Connection> m_connection; ///< OracleOci database connection
    mutable std::mutex m_mutex;                       ///< Mutex that protects access to data members

    void createQueryFieldsFromMetadata(Query* query, ocilib::Resultset resultSet);
};
/**
 * @}
 */
} // namespace sptk

#endif

extern "C" {
SP_DRIVER_EXPORT void* oracleoci_create_connection(const char* connectionString, size_t connectionTimeoutSeconds);
SP_DRIVER_EXPORT void oracleoci_destroy_connection(void* connection);
}
