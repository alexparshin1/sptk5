/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        DatabaseConnectionString.h - description              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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
#include <sptk5/Exception.h>

namespace sptk
{

/// @addtogroup Database Database Support
/// @{


/// @brief Database Connection String
///
/// Database connection string includes driver name ('odbc', 'sqlite3', etc) as a protocol name,
/// and username, password, server name in a traditional form. Database name is optionally defined
/// after server name and '/' delimiter.
///
/// Example:
///   drivername://[username:password]\@servername[:port]/databasename
///
/// Some driver-specific parameters may be passed after '?':
///   drivername://username:password\@servername/databasename?timeout=10&reconnect=30
///
/// This class is thread-safe.
class SP_EXPORT DatabaseConnectionString
{
public:
    /// @brief Connection string parameters
    typedef std::map<std::string,std::string> Parameters;

protected:
    /// @brief Parses connection string
    void parse() THROWS_EXCEPTIONS;

    std::string     m_connectionString;         ///< Database connection string
    std::string     m_driverName;               ///< Database driver name
    std::string     m_hostName;                 ///< Database server host name
    uint16_t        m_portNumber;               ///< Database server port number
    std::string     m_userName;                 ///< Database user name
    std::string     m_password;                 ///< Database user name
    std::string     m_databaseName;             ///< Database user password
    Parameters      m_parameters;               ///< Optional parameters

public:
    /// @brief Constructor
    /// @param connectionString std::string, Database connection string
    DatabaseConnectionString(std::string connectionString) :
        m_connectionString(connectionString),
        m_portNumber(0)
    {
        parse();
    }

    /// @brief Copy constructor
    /// @param cs const DatabaseConnectionString&, Database connection string object to copy from
    DatabaseConnectionString(const DatabaseConnectionString& cs) :
        m_connectionString(cs.m_connectionString),
        m_portNumber(0)
    {
        parse();
    }

    /// @brief Assignment
    /// @param cs const DatabaseConnectionString&, Database connection string object to copy from
    DatabaseConnectionString& operator = (const DatabaseConnectionString& cs)
    {
        m_connectionString = cs.m_connectionString;
        m_driverName = "";
        m_hostName = "";
        m_portNumber = 0;
        m_userName = "";
        m_password = "";
        m_databaseName = "";
        m_parameters.clear();
        parse();
        return *this;
    }

    /// @brief Returns connection string
    const std::string& str() const
    {
        return m_connectionString;
    }

    /// @brief Returns driver name
    const std::string& driverName() const
    {
        return m_driverName;
    }

    /// @brief Returns host name
    const std::string& hostName() const
    {
        return m_hostName;
    }

    /// @brief Returns user name
    const std::string& userName() const
    {
        return m_userName;
    }

    /// @brief Returns user password
    const std::string& password() const
    {
        return m_password;
    }

    /// @brief Returns database name
    const std::string& databaseName() const
    {
        return m_databaseName;
    }

    /// @brief Returns server port number
    uint16_t portNumber() const
    {
        return m_portNumber;
    }

    /// @brief Returns optional database parameters
    const Parameters& parameters() const
    {
        return m_parameters;
    }
};
/// @}
}
#endif
