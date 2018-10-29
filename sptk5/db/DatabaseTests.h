/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DatabaseTests.cpp - description                        ║
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

#ifndef __SPTK_DATABASE_TESTS_H__
#define __SPTK_DATABASE_TESTS_H__

#include "DatabaseConnectionString.h"
#include "AutoDatabaseConnection.h"

namespace sptk {

class DatabaseTests
{
    std::map<String, DatabaseConnectionString> m_connectionStrings;

    size_t countRowsInTable(DatabaseConnection db, const String& table);
public:
    DatabaseTests();
    void addDatabaseConnection(const DatabaseConnectionString& connectionString);
    std::vector<DatabaseConnectionString> connectionStrings() const;
    DatabaseConnectionString connectionString(const String& driverName) const;

    void testConnect(const DatabaseConnectionString& connectionString);
    void testDDL(const DatabaseConnectionString& connectionString);
    void testQueryParameters(const DatabaseConnectionString& connectionString);
    void testTransaction(const DatabaseConnectionString& connectionString);
    void testBulkInsert(const DatabaseConnectionString& connectionString);
};

extern DatabaseTests databaseTests;

}

#endif