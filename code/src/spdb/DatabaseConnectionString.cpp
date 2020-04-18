/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DatabaseConnectionString.cpp - description             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
#include <sptk5/RegularExpression.h>

using namespace std;
using namespace sptk;

const RegularExpression DatabaseConnectionString::parseConnectionString(
        "^(?<driver>sqlite3|postgres|postgresql|oracle|mysql|firebird|odbc|mssql)://"
        "((?<username>\\S+):(?<password>\\S+)@)?"
        "(?<host>[^:/]+)(:(?<port>\\d+))?"
        "(/(?<database>[^/]+))?"
        "(/(?<schema>[^/]+))?$"
        );

void DatabaseConnectionString::parse()
{
    m_driverName = "";
    m_hostName = "";
    m_portNumber = 0;
    m_userName = "";
    m_password = "";
    m_databaseName = "";
    m_schema = "";
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

    auto matches = parseConnectionString.m(connStr);
    if (!matches)
        throwDatabaseException("Database connection string is invalid: " + m_connectionString)

    m_driverName = matches[String("driver")].value;
    if (m_driverName == "postgres")
        m_driverName = "postgresql";

    m_userName = matches[String("username")].value;
    m_password = matches[String("password")].value;
    m_databaseName = matches[String("database")].value;
    m_schema = matches[String("schema")].value;
    m_hostName = matches[String("host")].value;

    String port = matches[String("port")].value;
    if (!port.empty())
        m_portNumber = (uint16_t) strtol(port.c_str(), nullptr, 10);
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

TEST(SPTK_DatabaseConnectionString, ctorSimple)
{
    DatabaseConnectionString simple("postgres://localhost/dbname");
    EXPECT_STREQ("postgresql", simple.driverName().c_str());
    EXPECT_STREQ("localhost", simple.hostName().c_str());
    EXPECT_STREQ("dbname", simple.databaseName().c_str());
}

TEST(SPTK_DatabaseConnectionString, ctorAdvanced)
{
    DatabaseConnectionString simple("postgresql://localhost/dbname/schema");
    EXPECT_STREQ("postgresql", simple.driverName().c_str());
    EXPECT_STREQ("localhost", simple.hostName().c_str());
    EXPECT_STREQ("dbname", simple.databaseName().c_str());
    EXPECT_STREQ("schema", simple.schema().c_str());
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
