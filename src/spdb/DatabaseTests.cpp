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

#include <sptk5/db/DatabaseTests.h>
#include <sptk5/db/DatabaseConnectionPool.h>
#include <sptk5/db/Query.h>
#include <sptk5/db/Transaction.h>

using namespace std;
using namespace sptk;

DatabaseTests sptk::databaseTests;

DatabaseTests::DatabaseTests()
{
}

vector<DatabaseConnectionString> DatabaseTests::connectionStrings() const
{
    vector<DatabaseConnectionString> connectionStrings;
    for (auto itor: m_connectionStrings)
        connectionStrings.push_back(itor.second);
    return connectionStrings;
}

void DatabaseTests::addConnection(const DatabaseConnectionString& connectionString)
{
    m_connectionStrings[connectionString.driverName()] = connectionString;
}

void DatabaseTests::testConnect(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection* db = connectionPool.createConnection();

    db->open();

    Strings objects;
    db->objectList(DOT_TABLES, objects);
    db->close();

    connectionPool.destroyConnection(db);
}

void DatabaseTests::testDDL(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection* db = connectionPool.createConnection();

    db->open();

    Query createTable(db, "CREATE TABLE gtest_temp_table(id INT, name VARCHAR(20))");
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try {
        dropTable.exec();
    }
    catch (...) {}

    createTable.exec();
    dropTable.exec();

    db->close();
    connectionPool.destroyConnection(db);
}

struct Row {
    int         id;
    string      name;
    double      price;
    DateTime    ts;
};

static const vector<Row> rows = {
    { 1, "apple", 1.5, DateTime::Now() },
    { 2, "pear",  3.1, DateTime::Now() },
    { 3, "melon", 1.05, DateTime() },
    { 4, "watermelon", 0.85, DateTime::Now() },
    { 5, "lemon", 5.5, DateTime::Now() }
};

static const map<String,String> dateTimeFieldTypes = {
    { "mysql", "TIMESTAMP" },
    { "postgresql", "TIMESTAMP" },
    { "mssql", "DATETIME" },
    { "oracle", "DATETIME" }
};

void DatabaseTests::testQueryParameters(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection* db = connectionPool.createConnection();

    auto itor = dateTimeFieldTypes.find(connectionString.driverName());
    if (itor == dateTimeFieldTypes.end())
        throw Exception("DateTime data type mapping is not defined for the test");
    String dateTimeType = itor->second;

    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table(id INT, name VARCHAR(20), price DECIMAL(10,2), ";
    createTableSQL << "ts " << dateTimeType;
    createTableSQL << ")";

    db->open();
    Query createTable(db, createTableSQL.str());
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try { dropTable.exec(); } catch (...) {}

    createTable.exec();

    Query insert(db, "INSERT INTO gtest_temp_table VALUES(:id, :name, :price, :ts)");
    for (auto& row: rows) {
        insert.param("id") = row.id;
        insert.param("name") = row.name;
        insert.param("price") = row.price;
        insert.param("ts") = row.ts;
        insert.exec();
    }

    Query select(db, "SELECT * FROM gtest_temp_table");
    select.open();
    for (auto& row: rows) {
        if (select.eof())
            break;
        if (row.id != select["id"].asInteger())
            throw Exception("row.id != table data");
        if (row.name != select["name"].asString())
            throw Exception("row.name != table data");
        if (row.price != select["price"].asFloat())
            throw Exception("row.price != table data");
        select.next();
    }
    select.close();

    dropTable.exec();

    db->close();

    connectionPool.destroyConnection(db);
}

void DatabaseTests::testTransaction(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection* db = connectionPool.createConnection();

    db->open();
    Query createTable(db, "CREATE TABLE gtest_temp_table(id INT, name VARCHAR(20))");
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try { dropTable.exec(); } catch (...) {}

    createTable.exec();

    Transaction transaction(*db);
    transaction.begin();

    Query insert(db, "INSERT INTO gtest_temp_table VALUES('1', 'pear')");

    insert.exec();
    insert.exec();

    Query select(db, "SELECT count(*) cnt FROM gtest_temp_table");
    select.open();
    size_t count = select["cnt"];
    if (count != 2)
        throw Exception("count != 2");
    select.close();

    transaction.rollback();

    select.open();
    count = select["cnt"];
    if (count != 0)
        throw Exception("count != 0");
    select.close();

    dropTable.exec();

    db->close();

    connectionPool.destroyConnection(db);
}

DatabaseConnectionString DatabaseTests::connectionString(const String& driverName) const
{
    auto itor = m_connectionStrings.find(driverName);
    if (itor == m_connectionStrings.end())
        return DatabaseConnectionString("");
    return itor->second;
}
