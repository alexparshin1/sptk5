/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       unit_tests.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/test/TestRunner.h>
#include <sptk5/threads/Locks.h>
#include <sptk5/DateTime.h>

using namespace std;
using namespace sptk;

int main(int argc, char* argv[])
{
    TestRunner  tests(argc, argv);

    tests.addDatabaseConnection(DatabaseConnectionString("postgresql://gtest:test#123@dbhost_pg:5432/gtest"));
    tests.addDatabaseConnection(DatabaseConnectionString("mysql://gtest:test#123@dbhost_mysql:3306/gtest"));
    tests.addDatabaseConnection(DatabaseConnectionString("mssql://gtest:test#123@dsn_mssql/gtest"));
    tests.addDatabaseConnection(DatabaseConnectionString("oracle://gtest:test#123@oracledb:1521/XE"));

    return tests.runAllTests();
}
