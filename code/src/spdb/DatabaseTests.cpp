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
#include <cmath>

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

void DatabaseTests::addDatabaseConnection(const DatabaseConnectionString& connectionString)
{
    m_connectionStrings[connectionString.driverName()] = connectionString;
}

void DatabaseTests::testConnect(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    db->open();

    Strings objects;
    db->objectList(DOT_TABLES, objects);
    db->close();
}

void DatabaseTests::testDDL(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

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
    { "mysql",      "TIMESTAMP" },
    { "postgresql", "TIMESTAMP" },
    { "mssql",      "DATETIME" },
    { "oracle",     "TIMESTAMP" }
};

void DatabaseTests::testQueryParameters(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    auto itor = dateTimeFieldTypes.find(connectionString.driverName());
    if (itor == dateTimeFieldTypes.end())
        throw Exception("DateTime data type mapping is not defined for the test");
    String dateTimeType = itor->second;

    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table(id INT, name VARCHAR(20), price DECIMAL(10,2), ";
    createTableSQL << "ts " << dateTimeType << " NULL";
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
        insert.param("ts").setNull(VAR_DATE_TIME);
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

        if (std::round(row.price * 100) != round(select["price"].asFloat() * 100))
            throw Exception("row.price is " + select["price"].asString());
        select.next();
    }
    select.close();

    //dropTable.exec();
}

void DatabaseTests::testTransaction(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    db->open();
    Query createTable(db, "CREATE TABLE gtest_temp_table(id INT, name VARCHAR(20))");
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try { dropTable.exec(); } catch (...) {}

    createTable.exec();

    Transaction transaction(db);
    transaction.begin();

    size_t count = countRowsInTable(db, "gtest_temp_table");
    if (count != 0)
        throw Exception("initial count != 0");

    Query insert(db, "INSERT INTO gtest_temp_table VALUES('1', 'pear')");

    insert.exec();
    insert.exec();

    count = countRowsInTable(db, "gtest_temp_table");
    if (count != 2)
        throw Exception("count != 2");

    transaction.rollback();

    count = countRowsInTable(db, "gtest_temp_table");
    if (count != 0)
        throw Exception("count != 0");

    dropTable.exec();
}

DatabaseConnectionString DatabaseTests::connectionString(const String& driverName) const
{
    auto itor = m_connectionStrings.find(driverName);
    if (itor == m_connectionStrings.end())
        return DatabaseConnectionString("");
    return itor->second;
}

static const string expectedBulkInsertResult("1|Alex|Programmer|01-JAN-2014 # 2|David|CEO|01-JAN-2014 # 3|Roger|Bunny|01-JAN-2014");

void DatabaseTests::testBulkInsert(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    auto itor = dateTimeFieldTypes.find(connectionString.driverName());
    if (itor == dateTimeFieldTypes.end())
        throw Exception("DateTime data type mapping is not defined for the test");
    String dateTimeType = itor->second;

    db->open();
    Query createTable(db, "CREATE TABLE gtest_temp_table(id INTEGER,name CHAR(40),position_name CHAR(20),hire_date CHAR(12))");
    Query dropTable(db, "DROP TABLE gtest_temp_table");
    Query selectData(db, "SELECT * FROM gtest_temp_table");

    try { dropTable.exec(); } catch (...) {}

    createTable.exec();

    Strings data;
    data.push_back(string("1\tAlex\tProgrammer\t01-JAN-2014"));
    data.push_back(string("2\tDavid\tCEO\t01-JAN-2014"));
    data.push_back(string("3\tRoger\tBunny\t01-JAN-2014"));

    Strings columnNames("id,name,position_name,hire_date", ",");
    db->bulkInsert("gtest_temp_table", columnNames, data);

    selectData.open();
    Strings rows;
    while (!selectData.eof()) {
        Strings row;
        for (auto field: selectData.fields())
            row.push_back(field->asString().trim());
        rows.push_back(row.join("|"));
        selectData.next();
    }
    selectData.close();

    if (rows.size() > 3)
        throw Exception("Expected bulk insert result (3 rows) doesn't match table data (" + int2string(rows.size()) + ")");

    String actualResult(rows.join(" # "));
    if (actualResult != expectedBulkInsertResult)
        throw Exception("Expected bulk insert result doesn't match inserted data");
}

size_t DatabaseTests::countRowsInTable(DatabaseConnection db, const String& table)
{
    Query select(db, "SELECT count(*) cnt FROM " + table);
    select.open();
    size_t count = select["cnt"];
    select.close();

    return count;
}
