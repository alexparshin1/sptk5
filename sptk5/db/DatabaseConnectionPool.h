/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        DatabaseConnectionPool.h - description                ║
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

#ifndef __SPTK_DATABASECONNECTIONLOADER_H__
#define __SPTK_DATABASECONNECTIONLOADER_H__

#include <sptk5/db/AutoDatabaseConnection.h>
#include <sptk5/db/PoolDatabaseConnection.h>
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
typedef PoolDatabaseConnection* CreateDriverInstance(const char*);

/**
 * @brief Destroy driver instance function type
 */
typedef void DestroyDriverInstance(PoolDatabaseConnection*);

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
    friend class AutoDatabaseConnection;

    /**
     * Database driver
     */
    DatabaseDriver*                            m_driver {nullptr};

    /**
     * Function that creates driver instances
     */
    CreateDriverInstance*                      m_createConnection {nullptr};

    /**
     * Function that destroys driver instances
     */
    DestroyDriverInstance*                     m_destroyConnection {nullptr};

    /**
     * Maximum number of connections in the pool
     */
    unsigned                                   m_maxConnections;

    /**
     * Connection pool
     */
    SynchronizedQueue<PoolDatabaseConnection*>     m_pool;

protected:

    /**
     * List all connections
     */
    SynchronizedList<PoolDatabaseConnection*>      m_connections;

    /**
     * @brief Loads database driver
     *
     * First successfull driver load places driver into driver cache.
     */
    void load();

    /**
     * Close connection callback
     * @param item          Database connection
     * @param data          Data (connection pool pointer)
     * @return true if callback should continue to be executed
     */
    static bool closeConnectionCB(PoolDatabaseConnection*& item, void* data);

public:
    /**
     * @brief Constructor
     *
     * Database connection string is the same for all connections,
     * created with this object.
     * @param connectionString  Database connection string
     * @param maxConnections    Maximum number of connections in the pool
     */
    DatabaseConnectionPool(const String& connectionString, unsigned maxConnections = 100);

    /**
     * @brief Destructor
     *
     * Closes and destroys all created connections
     */
    ~DatabaseConnectionPool();

    DatabaseConnection getConnection();

protected:

    /**
     * @brief Creates database connection
     */
    PoolDatabaseConnection* createConnection();

    /**
     * @brief Returns used database connection back to the pool
     * @param connection        Database that is no longer in use and may be returned to the pool
     */
    void releaseConnection(PoolDatabaseConnection* connection);

    /**
     * @brief Destroys connection
     * @param connection DatabaseConnection*, destroys the driver instance
     * @param unlink            Should always be true for any external use
     */
    void destroyConnection(PoolDatabaseConnection* connection, bool unlink=true);
};

/**
 * @}
 */
}
#endif
