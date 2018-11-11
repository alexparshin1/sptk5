/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DatabaseConnectionString.cpp - description             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/db/DatabaseConnectionString.h>
#include <sptk5/Exception.h>

using namespace std;
using namespace sptk;

static const Strings driverNames("sqlite3|postgres|postgresql|oracle|mysql|firebird|odbc|mssql", "|");

void DatabaseConnectionString::parse()
{
    m_driverName = "";
    m_hostName = "";
    m_portNumber = 0;
    m_userName = "";
    m_password = "";
    m_databaseName = "";
    m_parameters.clear();

    if (m_connectionString.empty())
        return;

    size_t pos;
    String connStr(m_connectionString);

    // Find extra parameters
    pos = connStr.find_first_of('?');
    if (pos != string::npos) {
        Strings parameters(connStr.substr(pos + 1),"&");
        for (auto& item: parameters) {
            Strings pair(item, "='", Strings::SM_ANYCHAR);
            if (pair.size() == 2)
                m_parameters[ pair[0] ] = pair[1];
        }
        connStr.erase(pos);
    }

    pos = connStr.find("://");
    if (pos != string::npos) {
        m_driverName = connStr.substr(0, pos);
        connStr.erase(0, pos + 3);
        if (driverNames.indexOf(m_driverName) < 0)
            throwDatabaseException("Driver name is unknown: " + m_connectionString);
    } else
        throwDatabaseException("Driver name is missing: " + m_connectionString);

    pos = connStr.find('@');
    if (pos != string::npos) {
        Strings usernameAndPassword(connStr.substr(0, pos),":");
        m_userName = usernameAndPassword[0];
        if (usernameAndPassword.size() > 1)
            m_password = usernameAndPassword[1];
        connStr.erase(0, pos + 1);
    }

    pos = connStr.find('/');
    if (pos != string::npos) {
        m_databaseName = connStr.substr(pos + 1);
        connStr.erase(pos);
        if (m_databaseName.find('/') != string::npos)
            m_databaseName = "/" + m_databaseName;
    }

    Strings hostAndPort(connStr, ":");
    m_hostName = hostAndPort[0];
    if (hostAndPort.size() > 1)
        m_portNumber = (uint16_t) strtol(hostAndPort[1].c_str(), nullptr, 10);
}

String DatabaseConnectionString::toString() const
{
    stringstream result;

    result << (m_driverName.empty() ? "unknown" : m_driverName) << "://";
    if (!m_userName.empty()) {
        result << m_userName;
        if (!m_password.empty())
            result << ":" << m_password;
        result << "@";
    }

    result << m_hostName;
    if (!m_databaseName.empty())
        result << "/" << m_databaseName;

    if (!m_parameters.empty()) {
        result << "?";
        bool first = true;
        for (auto& parameter: m_parameters) {
            if (first)
                first = false;
            else
                result << "&";
            result << parameter.first << "=" << parameter.second;
        }
    }

    return result.str();
}

String DatabaseConnectionString::parameter(const String& name) const
{
    auto itor = m_parameters.find(name);
    if (itor == m_parameters.end())
        return "";
    return itor->second;
}

bool DatabaseConnectionString::empty() const
{
    return m_hostName.empty();
}

#if USE_GTEST
#include <gtest/gtest.h>

TEST(SPTK_DatabaseConnectionString, ctorSimple)
{
    DatabaseConnectionString simple("postgres://localhost/dbname");
    EXPECT_STREQ("postgres", simple.driverName().c_str());
    EXPECT_STREQ("localhost", simple.hostName().c_str());
    EXPECT_STREQ("dbname", simple.databaseName().c_str());
}

TEST(SPTK_DatabaseConnectionString, ctorFull)
{
    DatabaseConnectionString simple("postgres://auser:apassword@localhost:5432/dbname?encoding=UTF8&schema=main");
    EXPECT_STREQ("auser", simple.userName().c_str());
    EXPECT_STREQ("apassword", simple.password().c_str());
    EXPECT_STREQ("localhost", simple.hostName().c_str());
    EXPECT_EQ(5432, simple.portNumber());

    EXPECT_STREQ("UTF8", simple.parameter("encoding").c_str());
    EXPECT_STREQ("main", simple.parameter("schema").c_str());
}

#endif
