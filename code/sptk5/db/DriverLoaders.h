/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cutils>

namespace sptk {
class PoolDatabaseConnection;

/**
 * @brief Create the driver instance function.
 */
using CreateDriverInstance = PoolDatabaseConnection*(const char* connectString, size_t connectTimeoutSeconds);

/**
 * @brief Destroy the driver instance function.
 */
using DestroyDriverInstance = void(PoolDatabaseConnection*);

#ifdef WIN32
/**
 * @brief Windows: Driver DLL handle type.
 */
using DriverHandle = HMODULE;

#else
/**
 * @brief Unix: Driver shared library handle type.
 */
using DriverHandle = uint8_t*;

#endif

/**
 * @brief Information about the loaded database driver.
 */
struct SP_EXPORT DatabaseDriver
{
    /**
     * @brief Driver SO/DLL handle after the load.
     */
    DriverHandle m_handle;

    /**
     * @brief Function that creates driver instances.
     */
    CreateDriverInstance* m_createConnection;

    /**
     * @brief Function that destroys driver instances.
     */
    DestroyDriverInstance* m_destroyConnection;
};

/**
 * @brief Cache of the loaded database drivers.
 */
class SP_EXPORT DriverLoaders
{
public:
    /**
     * @brief Get a database driver by name.
     * @param driverName Driver name.
     * @return Database driver instance or nullptr if not found.
     */
    std::shared_ptr<DatabaseDriver> get(const String& driverName)
    {
        const std::scoped_lock lock(m_mutex);
        const auto             itor = m_drivers.find(driverName.toLowerCase());
        if (itor == m_drivers.end())
        {
            return nullptr;
        }
        return itor->second;
    }

    /**
     * @brief Add a database driver to the cache.
     * @param driverName Driver name.
     * @param driver Database driver instance.
     */
    void add(const String& driverName, const std::shared_ptr<DatabaseDriver>& driver)
    {
        const std::scoped_lock lock(m_mutex);
        m_drivers[driverName.toLowerCase()] = driver;
    }

    static DriverLoaders loadedDrivers;

private:
    std::mutex                                                          m_mutex;
    std::map<std::string, std::shared_ptr<DatabaseDriver>, std::less<>> m_drivers;
};

} // namespace sptk
