/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        OracleEnvironment.h - description                     ║
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

#ifndef __SPTK_ORACLEENVIRONMENT_H__
#define __SPTK_ORACLEENVIRONMENT_H__

#include <sptk5/db/PoolDatabaseConnection.h>

#if HAVE_ORACLE == 1

#include <occi.h>

namespace sptk
{

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * @brief Oracle Environment
 *
 * Allows creating and terminating connections
 */
class OracleEnvironment
{
    oracle::occi::Environment* m_handle;
public:
    /**
     * @brief Constructor
     */
    OracleEnvironment();

    /**
     * @brief Destructor
     */
    ~OracleEnvironment();

    /**
     * @brief Returns environment handle
     */
    oracle::occi::Environment* handle() const
    {
        return m_handle;
    }

    /**
     * @brief Returns client version
     */
    std::string clientVersion() const;

    /**
     * @brief Creates new database connection
     * @param connectionString DatabaseConnectionString&, Connection parameters
     */
    oracle::occi::Connection* createConnection(DatabaseConnectionString& connectionString);

    /**
     * @brief Terminates database connection
     * @param connection oracle::occi::Connection*, Oracle connection
     */
    void terminateConnection(oracle::occi::Connection* connection);
};

/**
 * @}
 */
}

#endif

#endif
