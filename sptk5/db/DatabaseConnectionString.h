/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        DatabaseConnectionString.h - description              ║
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

#ifndef __SPTK_DATABASECONNECTIONSTRING_H__
#define __SPTK_DATABASECONNECTIONSTRING_H__

#include <sptk5/sptk.h>
#include <sptk5/Strings.h>

namespace sptk
{

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
     * Connection string parameters
     */
    typedef std::map<String,String> Parameters;

private:

    /**
     * Database connection string
     */
    String     m_connectionString;

    /**
     * Database server host name
     */
    String     m_hostName;

    /**
     * Database server port number
     */
    uint16_t   m_portNumber {0};

    /**
     * Database user name
     */
    String     m_userName;

    /**
     * Database user password
     */
    String     m_password;

    /**
     * Database name
     */
    String     m_databaseName;

    /**
     * Optional parameters
     */
    Parameters m_parameters;

protected:

    /**
     * Database driver name
     */
    String     m_driverName;

    /**
     * Parses connection string
     */
    void parse();


public:
    /**
     * Constructor
     * @param connectionString  Database connection string
     */
    explicit DatabaseConnectionString(const String& connectionString = "") :
        m_connectionString(connectionString)
    {
        parse();
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
    DatabaseConnectionString& operator = (const DatabaseConnectionString& cs) = default;

    /**
     * Returns connection string
     */
    String toString() const;

    /**
     * Returns driver name
     */
    const String& driverName() const
    {
        return m_driverName;
    }

    /**
     * Returns host name
     */
    const String& hostName() const
    {
        return m_hostName;
    }

    /**
     * Returns user name
     */
    const String& userName() const
    {
        return m_userName;
    }

    /**
     * Returns user password
     */
    const String& password() const
    {
        return m_password;
    }

    /**
     * Returns database name
     */
    const String& databaseName() const
    {
        return m_databaseName;
    }

    /**
     * Returns server port number
     */
    uint16_t portNumber() const
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
    String parameter(const String& name) const;

    /**
     * Is connection string empty?
     */
    bool empty() const;
};
/**
 * @}
 */
}
#endif
