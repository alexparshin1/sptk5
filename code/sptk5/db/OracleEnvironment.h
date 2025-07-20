/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#ifdef HAVE_ORACLE

#include <occi.h>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * Oracle Environment
 *
 * Allows creating and terminating connections
 */
class OracleEnvironment
{
public:
    /**
     * Constructor
     */
    OracleEnvironment() = default;

    /**
     * Deleted copy constructor
     */
    OracleEnvironment(const OracleEnvironment&) = delete;

    /**
     * Move constructor
     */
    OracleEnvironment(OracleEnvironment&&) = default;

    /**
     * Deleted copy assignment
     */
    OracleEnvironment& operator=(const OracleEnvironment&) = delete;

    /**
     * Move assignment
     */
    OracleEnvironment& operator=(OracleEnvironment&&) = default;

    /**
     * Returns environment handle
     */
    oracle::occi::Environment* handle() const
    {
        return m_handle.get();
    }

    /**
     * Returns client version
     */
    static String clientVersion();

    /**
     * Creates new database connection
     * @param connectionString DatabaseConnectionString&, Connection parameters
     */
    oracle::occi::Connection* createConnection(const DatabaseConnectionString& connectionString);

    /**
     * Terminates database connection
     * @param connection oracle::occi::Connection*, Oracle connection
     */
    void terminateConnection(oracle::occi::Connection* connection) const;

private:
    /**
     * Environment handle
     */
    std::shared_ptr<oracle::occi::Environment> m_handle {
        std::shared_ptr<oracle::occi::Environment>(
            oracle::occi::Environment::createEnvironment("UTF8", "UTF8", oracle::occi::Environment::THREADED_MUTEXED),
            [](oracle::occi::Environment* handle)
            {
                oracle::occi::Environment::terminateEnvironment(handle);
            })};
};

/**
 * @}
 */
} // namespace sptk

#endif
