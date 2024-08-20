/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
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
│   General Public License for more details.  OpenAPI generation development                                 │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <gtest/gtest.h>
#include <set> // Fedora
#include <sptk5/db/DatabaseConnectionString.h>
#include <sptk5/net/URL.h>

using namespace std;
using namespace sptk;

TEST(SPTK_DatabaseConnectionString, ctorSimple)
{
    const DatabaseConnectionString empty;
    EXPECT_TRUE(empty.empty());

    const DatabaseConnectionString simple("postgres://localhost/dbname");
    EXPECT_STREQ("postgresql", simple.driverName().c_str());
    EXPECT_STREQ("localhost", simple.hostName().c_str());
    EXPECT_STREQ("dbname", simple.databaseName().c_str());
}

TEST(SPTK_DatabaseConnectionString, errorHandling)
{
    EXPECT_THROW(
        DatabaseConnectionString("unknown://localhost/dbname"), DatabaseException);
}

TEST(SPTK_DatabaseConnectionString, ctorAdvanced)
{
    const DatabaseConnectionString simple("postgresql://localhost/dbname/schema");
    EXPECT_STREQ("postgresql", simple.driverName().c_str());
    EXPECT_STREQ("localhost", simple.hostName().c_str());
    EXPECT_STREQ("dbname", simple.databaseName().c_str());
    EXPECT_STREQ("schema", simple.schema().c_str());
}

TEST(SPTK_DatabaseConnectionString, ctorFull)
{
    const DatabaseConnectionString simple("postgres://auser:apassword@localhost:5432/dbname/main?encoding=UTF8&mode=free");
    EXPECT_STREQ("auser", simple.userName().c_str());
    EXPECT_STREQ("apassword", simple.password().c_str());
    EXPECT_STREQ("localhost", simple.hostName().c_str());
    EXPECT_EQ(5432, simple.portNumber());
    EXPECT_STREQ("dbname", simple.databaseName().c_str());
    EXPECT_STREQ("main", simple.schema().c_str());

    EXPECT_STREQ("UTF8", simple.parameter("encoding").c_str());
    EXPECT_STREQ("free", simple.parameter("mode").c_str());
    EXPECT_STREQ("", simple.parameter("xyz").c_str());
}

TEST(SPTK_DatabaseConnectionString, ctorCopy)
{
    const DatabaseConnectionString source("postgres://auser:apassword@localhost:5432/dbname/main?encoding=UTF8&mode=free");
    const DatabaseConnectionString simple(source);
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
    const String connectionString("postgresql://auser:apassword@localhost:5432/dbname/main?encoding=UTF8&mode=free");
    const DatabaseConnectionString simple(connectionString);
    EXPECT_STREQ(connectionString.c_str(), simple.toString().c_str());
}
