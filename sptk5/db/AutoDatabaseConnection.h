/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        AutoDatabaseConnection.h - description                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __SPTK_AUTO_DATABASE_CONNECTION_H__
#define __SPTK_AUTO_DATABASE_CONNECTION_H__

#include <sptk5/db/PoolDatabaseConnection.h>
#include <memory>

namespace sptk
{

/**
 * @addtogroup Database Database Support
 * @{
 */
class DatabaseConnectionPool;

/**
 * @brief Wrapper for CDatabase connection that automatically handles connection create and release
 */
class AutoDatabaseConnection
{
    /**
     * Database connection pool
     */
    DatabaseConnectionPool&     m_connectionPool;

    /**
     * Database connection
     */
    PoolDatabaseConnection*     m_connection;

public:

    /**
     * @brief Constructor
     * Automatically gets connection from connection pool
     * @param connectionPool    DatabaseConnectionPool&, Database connection pool
     */
    explicit AutoDatabaseConnection(DatabaseConnectionPool& connectionPool);

    /**
     * @brief Destructor
     * Releases connection to connection pool
     */
    ~AutoDatabaseConnection();

    /**
     * @brief Returns database connection acquired from the connection pool
     */
    PoolDatabaseConnection* connection();

    /**
     * @brief Opens the database connection
     *
     * If unsuccessful throws an exception.
     * @param connectionString  The ODBC connection string
     */
    void open(const String& connectionString = "")
    {
        m_connection->open(connectionString);
    }

    /**
     * @brief Closes the database connection. If unsuccessful throws an exception.
     */
    void close()
    {
        m_connection->close();
    }

    /**
     * @brief Returns true if database is opened
     */
    bool active() const
    {
        return m_connection->active();
    }

    /**
     * @brief Returns the connection string
     */
    const DatabaseConnectionString& connectionString() const
    {
        return m_connection->connectionString();
    }

    /**
     * @brief Returns driver-specific connection string
     */
    String nativeConnectionString() const
    {
        return m_connection->nativeConnectionString();
    }

    /**
     * @brief Returns the connection type
     */
    DatabaseConnectionType connectionType() const
    {
        return m_connection->connectionType();
    }

    /**
     * @brief Returns the driver description
     */
    String driverDescription() const
    {
        return m_connection->driverDescription();
    }

    /**
     * @brief Begins the transaction
     */
    void beginTransaction()
    {
        m_connection->beginTransaction();
    }

    /**
     * @brief Commits the transaction
     */
    void commitTransaction()
    {
        m_connection->commitTransaction();
    }

    /**
     * @brief Rolls back the transaction
     */
    void rollbackTransaction()
    {
        m_connection->rollbackTransaction();
    }

    /**
     * @brief Reports true if in transaction
     */
    int inTransaction()
    {
        return m_connection->inTransaction();
    }

    /**
     * @brief Lists database objects
     *
     * Not implemented in DatabaseConnection. The derived database class
     * must provide its own implementation
     * @param objectType        Object type to list
     * @param objects           Object list (output)
     */
    void objectList(DatabaseObjectType objectType, Strings& objects)
    {
        m_connection->objectList(objectType, objects);
    }

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
        m_connection->bulkInsert(tableName, columnNames, data, format);
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
        m_connection->executeBatchFile(batchFileName, errors);
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
        m_connection->executeBatchSQL(batchSQL, errors);
    }
};

typedef std::shared_ptr<AutoDatabaseConnection> DatabaseConnection;

/**
 * @}
 */
}
#endif
