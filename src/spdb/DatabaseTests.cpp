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

using namespace std;
using namespace sptk;

DatabaseTests sptk::databaseTests;

DatabaseTests::DatabaseTests()
{
    DatabaseConnectionPool connectionPool("postgresql://localhost/postgres");
}

const std::vector<DatabaseConnectionString>& DatabaseTests::connectionStrings() const
{
    return m_connectionStrings;
}

void DatabaseTests::addConnection(const DatabaseConnectionString& connectionString)
{
    m_connectionStrings.push_back(connectionString);
}

#if USE_GTEST
#include <gtest/gtest.h>

TEST(Database, open)
{
    for (auto& connectionString: databaseTests.connectionStrings()) {
        DatabaseConnectionPool connectionPool(connectionString.toString());
        DatabaseConnection* db = connectionPool.createConnection();
        try {
            db->open();
        }
        catch (const DatabaseException& e) {
            FAIL() << e.what();
        }
        if (db->active()) {
            Strings objects;
            EXPECT_NO_THROW(db->objectList(DOT_TABLES, objects));
            EXPECT_NO_THROW(db->close());
        }
        connectionPool.destroyConnection(db);
    }
}

TEST(Database, execDDL)
{
    for (auto& connectionString: databaseTests.connectionStrings()) {
        DatabaseConnectionPool connectionPool(connectionString.toString());
        DatabaseConnection* db = connectionPool.createConnection();
        try {
            db->open();
            Query createTable(db, "CREATE TEMP TABLE gtest_temp_table(id INT, name VARCHAR(20))");
            createTable.exec();
            db->close();
        }
        catch (const DatabaseException& e) {
            FAIL() << e.what();
        }
        connectionPool.destroyConnection(db);
    }
}

struct Row {
    int     id;
    string  name;
    double  price;
};

static vector<Row> rows = {
    { 1, "apple", 1.5 },
    { 2, "pear",  3.1 },
    { 3, "melon", 1.05 },
    { 4, "watermelon", 0.85 },
    { 5, "lemon", 5.5 }
};

TEST(Database, queryParameters)
{
    for (auto& connectionString: databaseTests.connectionStrings()) {
        DatabaseConnectionPool connectionPool(connectionString.toString());
        DatabaseConnection* db = connectionPool.createConnection();
        try {
            db->open();
            Query createTable(db, "CREATE TEMP TABLE gtest_temp_table(id INT, name VARCHAR(20), price DECIMAL(10,2))");
            createTable.exec();

            Query insert(db, "INSERT INTO gtest_temp_table VALUES(:id, :name, :price)");
            for (auto& row: rows) {
                insert.param("id") = row.id;
                insert.param("name") = row.name;
                insert.param("price") = row.price;
                insert.exec();
            }

            Query select(db, "SELECT * FROM gtest_temp_table");
            select.open();
            for (auto& row: rows) {
                if (select.eof())
                    break;
                EXPECT_EQ(row.id, select["id"].asInteger());
                EXPECT_STREQ(row.name.c_str(), select["name"].asString().c_str());
                EXPECT_DOUBLE_EQ(row.price, select["price"].asFloat());
                select.next();
            }
            select.close();

            db->close();
        }
        catch (const DatabaseException& e) {
            FAIL() << e.what();
        }
        connectionPool.destroyConnection(db);
    }
}

TEST(Database, transaction)
{
    for (auto& connectionString: databaseTests.connectionStrings()) {
        DatabaseConnectionPool connectionPool(connectionString.toString());
        DatabaseConnection* db = connectionPool.createConnection();
        try {
            db->open();
            Query createTable(db, "CREATE TABLE gtest_temp_table(id INT, name VARCHAR(20))");
            createTable.exec();

            db->beginTransaction();
            Query insert(db, "INSERT INTO gtest_temp_table VALUES('1', 'pear')");

            EXPECT_NO_THROW(insert.exec());
            EXPECT_NO_THROW(insert.exec());

            Query select(db, "SELECT count(*) cnt FROM gtest_temp_table");
            EXPECT_NO_THROW(select.open());
            size_t count = select["cnt"];
            EXPECT_EQ(size_t(2), count);
            select.close();

            db->rollbackTransaction();

            EXPECT_NO_THROW(select.open());
            count = select["cnt"];
            EXPECT_EQ(size_t(0), count);
            select.close();

            Query dropTable(db, "DROP TABLE gtest_temp_table");
            dropTable.exec();

            db->close();
        }
        catch (const DatabaseException& e) {
            FAIL() << e.what();
        }
        connectionPool.destroyConnection(db);
    }
}

#endif
