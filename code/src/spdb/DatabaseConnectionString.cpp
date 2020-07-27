/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
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
│   General Public License for more details.  OpenAPI generation development                                 │
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
#include <sptk5/net/URL.h>

#include <set>

using namespace std;
using namespace sptk;

void DatabaseConnectionString::parse()
{
    static const set<string> supportedDrivers{ "sqlite3", "postgres", "postgresql", "oracle", "mysql", "firebird", "odbc", "mssql" };

    URL url(m_connectionString);

    if (supportedDrivers.find(url.protocol()) == supportedDrivers.end())
        throwDatabaseException("Unsupported driver: " + url.protocol());

    m_driverName = url.protocol();
    if (m_driverName == "postgres" || m_driverName == "pg")
        m_driverName = "postgresql";

    Strings hostAndPort(url.hostAndPort(), ":");
    while (hostAndPort.size() < 2)
        hostAndPort.push_back("");
    m_hostName = hostAndPort[0];
    m_portNumber = string2int(hostAndPort[1], 0);
    m_userName = url.username();
    m_password = url.password();

    Strings databaseAndSchema(url.path().c_str() + 1, "/");
    while (databaseAndSchema.size() < 2)
        databaseAndSchema.push_back("");
    m_databaseName = databaseAndSchema[0];
    m_schema = databaseAndSchema[1];

    m_parameters = url.params();
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
    if (m_portNumber != 0)
        result << ":" << m_portNumber;

    if (!m_databaseName.empty())
        result << "/" << m_databaseName;

    if (!m_schema.empty())
        result << "/" << m_schema;

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
    DatabaseConnectionString simple("postgres://auser:apassword@localhost:5432/dbname/main?encoding=UTF8&mode=free");
    EXPECT_STREQ("auser", simple.userName().c_str());
    EXPECT_STREQ("apassword", simple.password().c_str());
    EXPECT_STREQ("localhost", simple.hostName().c_str());
    EXPECT_EQ(5432, simple.portNumber());
    EXPECT_STREQ("dbname", simple.databaseName().c_str());
    EXPECT_STREQ("main", simple.schema().c_str());

    EXPECT_STREQ("UTF8", simple.parameter("encoding").c_str());
    EXPECT_STREQ("free", simple.parameter("mode").c_str());
}

TEST(SPTK_DatabaseConnectionString, ctorCopy)
{
    DatabaseConnectionString source("postgres://auser:apassword@localhost:5432/dbname/main?encoding=UTF8&mode=free");
    DatabaseConnectionString simple(source);
    EXPECT_STREQ("auser", simple.userName().c_str());
    EXPECT_STREQ("apassword", simple.password().c_str());
    EXPECT_STREQ("localhost", simple.hostName().c_str());
    EXPECT_EQ(5432, simple.portNumber());
    EXPECT_STREQ("dbname", simple.databaseName().c_str());
    EXPECT_STREQ("main", simple.schema().c_str());
    EXPECT_STREQ("UTF8", simple.parameter("encoding").c_str());
    EXPECT_STREQ("free", simple.parameter("mode").c_str());
}

TEST(SPTK_DatabaseConnectionString, toString)
{
    String connectionString("postgresql://auser:apassword@localhost:5432/dbname/main?encoding=UTF8&mode=free");
    DatabaseConnectionString simple(connectionString);
    EXPECT_STREQ(connectionString.c_str(), simple.toString().c_str());
}

#endif
