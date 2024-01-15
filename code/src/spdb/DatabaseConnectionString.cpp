/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
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
│   General Public License for more details.  OpenAPI generation development                                 │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <set> // Fedora
#include <sptk5/db/DatabaseConnectionString.h>
#include <sptk5/net/URL.h>

using namespace std;
using namespace sptk;

void DatabaseConnectionString::parse()
{
    static const set<String, less<>> supportedDrivers {"sqlite3", "postgres", "postgresql", "oracle", "oracleoci", "mysql",
                                                       "firebird", "odbc", "mssql"};

    URL url(m_connectionString);

    if (!supportedDrivers.contains(url.protocol()))
    {
        throw DatabaseException("Unsupported driver: " + url.protocol());
    }

    m_driverName = url.protocol();
    if (m_driverName == "postgres" || m_driverName == "pg")
    {
        m_driverName = "postgresql";
    }

    Strings hostAndPort(url.hostAndPort(), ":");
    while (hostAndPort.size() < 2)
    {
        hostAndPort.push_back("");
    }
    m_hostName = hostAndPort[0];
    m_portNumber = (uint16_t) string2int(hostAndPort[1], 0);
    m_userName = url.username();
    m_password = url.password();

    Strings databaseAndSchema(url.path().c_str() + 1, "/");
    while (databaseAndSchema.size() < 2)
    {
        databaseAndSchema.push_back("");
    }
    m_schema = databaseAndSchema[databaseAndSchema.size() - 1];
    databaseAndSchema.resize(databaseAndSchema.size() - 1);
    m_databaseName = databaseAndSchema.join("/");

    m_parameters = url.params();
}

String DatabaseConnectionString::toString() const
{
    stringstream result;

    result << (m_driverName.empty() ? "unknown" : m_driverName) << "://";
    if (!m_userName.empty())
    {
        result << m_userName;
        if (!m_password.empty())
        {
            result << ":" << m_password;
        }
        result << "@";
    }

    result << m_hostName;
    if (m_portNumber != 0)
    {
        result << ":" << m_portNumber;
    }

    if (!m_databaseName.empty())
    {
        result << "/" << m_databaseName;
    }

    if (!m_schema.empty())
    {
        result << "/" << m_schema;
    }

    if (!m_parameters.empty())
    {
        result << "?";
        bool first = true;
        for (const auto& [name, value]: m_parameters)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                result << "&";
            }
            result << name << "=" << value;
        }
    }

    return result.str();
}

String DatabaseConnectionString::parameter(const String& name) const
{
    auto itor = m_parameters.find(name);
    if (itor == m_parameters.end())
    {
        return "";
    }
    return itor->second;
}

bool DatabaseConnectionString::empty() const
{
    return m_hostName.empty();
}
