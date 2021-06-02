/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/db/PoolDatabaseConnection.h>
#include <mutex>

#if HAVE_ORACLE == 1

#include <sptk5/db/OracleStatement.h>
#include <sptk5/db/OracleEnvironment.h>
#include <src/drivers/Oracle/OracleBulkInsertQuery.h>
#include "DatabaseField.h"

namespace sptk
{

/**
 * @addtogroup Database Database Support
 * @{
 */

class OracleStatement;
class OracleBulkInsertQuery;

/**
 * @brief Oracle database
 *
 * COracleConnection is thread-safe connection to Oracle database.
 */
class SP_EXPORT OracleConnection: public PoolDatabaseConnection
{
    friend class Query;
    friend class OracleStatement;

public:
    typedef oracle::occi::Environment   Environment;
    typedef oracle::occi::Connection    Connection;
    typedef oracle::occi::Statement     Statement;
    typedef oracle::occi::Type          Type;
    typedef oracle::occi::Clob          Clob;
    typedef oracle::occi::Blob          Blob;

    /**
     * @brief Returns the Oracle connection object
     */
    Connection* connection() const
    {
        return m_connection;
    }

    /**
     * @brief Returns the Oracle connection object
     */
    Environment* environment() const
    {
        return m_environment.handle();
    }

    Statement* createStatement(const std::string& sql)
    {
        return m_connection->createStatement(sql);
    }

    Statement* createStatement()
    {
        return m_connection->createStatement();
    }

    /**
     * @brief Opens the database connection. If unsuccessful throws an exception.
     * @param connectionString  The Oracle connection string
     */
    void _openDatabase(const String& connectionString) override;

    /**
     * @brief Executes bulk inserts of data from memory buffer
     *
     * Data is inserted the fastest possible way. The server-specific format definition provides extra information
     * about data. If format is empty than default server-specific data format is used.
     * For instance, for PostgreSQL it is TAB-delimited data, with some escaped characters ('\\t', '\\n', '\\r') and "\\N" for NULLs.
     * @param fullTableName     Table name to insert into, with or without schema name
     * @param columnNames       List of table columns to populate
     * @param data              Data for bulk insert
     */
    void _bulkInsert(const String& fullTableName, const Strings& columnNames,
                     const std::vector<VariantVector>& data) override;

    /**
     * @brief Executes SQL batch file
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL          SQL batch file
     * @param errors            Errors during execution. If provided, then errors are stored here, instead of exceptions
     */
    void _executeBatchSQL(const Strings& batchSQL, Strings* errors) override;

    /**
     * @brief Constructor
     * @param connectionString  The Oracle connection string
     */
    explicit OracleConnection(const String& connectionString = "");

    /**
     * @brief Destructor
     */
    ~OracleConnection() override;

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
    void* handle() const override;

    /**
     * @brief Returns driver-specific connection string
     */
    String nativeConnectionString() const override;

    /**
     * @brief Returns the Oracle driver description for the active connection
     */
    String driverDescription() const override;

    /**
     * @brief Lists database objects
     * @param objectType        Object type to list
     * @param objects           Object list (output)
     */
    void objectList(DatabaseObjectType objectType, Strings& objects) override;

protected:

    /**
     * @brief Begins the transaction
     */
    void driverBeginTransaction() override;

    /**
     * @brief Ends the transaction
     * @param commit            commit if true, rollback if false
     */
    void driverEndTransaction(bool commit) override;

    // These methods implement the actions requested by CQuery
    /**
     * Retrieves an error (if any) after executing a statement
     */
    String queryError(const Query *query) const override;

    /**
     * Allocates an Oracle statement
     */
    void queryAllocStmt(Query *query) override;

    /**
     * Deallocates an Oracle statement
     */
    void queryFreeStmt(Query *query) override;

    /**
     * Closes an Oracle statement
     */
    void queryCloseStmt(Query *query) override;

    /**
     * Prepares a query if supported by database
     */
    void queryPrepare(Query *query) override;

    /**
     * Unprepares a query if supported by database
     */
    void queryUnprepare(Query *query) override;

    /**
     * Executes a statement
     */
    void queryExecute(Query *query) override;

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    int  queryColCount(Query *query) override;

    /**
     * Binds the parameters to the query
     */
    void queryBindParameters(Query *query) override;

    /**
     * Opens the query for reading data from the query' recordset
     */
    void queryOpen(Query *query) override;

    /**
     * Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.
     */
    void queryFetch(Query *query) override;


    /**
     * @brief Returns parameter mark
     *
     * Parameter mark is generated from the parameterIndex.
     * @param paramIndex        parameter index in SQL starting from 0
     */
    String paramMark(unsigned paramIndex) override;

private:

    mutable std::mutex          m_mutex;                ///< Mutex that protects access to data members
    OracleEnvironment           m_environment;          ///< Oracle connection environment
    Connection*                 m_connection {nullptr}; ///< Oracle database connection
    std::string                 m_lastError;            ///< Last error in this connection or query

    void executeMultipleStatements(const Strings& statements, Strings* errors);

    static void readTimestamp(oracle::occi::ResultSet* resultSet, sptk::DatabaseField* field, unsigned int columnIndex);

    static void readDate(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex);

    static void readBLOB(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex);

    static void readCLOB(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex);

    void setMaxParamSizes(const CParamVector& enumeratedParams, Statement* stmt,
                          const QueryColumnTypeSizeMap& columnTypeSizes) const;

    void createQueryFieldsFromMetadata(Query* query, oracle::occi::ResultSet* resultSet) const;

    void bulkInsertSingleRow(const Strings& columnNames, const QueryColumnTypeSizeVector& columnTypeSizeVector,
                             sptk::OracleBulkInsertQuery& insertQuery, const VariantVector& row) const;
};

/**
 * @brief Translates Oracle native type to CVariant type
 * @param oracleType        Oracle native type
 * @returns Variant type
 */
VariantType OracleTypeToVariantType(OracleConnection::Type oracleType, int scale);

/**
 * @brief Translates CVariant type to Oracle native type
 * @param dataType          CVariant type
 * @returns Oracle native type
 */
OracleConnection::Type VariantTypeToOracleType(VariantType dataType);

#define throwOracleException(description) { m_lastError = description; throwDatabaseException(m_lastError); }

/**
 * @}
 */
}

#endif

extern "C" {
    SP_DRIVER_EXPORT void* oracle_create_connection(const char* connectionString);
    SP_DRIVER_EXPORT void  oracle_destroy_connection(void* connection);
}

