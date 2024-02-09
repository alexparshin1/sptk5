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

#include <sptk5/Strings.h>
#include <sptk5/net/HttpParams.h>

#include <utility>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */


/**
 * Database Connection String
 *
 * Database connection string includes driver name ('odbc', 'sqlite3', etc) as a protocol name,
 * and username, password, server name in a traditional form. Database name is optionally defined
 * after server name and '/' delimiter.
 *
 * Example:
 *   drivername://[username:password]\@servername[:port]/databasename
 *
 * Some driver-specific parameters may be passed after '?':
 *   drivername://username:password\@servername/databasename?timeout=10&reconnect=30
 *
 * This class is thread-safe.
 */
class SP_EXPORT DatabaseConnectionString
{
public:
    /**
     * Constructor
     * @param connectionString  Database connection string
     */
    explicit DatabaseConnectionString(String connectionString = "")
        : m_connectionString(std::move(connectionString))
    {
        if (!m_connectionString.empty())
        {
            parse();
        }
    }

    /**
     * Copy constructor
     * @param cs                Database connection string object to copy from
     */
    DatabaseConnectionString(const DatabaseConnectionString& cs) = default;

    /**
     * Assignment
     * @param cs                Database connection string object to copy from
     */
    DatabaseConnectionString& operator=(const DatabaseConnectionString& cs) = default;

    /**
     * Returns connection string
     */
    [[nodiscard]] String toString() const;

    /**
     * Returns driver name
     */
    [[nodiscard]] const String& driverName() const
    {
        return m_driverName;
    }

    /**
     * Returns host name
     */
    [[nodiscard]] const String& hostName() const
    {
        return m_hostName;
    }

    /**
     * Returns user name
     */
    [[nodiscard]] const String& userName() const
    {
        return m_userName;
    }

    /**
     * Returns user password
     */
    [[nodiscard]] const String& password() const
    {
        return m_password;
    }

    /**
     * Returns database name
     */
    [[nodiscard]] const String& databaseName() const
    {
        return m_databaseName;
    }

    /**
     * Returns schema name
     */
    [[nodiscard]] const String& schema() const
    {
        return m_schema;
    }

    /**
     * Returns server port number
     */
    [[nodiscard]] uint16_t portNumber() const
    {
        return m_portNumber;
    }

    /**
     * Set new user name
     * @param user              New user name
     */
    void userName(const String& user)
    {
        m_userName = user;
    }

    /**
     * Set new password
     * @param pass              New password
     */
    void password(const String& pass)
    {
        m_password = pass;
    }

    /**
     * Return optional parameter value
     * @param name              Parameter name
     * @return parameter value
     */
    [[nodiscard]] String parameter(const String& name) const;

    /**
     * Is connection string empty?
     */
    [[nodiscard]] bool empty() const;

protected:
    /**
     * Parses connection string
     */
    void parse();

private:
    /**
     * Database connection string
     */
    String m_connectionString;

    /**
     * Database server host name
     */
    String m_hostName;

    /**
     * Database server port number
     */
    uint16_t m_portNumber {0};

    /**
     * Database user name
     */
    String m_userName;

    /**
     * Database user password
     */
    String m_password;

    /**
     * Database name
     */
    String m_databaseName;

    /**
     * Database schema
     */
    String m_schema;

    /**
     * Optional parameters
     */
    HttpParams m_parameters;

    /**
     * Database driver name
     */
    String m_driverName;
};

/**
 * @}
 */
} // namespace sptk
