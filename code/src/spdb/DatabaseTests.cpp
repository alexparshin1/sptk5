/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DatabaseTests.cpp - description                        ║
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

#include <sptk5/cutils>
#include <sptk5/db/DatabaseTests.h>
#include <sptk5/db/DatabaseConnectionPool.h>
#include <sptk5/db/Query.h>
#include <sptk5/db/Transaction.h>
#include <cmath>

using namespace std;
using namespace sptk;

DatabaseTests sptk::databaseTests;

vector<DatabaseConnectionString> DatabaseTests::connectionStrings() const
{
    vector<DatabaseConnectionString> connectionStrings;
    for (auto& itor: m_connectionStrings)
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
    catch (const Exception& e) {
        RegularExpression matchTableNotExists("not exist|unknown table", "i");
        if (!matchTableNotExists.matches(e.what()))
            CERR(e.what() << endl);
    }

    createTable.exec();
    dropTable.exec();

    db->close();
}

struct Row
{
    int id;
    string name;
    double price;
    DateTime ts;
};

static const vector<Row> rows = {
        {1, "apple",      1.5,  DateTime::Now()},
        {2, "pear",       3.1,  DateTime::Now()},
        {3, "melon",      1.05, DateTime()},
        {4, "watermelon", 0.85, DateTime::Now()},
        {5, "lemon",      5.5,  DateTime::Now()}
};

static const map<String, String> dateTimeFieldTypes = {
        {"mysql",      "TIMESTAMP"},
        {"postgresql", "TIMESTAMP"},
        {"mssql",      "DATETIME"},
        {"oracle",     "TIMESTAMP"}
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

    try {
        dropTable.exec();
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
    }

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
}

void DatabaseTests::testTransaction(DatabaseConnection db, bool commit)
{
    Query deleteRecords(db, "DELETE FROM gtest_temp_table");
    deleteRecords.exec();

    Transaction transaction(db);
    transaction.begin();

    Query insert(db, "INSERT INTO gtest_temp_table VALUES('1', 'pear')");

    size_t maxRecords = 100;

    for (unsigned i = 0; i < maxRecords; i++)
        insert.exec();

    auto count = countRowsInTable(db, "gtest_temp_table");
    if (count != maxRecords)
        throw Exception("count " + to_string(count) + " != " + to_string(maxRecords));

    if (commit) {
        transaction.commit();

        count = countRowsInTable(db, "gtest_temp_table");
        if (count != maxRecords)
            throw Exception("count != " + to_string(maxRecords) + " after commit)");
    } else {
        transaction.rollback();

        count = countRowsInTable(db, "gtest_temp_table");
        if (count != 0)
            throw Exception("count != 0 (after rollback)");
    }
}

void DatabaseTests::testTransaction(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    db->open();
    Query createTable(db, "CREATE TABLE gtest_temp_table(id INT, name VARCHAR(20))");
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try {
        dropTable.exec();
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
    }

    createTable.exec();

    testTransaction(db, false);
    for (unsigned i = 0; i < 3; i++)
        testTransaction(db, true);

    dropTable.exec();
}

DatabaseConnectionString DatabaseTests::connectionString(const String& driverName) const
{
    auto itor = m_connectionStrings.find(driverName);
    if (itor == m_connectionStrings.end())
        return DatabaseConnectionString("");
    return itor->second;
}

static const string expectedBulkInsertResult(
        "1|Alex|Programmer|01-JAN-2014 # 2|David|CEO|01-JAN-2014 # 3|Roger|Bunny|01-JAN-2014");

void DatabaseTests::testBulkInsert(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    auto itor = dateTimeFieldTypes.find(connectionString.driverName());
    if (itor == dateTimeFieldTypes.end())
        throw Exception("DateTime data type mapping is not defined for the test");
    String dateTimeType = itor->second;

    db->open();
    Query createTable(db,
                      "CREATE TABLE gtest_temp_table(id INTEGER,name CHAR(40),position_name CHAR(20),hire_date CHAR(12))");
    Query dropTable(db, "DROP TABLE gtest_temp_table");
    Query selectData(db, "SELECT * FROM gtest_temp_table");

    try {
        dropTable.exec();
    }
    catch (const Exception& e) {
        RegularExpression matchTableNotExists("not exist|unknown table", "i");
        if (!matchTableNotExists.matches(e.what()))
            CERR(e.what() << endl);
    }

    createTable.exec();

    Strings data;
    data.push_back(string("1\tAlex\tProgrammer\t01-JAN-2014"));
    data.push_back(string("2\tDavid\tCEO\t01-JAN-2014"));
    data.push_back(string("3\tRoger\tBunny\t01-JAN-2014"));

    Strings columnNames("id,name,position_name,hire_date", ",");
    db->bulkInsert("gtest_temp_table", columnNames, data);

    selectData.open();
    Strings printRows;
    while (!selectData.eof()) {
        Strings row;
        for (auto* field: selectData.fields())
            row.push_back(field->asString().trim());
        printRows.push_back(row.join("|"));
        selectData.next();
    }
    selectData.close();

    if (printRows.size() > 3)
        throw Exception(
                "Expected bulk insert result (3 rows) doesn't match table data (" + int2string(printRows.size()) + ")");

    String actualResult(printRows.join(" # "));
    if (actualResult != expectedBulkInsertResult)
        throw Exception("Expected bulk insert result doesn't match inserted data");
}

void DatabaseTests::testSelect(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    auto itor = dateTimeFieldTypes.find(connectionString.driverName());
    if (itor == dateTimeFieldTypes.end())
        throw Exception("DateTime data type mapping is not defined for the test");
    String dateTimeType = itor->second;

    db->open();
    Query createTable(db,
                      "CREATE TABLE gtest_temp_table(id INTEGER NULL, name CHAR(40) NULL, position_name CHAR(20) NULL, hire_date CHAR(12) NULL)");
    Query dropTable(db, "DROP TABLE gtest_temp_table");
    Query insertData(db, "INSERT INTO gtest_temp_table VALUES (:id, :name, :position, :hired)");
    Query selectData(db, "SELECT * FROM gtest_temp_table");

    try {
        dropTable.exec();
    }
    catch (const Exception& e) {
        RegularExpression matchTableNotExists("not exist|unknown table", "i");
        if (!matchTableNotExists.matches(e.what()))
            CERR(e.what() << endl);
    }

    createTable.exec();

    Strings data;
    data.push_back(string("1\tAlex\tProgrammer\t01-JAN-2014"));
    data.push_back(string("2\tDavid\tCEO\t01-JAN-2014"));
    data.push_back(string("3\tRoger\tBunny\t01-JAN-2014"));

    for (auto& row: data) {
        // Insert all nulls
        insertData.param("id").setNull(VAR_INT);
        insertData.param("name").setNull(VAR_STRING);
        insertData.param("position").setNull(VAR_STRING);
        insertData.param("hired").setNull(VAR_STRING);
        insertData.exec();

        // Insert data row
        Strings values(row, "\t");
        insertData.param("id") = string2int(values[0]);
        insertData.param("name") = values[1];
        insertData.param("position") = values[2];
        insertData.param("hired") = values[3];
        insertData.exec();
    }

    selectData.open();
    Strings printRows;
    while (!selectData.eof()) {
        // Check if all fields are NULLs
        int column = 0;
        for (auto* field: selectData.fields()) {
            if (!field->isNull())
                throw Exception("Field " + field->fieldName() + " = [" + field->asString() + "] but null is expected");
            VariantType expectedType = VAR_INT;
            if (column != 0)
                expectedType = VAR_STRING;
            if (field->dataType() != expectedType)
                throw Exception("Field " + field->fieldName() + " has data type " + to_string(field->dataType()) + " but expected " + to_string(expectedType));
            column++;
        }
        selectData.next();

        Strings row;
        for (auto* field: selectData.fields())
            row.push_back(field->asString().trim());
        printRows.push_back(row.join("|"));
        selectData.next();
    }
    selectData.close();

    if (printRows.size() > 3)
        throw Exception(
                "Expected result (3 rows) doesn't match table data (" + int2string(printRows.size()) + ")");

    String actualResult(printRows.join(" # "));
    if (actualResult != expectedBulkInsertResult)
        throw Exception("Expected result doesn't match inserted data");
}

size_t DatabaseTests::countRowsInTable(DatabaseConnection& db, const String& table)
{
    Query select(db, "SELECT count(*) cnt FROM " + table);
    select.open();
    size_t count = (size_t) select["cnt"].asInteger();
    select.close();

    return count;
}
