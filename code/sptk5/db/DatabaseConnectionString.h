/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
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

#include <sptk5/Strings.h>
#include <sptk5/net/HttpParams.h>

#include <utility>

namespace sptk {

/**
 * @addtogroup Database Database Support.
 * @{
 */


/**
 * @brief Database Connection String.
 *
 * Database connection string includes driver name ('odbc', 'sqlite3', etc.) as a protocol name,
 * and username, password, server name in a traditional form. Database name is optionally defined
 * after the server name and '/' delimiter.
 *
 * Example:
 *   drivername://[username:password]\@servername[:port]/databasename.
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
     * @brief Constructor.
     * @param connectionString  Database connection string.
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
     * @brief Copy constructor.
     * @param cs                Database connection string object to copy from.
     */
    DatabaseConnectionString(const DatabaseConnectionString& cs) = default;

    /**
     * @brief Assignment.
     * @param cs                Database connection string object to copy from.
     */
    DatabaseConnectionString& operator=(const DatabaseConnectionString& cs) = default;

    /**
     * @brief Return the connection string as a string, with the optional password.
     * @param includePassword   If true, then the password is included.
     * @return Connection string.
     */
    [[nodiscard]] String toString(bool includePassword = true) const;

    /**
     * @brief Returns driver name.
     */
    [[nodiscard]] const String& driverName() const
    {
        return m_driverName;
    }

    /**
     * @brief Returns host name.
     */
    [[nodiscard]] const String& hostName() const
    {
        return m_hostName;
    }

    /**
     * @brief Returns user name.
     */
    [[nodiscard]] const String& userName() const
    {
        return m_userName;
    }

    /**
     * @brief Returns user password.
     */
    [[nodiscard]] const String& password() const
    {
        return m_password;
    }

    /**
     * @brief Returns database name.
     */
    [[nodiscard]] const String& databaseName() const
    {
        return m_databaseName;
    }

    /**
     * @brief Returns schema name.
     */
    [[nodiscard]] const String& schema() const
    {
        return m_schema;
    }

    /**
     * @brief Returns server port number.
     */
    [[nodiscard]] uint16_t portNumber() const
    {
        return m_portNumber;
    }

    /**
     * @brief Set the new username.
     * @param user              New username.
     */
    void userName(const String& user)
    {
        m_userName = user;
    }

    /**
     * @brief Set the new password.
     * @param pass              New password.
     */
    void password(const String& pass)
    {
        m_password = pass;
    }

    /**
     * @brief Return optional parameter value.
     * @param name              Parameter name.
     * @return parameter value.
     */
    [[nodiscard]] String parameter(const String& name) const;

    /**
     * @brief Is the connection string empty?
     */
    [[nodiscard]] bool empty() const;

protected:
    /**
     * @brief Parses connection string.
     */
    void parse();

private:
    /**
     * @brief Database connection string.
     */
    String m_connectionString;

    /**
     * @brief Database server host name.
     */
    String m_hostName;

    /**
     * @brief Database server port number.
     */
    uint16_t m_portNumber {0};

    /**
     * @brief Database username.
     */
    String m_userName;

    /**
     * @brief Database user password.
     */
    String m_password;

    /**
     * @brief Database name.
     */
    String m_databaseName;

    /**
     * @brief Database schema.
     */
    String m_schema;

    /**
     * @brief Optional parameters.
     */
    HttpParams m_parameters;

    /**
     * @brief Database driver name.
     */
    String m_driverName;
};

/**
 * @}
 */
} // namespace sptk
