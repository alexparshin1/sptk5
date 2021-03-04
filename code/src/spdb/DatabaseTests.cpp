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
#include <sptk5/db/InsertQuery.h>

using namespace std;
using namespace sptk;
using namespace chrono;

DatabaseTests DatabaseTests::_databaseTests;

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

    for (int i = 0; i < 2; ++i) {
        DatabaseConnection db = connectionPool.getConnection();
        db->open();

        Strings objects;
        db->objectList(DOT_TABLES, objects);
        db->close();
    }
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
            CERR(e.what() << endl)
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

static const DateTime testDateTime("2015-06-01 11:22:33");

static const vector<Row> rows = {
        {1, "apple",      1.5,  testDateTime},
        {2, "pear",       3.1,  testDateTime},
        {3, "melon",      1.05, testDateTime},
        {4, "watermelon", 0.85, testDateTime},
        {5, "lemon",      5.5,  testDateTime}
};

static const map<String, String> dateTimeFieldTypes = {
        {"mysql",      "TIMESTAMP"},
        {"postgresql", "TIMESTAMP"},
        {"mssql",      "DATETIME2"},
        {"oracle",     "TIMESTAMP"}
};

static const map<String, String> boolFieldTypes = {
        {"mysql",      "BOOL"},
        {"postgresql", "BOOL"},
        {"mssql",      "BIT"},
        {"oracle",     "NUMBER(1)"}
};

static const map<String, String> textFieldTypes = {
        {"mysql",      "LONGTEXT"},
        {"postgresql", "TEXT"},
        {"mssql",      "NVARCHAR(MAX)"},
        {"oracle",     "CLOB"}
};

static String fieldType(const String& fieldType, const String& driverName)
{
    const map<String, String>* fieldTypes;
    if (fieldType == "DATETIME")
        fieldTypes = &dateTimeFieldTypes;
    else if (fieldType == "BOOL")
        fieldTypes = &boolFieldTypes;
    else
        fieldTypes = &textFieldTypes;

    auto itor = fieldTypes->find(driverName);
    if (itor == fieldTypes->end())
        throw Exception("Data type mapping is not defined for the test");
    return itor->second;
}

void DatabaseTests::testQueryInsertDate(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    auto itor = dateTimeFieldTypes.find(connectionString.driverName());
    if (itor == dateTimeFieldTypes.end())
        throw Exception("DateTime data type mapping is not defined for the test");
    String dateTimeType = itor->second;

    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table(ts " << dateTimeType << " NULL)";

    db->open();
    Query createTable(db, createTableSQL.str());
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try {
        dropTable.exec();
    }
    catch (const Exception& e) {
        CERR(e.what() << endl)
    }

    createTable.exec();

    Query insert1(db, "INSERT INTO gtest_temp_table VALUES('2015-06-01T11:22:33')");
    insert1.exec();
    Query insert2(db, "INSERT INTO gtest_temp_table VALUES(:dt)");
    insert2.param("dt") = DateTime("2015-06-01T11:22:33");
    insert2.exec();

#if USE_GTEST
    Query select(db, "SELECT ts FROM gtest_temp_table");
    select.open();
    EXPECT_TRUE(select["ts"].asDateTime().isoDateTimeString().startsWith("2015-06-01T11:22:33"));
    select.next();
    EXPECT_TRUE(select["ts"].asDateTime().isoDateTimeString().startsWith("2015-06-01T11:22:33"));
    select.close();
#endif
}

void DatabaseTests::testQueryParameters(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table( -- Create a test temp table" << endl;
    createTableSQL << "id INT, /* This is the unique id column */";
    createTableSQL << "name VARCHAR(20), price DECIMAL(10,2), ";
    createTableSQL << "ts " << fieldType("DATETIME", connectionString.driverName()) << " NULL, ";
    createTableSQL << "enabled " << fieldType("BOOL", connectionString.driverName()) << " NULL, ";
    createTableSQL << "txt " << fieldType("TEXT", connectionString.driverName()) << " NULL ";
    createTableSQL << ")";

    db->open();
    Query createTable(db, createTableSQL.str());
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try {
        dropTable.exec();
    }
    catch (const Exception& e) {
        CERR(e.what() << endl)
    }

    createTable.exec();

    Buffer clob;
    while (clob.length() < 65 * 1024) // A size of the CLOB that is bigger than 64K
        clob.append("A text");

    Query insert(db, "INSERT INTO gtest_temp_table VALUES(:id, :name, :price, :ts, :enabled, :txt)");
    for (auto& row: rows) {
        insert.param("id") = row.id;
        insert.param("name") = row.name;
        insert.param("price") = row.price;
        insert.param("ts").setNull(VAR_DATE_TIME);
        insert.param("enabled").setBool(true);
        insert.param("txt").setBuffer(clob.c_str(), clob.length(), VAR_TEXT);
        insert.exec();
    }

#if USE_GTEST
    Query select(db, "SELECT * FROM gtest_temp_table ORDER BY id");
    select.open();
    for (auto& row: rows) {
        if (select.eof())
            break;
        EXPECT_EQ(row.id, select["id"].asInteger());
        EXPECT_STREQ(row.name.c_str(), select["name"].asString().c_str());
        EXPECT_FLOAT_EQ(row.price, select["price"].asFloat());
        EXPECT_STREQ(clob.c_str(), select["txt"].asString().c_str());
        select.next();
    }
    select.close();
#endif
}

void DatabaseTests::testTransaction(DatabaseConnection db, bool commit)
{
    Query deleteRecords(db, "DELETE FROM gtest_temp_table");
    deleteRecords.exec();

    Transaction transaction(db);
    transaction.begin();

    Query insert(db, "INSERT INTO gtest_temp_table VALUES('1', 'pear')");

    size_t maxRecords = 100;

    for (unsigned i = 0; i < maxRecords; ++i)
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
        CERR(e.what() << endl)
    }

    createTable.exec();

    testTransaction(db, false);
    for (unsigned i = 0; i < 3; ++i)
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

void DatabaseTests::createTestTable(DatabaseConnection db)
{
    auto itor = dateTimeFieldTypes.find(db->connectionString().driverName());
    if (itor == dateTimeFieldTypes.end())
        throw Exception("DateTime data type mapping is not defined for the test");
    String dateTimeType = itor->second;

    db->open();
    Query createTable(db,
                      "CREATE TABLE gtest_temp_table("
                        "id INTEGER NULL, "
                        "name CHAR(40) NULL, "
                        "position_name CHAR(20) NULL, "
                        "hire_date CHAR(12) NULL)");
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try {
        dropTable.exec();
    }
    catch (const Exception& e) {
        RegularExpression matchTableNotExists("not exist|unknown table", "i");
        if (!matchTableNotExists.matches(e.what()))
        CERR(e.what() << endl)
    }

    createTable.exec();
}

void DatabaseTests::createTestTableWithSerial(DatabaseConnection db)
{
    auto itor = dateTimeFieldTypes.find(db->connectionString().driverName());
    if (itor == dateTimeFieldTypes.end())
        throw Exception("DateTime data type mapping is not defined for the test");
    String dateTimeType = itor->second;

    db->open();

    stringstream sql;
    String idDefinition;

    switch (db->connectionType()) {
        case DCT_MYSQL:
        case DCT_POSTGRES:
            idDefinition = "id serial";
            break;
        case DCT_MSSQL_ODBC:
            idDefinition = "id int identity";
            break;
        case DCT_ORACLE:
            idDefinition = "id int";
            break;
        default:
            throw DatabaseException("InsertQuery doesn't support " + db->driverDescription());
    }

    sql << "CREATE TABLE gtest_temp_table2(" << idDefinition << " primary key, name varchar(40))";

    Query createTable(db, sql.str());
    Query dropTable(db, "DROP TABLE gtest_temp_table2");

    try {
        dropTable.exec();
    }
    catch (const Exception& e) {
        RegularExpression matchTableNotExists("not exist|unknown table", "i");
        if (!matchTableNotExists.matches(e.what()))
        CERR(e.what() << endl)
    }

    createTable.exec();

    if (db->connectionType() == DCT_ORACLE)
        createOracleAutoIncrement(db, "gtest_temp_table2", "id");

    InsertQuery query(db, "INSERT INTO gtest_temp_table2(name) VALUES(:name)");

    query.param("name") = "Alex";
    query.exec();
    auto id = query.id();
#if USE_GTEST
    EXPECT_EQ(id, uint64_t(1));
#endif

    query.param("name") = "David";
    query.exec();
    id = query.id();
#if USE_GTEST
    EXPECT_EQ(id, uint64_t(2));
#endif
}

static const string expectedBulkInsertResult(
        "1|Alex,'Doe'|Programmer|01-JAN-2014 # 2|David|CEO|01-JAN-2015 # 3|Roger|Bunny|01-JAN-2016");

void DatabaseTests::testBulkInsert(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();
    createTestTable(db);

    Query selectData(db, "SELECT * FROM gtest_temp_table");

    vector<VariantVector> data;

    VariantVector arow;

    arow.emplace_back(1);
    arow.emplace_back("Alex,'Doe'");
    arow.emplace_back("Programmer");
    arow.emplace_back("01-JAN-2014");
    data.push_back(move(arow));

    arow.emplace_back(2);
    arow.emplace_back("David");
    arow.emplace_back("CEO");
    arow.emplace_back("01-JAN-2015");
    data.push_back(move(arow));

    arow.emplace_back(3);
    arow.emplace_back("Roger");
    arow.emplace_back("Bunny");
    arow.emplace_back("01-JAN-2016");
    data.push_back(move(arow));

    Strings columnNames("id,name,position_name,hire_date", ",");
    db->bulkInsert("gtest_temp_table", columnNames, data);

    selectData.open();
    Strings printRows;
    while (!selectData.eof()) {
        Strings row;
        for (const auto* field: selectData.fields())
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

void DatabaseTests::testInsertQuery(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();
    createTestTableWithSerial(db);
}

void DatabaseTests::testBulkInsertPerformance(const DatabaseConnectionString& connectionString, size_t recordCount)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();
    createTestTable(db);

    Query selectData(db, "SELECT * FROM gtest_temp_table");
    Query insertData(db, "INSERT INTO gtest_temp_table VALUES (:id, :name, :position, :hired)");

    vector<VariantVector> data;
    VariantVector arow;
    for (size_t i = 1; i <= recordCount; ++i) {
        arow.emplace_back(int(i));
        arow.emplace_back("Alex,'Doe'");
        arow.emplace_back("Programmer");
        arow.emplace_back("01-JAN-2014");
        data.push_back(move(arow));
    }

    Transaction transaction(db);

    transaction.begin();
    DateTime started1("now");
    Strings columnNames("id,name,position_name,hire_date", ",");
    db->bulkInsert("gtest_temp_table", columnNames, data);
    DateTime ended1("now");
    transaction.commit();

    DateTime started2("now");
    size_t i = 1;

    auto& idParam = insertData.param("id");
    auto& nameParam = insertData.param("name");
    auto& positionParam = insertData.param("position");
    auto& hiredParam = insertData.param("hired");

    transaction.begin();
    for (auto& row: data) {
        idParam = row[0].asInteger();
        nameParam = row[1].asString();
        positionParam = row[2].asString();
        hiredParam = row[3].asString();
        insertData.exec();
        ++i;
    }
    transaction.commit();
    DateTime ended2("now");

    auto durationMS2 = duration_cast<milliseconds>(ended2 - started2).count();
    COUT(left << setw(25) << connectionString.driverName() + " insert:"
        << right << setw(4) << durationMS2 << " ms, "
        << setprecision(1) << fixed << setw(8) << data.size() * 1000.0 / durationMS2 << " rec/sec" << endl)

    auto durationMS1 = duration_cast<milliseconds>(ended1 - started1).count();
    COUT(left << setw(25) << connectionString.driverName() + " bulk insert:"
        << right << setw(4) << durationMS1 << " ms, "
        << setprecision(1) << fixed << setw(8) << data.size() * 1000.0 / durationMS1 << " rec/sec" << endl)
}

void DatabaseTests::testSelect(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    testSelect(connectionPool);
}

void DatabaseTests::testSelect(DatabaseConnectionPool& connectionPool)
{
    DatabaseConnection db = connectionPool.getConnection();
    createTestTable(db);

    if (db->connectionType() == DCT_POSTGRES) {
        Query testNumeric(db, "SELECT (20/1000000.0)::numeric(8,6)");
        testNumeric.open();
        String numeric = testNumeric[size_t(0)].asString();
#if USE_GTEST
        EXPECT_STREQ(numeric.c_str(), "0.000020");
#endif
        testNumeric.close();
    }

    Query selectData(db, "SELECT * FROM gtest_temp_table");
    Query insertData(db, "INSERT INTO gtest_temp_table VALUES (:id, :name, :position, :hired)");

    Strings data;
    data.push_back(string("1\tAlex,'Doe'\tProgrammer\t01-JAN-2014"));
    data.push_back(string("2\tDavid\tCEO\t01-JAN-2015"));
    data.push_back(string("3\tRoger\tBunny\t01-JAN-2016"));

    for (const auto& row: data) {
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
        for (const auto* field: selectData.fields()) {
            if (!field->isNull())
                throw Exception("Field " + field->fieldName() + " = [" + field->asString() + "] but null is expected");
            VariantType expectedType = VAR_INT;
            if (column != 0)
                expectedType = VAR_STRING;
            if (field->dataType() != expectedType)
                throw Exception("Field " + field->fieldName() + " has data type " + to_string(field->dataType()) + " but expected " + to_string(expectedType));
            ++column;
        }
        selectData.next();

        Strings row;
        for (const auto* field: selectData.fields())
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

size_t DatabaseTests::countRowsInTable(const DatabaseConnection& db, const String& table)
{
    Query select(db, "SELECT count(*) cnt FROM " + table);
    select.open();
    size_t count = (size_t) select["cnt"].asInteger();
    select.close();

    return count;
}

DatabaseTests::DatabaseTests()
{
    escapeSQLString("x", false);
}

DatabaseTests& DatabaseTests::tests()
{
    return _databaseTests;
}

void DatabaseTests::createOracleAutoIncrement(const DatabaseConnection& db, const String& tableName, const String& columnName)
{
    string baseName = "id_" + tableName.substr(0,27);
    string sequenceName = "sq_" + baseName;
    string triggerName = "tr_" + baseName;

    try {
        Query dropSequence(db,"DROP SEQUENCE " + sequenceName);
        dropSequence.exec();
    }
    catch (const Exception& e)
    {
        COUT(e.what() << endl)
    }

    Query createSequence(db,"CREATE SEQUENCE " + sequenceName + " START WITH 1 INCREMENT BY 1 NOMAXVALUE");
    createSequence.exec();

    try {
        Query createTrigger(db,
                            "CREATE OR REPLACE TRIGGER " + triggerName + "\n" +
                            "BEFORE INSERT ON " + tableName + "\n" +
                            "FOR EACH ROW\n" +
                            "BEGIN\n" +
                            "  IF :new." + columnName + " IS NULL THEN\n" +
                            "    :new." + columnName + " := " + sequenceName + ".nextval;\n" +
                            "  END IF;\n" +
                            "END;\n"
        );
        createTrigger.exec();
    }
    catch (const Exception& e) {
        CERR(e.what() << endl)
    }
}
