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

#include <sptk5/db/AutoDatabaseConnection.h>
#include <sptk5/db/DatabaseConnectionString.h>
#include <sptk5/db/DriverLoaders.h>
#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/threads/SynchronizedList.h>
#include <sptk5/threads/SynchronizedQueue.h>

namespace sptk {
/**
 * @addtogroup Database Database Support.
 * @{
 */

/**
 * @brief Database driver loader.
 *
 * Loads and initializes SPTK database driver by request.
 * Already loaded drivers are cached.
 */
class SP_EXPORT DatabaseConnectionPool
    : public DatabaseConnectionString
    , public std::mutex
{
    friend class AutoDatabaseConnection;

public:
    /**
     * @brief Constructor.
     *
     * Database connection string is the same for all connections created with this object.
     * @param connectionString  Database connection string.
     * @param maxConnections    Maximum number of connections in the pool.
     * @param connectionTimeout COnnection timeout.
     */
    DatabaseConnectionPool(const String& connectionString, unsigned maxConnections = 100, std::chrono::milliseconds connectionTimeout = std::chrono::seconds(60));

    /**
     * @brief Destructor.
     */
    ~DatabaseConnectionPool();

    /**
     * @brief Get database connection from the pool.
     * @return database connection.
     */
    [[nodiscard]] DatabaseConnection getConnection();

    /**
     * @brief Get the maximum number of connections in the pool.
     * @return maximum number of connections.
     */
    [[nodiscard]] size_t maxConnections() const
    {
        return m_maxConnections;
    }

    /**
     * @brief The timeout for waiting for a connection from the pool if it isn't available immediately.
     * @return The timeout for waiting for a connection.
     */
    [[nodiscard]] std::chrono::milliseconds connectionTimeout() const
    {
        return m_connectionTimeout;
    }

    /**
     * @brief Get the total number of allocated connections.
     * @return total allocated connections.
     */
    [[nodiscard]] size_t totalConnections() const;

    /**
     * @brief Get the number of available connections.
     * @return number of available connections.
     */
    [[nodiscard]] size_t availableConnections() const;

protected:
    /**
     * @brief Loads database driver.
     *
     * First successfull driver load places the driver into driver cache.
     */
    void load();

    /**
     * @brief Creates database connection.
     */
    [[nodiscard]] SPoolDatabaseConnection createConnection();

    /**
     * @brief Returns used database connection to the pool.
     * @param connection        Database that is no longer in use and may be returned to the pool.
     */
    void releaseConnection(const SPoolDatabaseConnection& connection);

private:
    std::shared_ptr<DatabaseDriver>            m_driver {nullptr};            ///< Database driver.
    CreateDriverInstance*                      m_createConnection {nullptr};  ///< Function that creates driver instances.
    DestroyDriverInstance*                     m_destroyConnection {nullptr}; ///< Function that destroys driver instances.
    std::atomic<size_t>                        m_maxConnections {128};        ///< Maximum number of connections in the pool.
    SynchronizedQueue<SPoolDatabaseConnection> m_pool;                        ///< Available connections.
    SynchronizedList<SPoolDatabaseConnection>  m_connections;                 ///< All connections.
    std::chrono::milliseconds                  m_connectionTimeout;           ///< Connection timeout.
};

/**
 * @}
 */
} // namespace sptk
