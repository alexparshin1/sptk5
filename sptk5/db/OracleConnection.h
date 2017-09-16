/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        OracleConnection.h - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_ORACLECONNECTION_H__
#define __SPTK_ORACLECONNECTION_H__

#include <sptk5/db/DatabaseConnection.h>

#if HAVE_ORACLE == 1

#include <sptk5/db/OracleStatement.h>
#include <sptk5/db/OracleEnvironment.h>

namespace sptk
{

/**
 * @addtogroup Database Database Support
 * @{
 */

class Query;
class OracleStatement;

/**
 * @brief Oracle database
 *
 * COracleConnection is thread-safe connection to Oracle database.
 */
class SP_EXPORT OracleConnection: public DatabaseConnection
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

private:

    /**
     * Oracle connection environment
     */
    OracleEnvironment  m_environment;

    /**
     * Oracle database connection
     */
    Connection*         m_connection;

    /**
     * Last error in this connection or query
     */
    std::string         m_lastError;


protected:

    /**
     * @brief Translates Oracle native type to CVariant type
     * @param oracleType oracle::occi::Type, Oracle native type
     * @returns Variant type
     */
    static VariantType OracleTypeToVariantType(Type oracleType);

    /**
     * @brief Translates CVariant type to Oracle native type
     * @param dataType VariantType&, CVariant type
     * @returns Oracle native type
     */
    static Type VariantTypeToOracleType(VariantType dataType);

protected:

    /**
     * @brief Begins the transaction
     */
    virtual void driverBeginTransaction();

    /**
     * @brief Ends the transaction
     * @param commit bool, commit if true, rollback if false
     */
    virtual void driverEndTransaction(bool commit);

    // These methods implement the actions requested by CQuery
    /**
     * Retrieves an error (if any) after executing a statement
     */
    virtual std::string queryError(const Query *query) const;

    /**
     * Allocates an Oracle statement
     */
    virtual void queryAllocStmt(Query *query);

    /**
     * Deallocates an Oracle statement
     */
    virtual void queryFreeStmt(Query *query);

    /**
     * Closes an Oracle statement
     */
    virtual void queryCloseStmt(Query *query);

    /**
     * Prepares a query if supported by database
     */
    virtual void queryPrepare(Query *query);

    /**
     * Unprepares a query if supported by database
     */
    virtual void queryUnprepare(Query *query);

    /**
     * Executes a statement
     */
    virtual void queryExecute(Query *query);

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    virtual int  queryColCount(Query *query);

    /**
     * Binds the parameters to the query
     */
    virtual void queryBindParameters(Query *query);

    /**
     * Opens the query for reading data from the query' recordset
     */
    virtual void queryOpen(Query *query);

    /**
     * Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.
     */
    virtual void queryFetch(Query *query);


    /**
     * @brief Returns parameter mark
     *
     * Parameter mark is generated from the parameterIndex.
     * @param paramIndex unsigned, parameter index in SQL starting from 0
     */
    virtual std::string paramMark(unsigned paramIndex);

public:
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

    Statement* createStatement(std::string sql)
    {
        return m_connection->createStatement(sql);
    }

    Statement* createStatement()
    {
        return m_connection->createStatement();
    }

public:

    /**
     * @brief Constructor
     * @param connectionString const std::string&, the Oracle connection string
     */
    OracleConnection(const std::string& connectionString = "");

    /**
     * @brief Destructor
     */
    virtual ~OracleConnection();

    /**
     * @brief Opens the database connection. If unsuccessful throws an exception.
     * @param connectionString const std::string&, the Oracle connection string
     */
    virtual void openDatabase(const std::string& connectionString = "") override;

    /**
     * @brief Closes the database connection. If unsuccessful throws an exception.
     */
    virtual void closeDatabase();

    /**
     * @brief Returns true if database is opened
     */
    virtual bool active() const;

    /**
     * @brief Returns the database connection handle
     */
    virtual void* handle() const;

    /**
     * @brief Returns driver-specific connection string
     */
    virtual std::string nativeConnectionString() const;

    /**
     * @brief Returns the Oracle driver description for the active connection
     */
    virtual std::string driverDescription() const;

    /**
     * @brief Lists database objects
     * @param objectType CDbObjectType, object type to list
     * @param objects Strings&, object list (output)
     */
    virtual void objectList(DatabaseObjectType objectType, Strings& objects) override;

    /**
     * @brief Executes bulk inserts of data from memory buffer
     *
     * Data is inserted the fastest possible way. The server-specific format definition provides extra information
     * about data. If format is empty than default server-specific data format is used.
     * For instance, for PostgreSQL it is TAB-delimited data, with some escaped characters ('\\t', '\\n', '\\r') and "\\N" for NULLs.
     * @param tableName std::string, table name to insert into
     * @param columnNames const Strings&, list of table columns to populate
     * @param data const Strings&, data for bulk insert
     */
    virtual void bulkInsert(std::string tableName, const Strings& columnNames, const Strings& data, std::string format="");

    /**
     * @brief Executes SQL batch file
     *
     * Queries are executed in not prepared mode.
     * Syntax of the SQL batch file is matching the native for the database.
     * @param batchSQL const sptk::Strings&, SQL batch file
     * @param errors Strings*, Errors during execution. If provided, then errors are stored here, instead of exceptions
     */
    virtual void executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors=NULL) override;
};

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

#endif
