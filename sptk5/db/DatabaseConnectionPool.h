/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        DatabaseConnectionPool.h - description                ║
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

#ifndef __SPTK_DATABASECONNECTIONLOADER_H__
#define __SPTK_DATABASECONNECTIONLOADER_H__

#include <sptk5/db/DatabaseConnection.h>
#include <sptk5/db/DatabaseConnectionString.h>
#include <sptk5/CaseInsensitiveCompare.h>
#include <sptk5/threads/SynchronizedList.h>
#include <sptk5/threads/SynchronizedQueue.h>

namespace sptk
{

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * @brief Create driver instance function type
 */
typedef DatabaseConnection* CreateDriverInstance(const char*);

/**
 * @brief Destroy driver instance function type
 */
typedef void DestroyDriverInstance(DatabaseConnection*);

#ifdef WIN32
    /**
     * Windows: Driver DLL handle type
     */
    typedef HMODULE DriverHandle;

#else
    /**
     * Unix: Driver shared library handle type
     */
    typedef void*   DriverHandle;

#endif

/**
 * @brief Information about loaded database driver
 */
struct SP_EXPORT DatabaseDriver
{
    /**
     * Driver SO/DLL handle after load
     */
    DriverHandle                               m_handle;

    /**
     * Function that creates driver instances
     */
    CreateDriverInstance*                      m_createConnection;

    /**
     * Function that destroys driver instances
     */
    DestroyDriverInstance*                     m_destroyConnection;

};

/**
 * @brief Database driver loader
 *
 * Loads and initializes SPTK database driver by request.
 * Already loaded drivers are cached.
 */
class SP_EXPORT DatabaseConnectionPool : public DatabaseConnectionString, public std::mutex
{
    /**
     * Database driver
     */
    DatabaseDriver*                            m_driver;

protected:
    /**
     * Function that creates driver instances
     */
    CreateDriverInstance*                      m_createConnection;

    /**
     * Function that destroys driver instances
     */
    DestroyDriverInstance*                     m_destroyConnection;

    /**
     * Maximum number of connections in the pool
     */
    unsigned                                   m_maxConnections;

    /**
     * List all connections
     */
    SynchronizedList<DatabaseConnection*>      m_connections;

    /**
     * Connection pool
     */
    SynchronizedQueue<DatabaseConnection*>     m_pool;


    /**
     * @brief Loads database driver
     *
     * First successfull driver load places driver into driver cache.
     */
    void load();

public:
    /**
     * @brief Constructor
     *
     * Database connection string is the same for all connections,
     * created with this object.
     * @param connectionString std::string, Database connection string
     * @param maxConnections unsigned, Maximum number of connections in the pool
     */
    DatabaseConnectionPool(const std::string& connectionString, unsigned maxConnections = 100);

    /**
     * @brief Destructor
     *
     * Closes and destroys all created connections
     */
    ~DatabaseConnectionPool();

    /**
     * @brief Creates database connection
     */
    DatabaseConnection* createConnection();

    /**
     * @brief Returns used database connection back to the pool
     * @param connection DatabaseConnection*, Database that is no longer in use and may be returned to the pool
     */
    void releaseConnection(DatabaseConnection* connection);

    /**
     * @brief Destroys connection
     * @param connection DatabaseConnection*, destroys the driver instance
     * @param unlink bool, should always be true for any external use
     */
    void destroyConnection(DatabaseConnection* connection, bool unlink=true);
};

/**
 * @brief Wrapper for CDatabase connection that automatically handles connection create and release
 */
class AutoDatabaseConnection
{
    /**
     * Database connection pool
     */
    DatabaseConnectionPool&    m_connectionPool;

    /**
     * Database connection
     */
    DatabaseConnection*        m_connection;

public:

    /**
     * @brief Constructor
     * Automatically gets connection from connection pool
     * @param connectionPool DatabaseConnectionPool&, Database connection pool
     */
    AutoDatabaseConnection(DatabaseConnectionPool& connectionPool)
    : m_connectionPool(connectionPool)
    {
        m_connection = m_connectionPool.createConnection();
    }

    /**
     * @brief Destructor
     * Releases connection to connection pool
     */
    ~AutoDatabaseConnection()
    {
        if (m_connection)
            m_connectionPool.releaseConnection(m_connection);
    }

    /**
     * @brief Returns database connection acquired from the connection pool
     */
    DatabaseConnection* connection()
    {
        return m_connection;
    }
};

/**
 * @}
 */
}
#endif
