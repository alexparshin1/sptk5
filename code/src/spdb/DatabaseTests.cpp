/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/db/DatabaseConnectionPool.h>
#include <sptk5/db/DatabaseTests.h>
#include <sptk5/db/Query.h>
#include <sptk5/db/Transaction.h>

#include <sptk5/db/InsertQuery.h>

using namespace std;
using namespace sptk;
using namespace chrono;

DatabaseTests DatabaseTests::_databaseTests;

vector<DatabaseConnectionString> DatabaseTests::connectionStrings() const
{
    vector<DatabaseConnectionString> connectionStrings;
    for (const auto& [name, connectionString]: m_connectionStrings)
    {
        connectionStrings.push_back(connectionString);
    }
    return connectionStrings;
}

void DatabaseTests::addDatabaseConnection(const DatabaseConnectionString& connectionString)
{
    m_connectionStrings[connectionString.driverName()] = connectionString;
}

void DatabaseTests::testConnect(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());

    for (int i = 0; i < 2; ++i)
    {
        DatabaseConnection db = connectionPool.getConnection();
#ifdef USE_GTEST
        EXPECT_STREQ(db->connectionString().toString().c_str(), connectionString.toString().c_str());
#endif
        db->open();
#ifdef USE_GTEST
        EXPECT_TRUE(db->active());
#endif

        if (auto info = db->driverDescription(); info.length() < 10)
        {
            throw DatabaseException("Driver info is empty");
        }

        Strings objects;
        db->objectList(DatabaseObjectType::TABLES, objects);
        db->objectList(DatabaseObjectType::DATABASES, objects);
        db->objectList(DatabaseObjectType::FUNCTIONS, objects);
        db->objectList(DatabaseObjectType::PROCEDURES, objects);
        db->objectList(DatabaseObjectType::VIEWS, objects);
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

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        RegularExpression matchTableNotExists("not exist|unknown table", "i");
        if (!matchTableNotExists.matches(e.what()))
            CERR(e.what() << endl)
    }

    createTable.exec();
    dropTable.exec();

    db->close();
}

struct Row {
    int id;
    int64_t ssid;
    string name;
    double price;
    DateTime ts;
};

static const DateTime testDateTime("2015-06-01 11:22:33");

static const vector<Row> rows = {
    {1, 1234567, "apple", 1.5, testDateTime},
    {2, 1234567, "pear", 3.1, testDateTime},
    {3, 1234567, "melon", 1.05, testDateTime},
    {4, 1234567, "watermelon", 0.85, testDateTime},
    {5, 1234567, "lemon", 5.5, testDateTime}};

static const map<String, String> dateFieldTypes = {
    {"mysql", "DATE"},
    {"postgresql", "DATE"},
    {"mssql", "DATE"},
    {"oracle", "DATE"},
    {"sqlite3", "VARCHAR(10)"}};

static const map<String, String> dateTimeFieldTypes = {
    {"mysql", "TIMESTAMP"},
    {"postgresql", "TIMESTAMP"},
    {"mssql", "DATETIME2"},
    {"oracle", "TIMESTAMP"},
    {"sqlite3", "VARCHAR(30)"}};

static const map<String, String> boolFieldTypes = {
    {"mysql", "BOOL"},
    {"postgresql", "BOOL"},
    {"mssql", "BIT"},
    {"oracle", "NUMBER(1)"},
    {"sqlite3", "INT"},
};

static const map<String, String> textFieldTypes = {
    {"mysql", "LONGTEXT"},
    {"postgresql", "TEXT"},
    {"mssql", "NVARCHAR(MAX)"},
    {"oracle", "CLOB"},
    {"sqlite3", "TEXT"}};

static const map<String, String> blobFieldTypes = {
    {"mysql", "LONGBLOB"},
    {"postgresql", "BYTEA"},
    {"mssql", "VARBINARY(MAX)"},
    {"oracle", "BLOB"},
    {"sqlite3", "TEXT"}};

static String fieldType(const String& fieldType, const String& driverName)
{
    const map<String, String>* fieldTypes = &textFieldTypes;
    if (fieldType == "DATETIME")
    {
        fieldTypes = &dateTimeFieldTypes;
    }
    else if (fieldType == "DATE")
    {
        fieldTypes = &dateFieldTypes;
    }
    else if (fieldType == "BOOL")
    {
        fieldTypes = &boolFieldTypes;
    }

    auto itor = fieldTypes->find(driverName);
    if (itor == fieldTypes->end())
    {
        throw Exception("Data type mapping is not defined for the test");
    }
    return itor->second;
}

void DatabaseTests::testQueryInsertDate(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    auto itor = dateFieldTypes.find(connectionString.driverName());
    if (itor == dateFieldTypes.end())
    {
        throw Exception("Date data type mapping is not defined for the test");
    }
    String dateTimeType = itor->second;

    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table(ts " << dateTimeType << " NULL)";

    db->open();
    Query createTable(db, createTableSQL.str());
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
    }

    createTable.exec();

    String testDate = db->connectionType() == DatabaseConnectionType::ORACLE ? "01-JUN-2015"
                                                                             : "2015-06-01";
    Query insert1(db, "INSERT INTO gtest_temp_table VALUES('" + testDate + "')");
    insert1.exec();
    Query insert2(db, "INSERT INTO gtest_temp_table VALUES(:dt)");

    DateTime dateTime("2015-06-01");
    Variant date;
    date.setDateTime(dateTime, true);
    insert2.param("dt") = date;
    insert2.exec();

#ifdef USE_GTEST
    Query select(db, "SELECT ts FROM gtest_temp_table");
    select.open();
    EXPECT_TRUE(select["ts"].asDateTime().isoDateTimeString().startsWith("2015-06-01"));
    select.next();
    EXPECT_TRUE(select["ts"].asDateTime().isoDateTimeString().startsWith("2015-06-01"));
    select.close();
#endif
}

void DatabaseTests::testQueryInsertDateTime(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    auto itor = dateTimeFieldTypes.find(connectionString.driverName());
    if (itor == dateTimeFieldTypes.end())
    {
        throw Exception("DateTime data type mapping is not defined for the test");
    }
    String dateTimeType = itor->second;

    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table(ts " << dateTimeType << " NULL)";

    db->open();
    Query createTable(db, createTableSQL.str());
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
    }

    createTable.exec();

    String testDate = db->connectionType() == DatabaseConnectionType::ORACLE ? "01-JUN-2015 11:22:33"
                                                                             : "2015-06-01T11:22:33";
    Query insert1(db, "INSERT INTO gtest_temp_table VALUES('" + testDate + "')");
    insert1.exec();
    Query insert2(db, "INSERT INTO gtest_temp_table VALUES(:dt)");
    insert2.param("dt") = DateTime("2015-06-01T11:22:33");
    insert2.exec();

#ifdef USE_GTEST
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
    createTableSQL << "ssid INT, ";
    createTableSQL << "name VARCHAR(20), price DECIMAL(10,2), ";
    createTableSQL << "ts " << fieldType("DATETIME", connectionString.driverName()) << " NULL, ";
    createTableSQL << "enabled " << fieldType("BOOL", connectionString.driverName()) << " NULL, ";
    createTableSQL << "txt " << fieldType("TEXT", connectionString.driverName()) << " NULL ";
    createTableSQL << ")";

    db->open();
    Query createTable(db, createTableSQL.str());
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
    }

    createTable.exec();

    Buffer clob;
    while (clob.length() < 65536)
    { // A size of the CLOB that is bigger than 64K
        clob.append("A text");
    }

    Query insert(db, "INSERT INTO gtest_temp_table VALUES(:id, :ssid, :name, :price, :ts, :enabled, :txt)");
    for (const auto& row: rows)
    {
        insert.param("id") = row.id;
        insert.param("ssid") = row.ssid;
        insert.param("name") = row.name;
        insert.param("price") = row.price;
        insert.param("ts") = DateTime::Now();
        insert.param("enabled").setBool(true);
        insert.param("txt").setBuffer((const uint8_t*) clob.data(), clob.length(), VariantDataType::VAR_TEXT);
        insert.exec();
    }

    Variant id;
    id.setImagePtr((const uint8_t*) &id);
    insert.param("id") = id;
    try
    {
        insert.exec();
#ifdef USE_GTEST
        FAIL() << "Unsupported parameter type not detected";
#endif
    }
    catch (const DatabaseException& e)
    {
        if (String(e.what()).find("Unsupported parameter type") == String::npos)
        {
#ifdef USE_GTEST
            FAIL() << e.what();
#endif
        }
    }

#ifdef USE_GTEST
    Query select(db, "SELECT * FROM gtest_temp_table ORDER BY id");
    select.open();
    for (const auto& row: rows)
    {
        if (select.eof())
        {
            break;
        }
        EXPECT_EQ(row.id, select["id"].asInteger());
        EXPECT_EQ(row.ssid, select["ssid"].asInt64());
        EXPECT_STREQ(row.name.c_str(), select["name"].asString().c_str());
        EXPECT_DOUBLE_EQ(row.price, select["price"].asFloat());
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

#ifdef USE_GTEST
    EXPECT_THROW(transaction.commit(), DatabaseException);
    EXPECT_THROW(transaction.rollback(), DatabaseException);
#endif

    transaction.begin();

    Query insert(db, "INSERT INTO gtest_temp_table VALUES('1', 'pear')");

    size_t maxRecords = 100;

    for (unsigned i = 0; i < maxRecords; ++i)
    {
        insert.exec();
    }

    auto count = countRowsInTable(db, "gtest_temp_table");
    if (count != maxRecords)
    {
        throw Exception("count " + to_string(count) + " != " + to_string(maxRecords));
    }

    if (commit)
    {
        transaction.commit();

        count = countRowsInTable(db, "gtest_temp_table");
        if (count != maxRecords)
        {
            throw Exception("count != " + to_string(maxRecords) + " after commit)");
        }
    }
    else
    {
        transaction.rollback();

        count = countRowsInTable(db, "gtest_temp_table");
        if (count != 0)
        {
            throw Exception("count != 0 (after rollback)");
        }
    }

#ifdef USE_GTEST
    EXPECT_THROW(transaction.commit(), DatabaseException);
#endif
}

void DatabaseTests::testTransaction(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();

    db->open();
    Query createTable(db, "CREATE TABLE gtest_temp_table(id INT, name VARCHAR(20))");
    Query dropTable(db, "DROP TABLE gtest_temp_table");

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
    }

    createTable.exec();

    testTransaction(db, false);
    for (unsigned i = 0; i < 3; ++i)
    {
        testTransaction(db, true);
    }

    dropTable.exec();
}

DatabaseConnectionString DatabaseTests::connectionString(const String& driverName) const
{
    auto itor = m_connectionStrings.find(driverName);
    if (itor == m_connectionStrings.end())
    {
        return DatabaseConnectionString("");
    }
    return itor->second;
}

void DatabaseTests::createTestTable(DatabaseConnection db, bool autoPrepare, bool withBlob)
{
    auto itor = blobFieldTypes.find(db->connectionString().driverName());
    if (itor == blobFieldTypes.end())
    {
        throw Exception("BLOB data type mapping is not defined for the test");
    }
    String blobType = itor->second;

    Strings fields {"id INTEGER NULL", "name CHAR(40) NULL", "position_name CHAR(20) NULL", "hire_date CHAR(12) NULL"};

    if (withBlob)
    {
        fields.push_back("data1 " + blobType + " NULL");
        fields.push_back("data2 " + blobType + " NULL");
    }

    String sql("CREATE TABLE gtest_temp_table(" + fields.join(", ") + ")");

    db->open();
    Query createTable(db, sql, autoPrepare);
    Query dropTable(db, "DROP TABLE gtest_temp_table", autoPrepare);

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
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
    {
        throw Exception("DateTime data type mapping is not defined for the test");
    }
    String dateTimeType = itor->second;

    db->open();

    stringstream sql;
    String idDefinition;

    switch (db->connectionType())
    {
        case DatabaseConnectionType::MYSQL:
        case DatabaseConnectionType::POSTGRES:
            idDefinition = "id serial";
            break;
        case DatabaseConnectionType::MSSQL_ODBC:
            idDefinition = "id int identity";
            break;
        case DatabaseConnectionType::ORACLE:
            idDefinition = "id int";
            break;
        case DatabaseConnectionType::FIREBIRD:
        case DatabaseConnectionType::SQLITE3:
        case DatabaseConnectionType::GENERIC_ODBC:
        case DatabaseConnectionType::UNKNOWN:
            throw DatabaseException("InsertQuery doesn't support " + db->driverDescription());
    }

    sql << "CREATE TABLE gtest_temp_table2(" << idDefinition << " primary key, name varchar(40))";

    Query createTable(db, sql.str());
    Query dropTable(db, "DROP TABLE gtest_temp_table2");

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        RegularExpression matchTableNotExists("not exist|unknown table", "i");
        if (!matchTableNotExists.matches(e.what()))
            CERR(e.what() << endl)
    }

    createTable.exec();

    if (db->connectionType() == DatabaseConnectionType::ORACLE)
    {
        createOracleAutoIncrement(db, "gtest_temp_table2", "id");
    }

    InsertQuery query(db, "INSERT INTO gtest_temp_table2(name) VALUES(:name)");

    query.param("name") = "Alex";
    query.exec();
#ifdef USE_GTEST
    auto id = query.id();
    EXPECT_EQ(id, uint64_t(1));
#endif

    query.param("name") = "David";
    query.exec();
#ifdef USE_GTEST
    id = query.id();
    EXPECT_EQ(id, uint64_t(2));
#endif
}

static const string expectedBulkInsertResult(
    "1|Alex,'Doe'|Programmer|01-JAN-2014 # 2|David|CEO|01-JAN-2015 # 3|Roger|Bunny|01-JAN-2016");

void DatabaseTests::testBulkInsert(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();
    createTestTable(db, false, false);

    Query selectData(db, "SELECT * FROM gtest_temp_table");

    vector<VariantVector> data;

    VariantVector arow;

    arow.emplace_back(1);
    arow.emplace_back("Alex,'Doe'");
    arow.emplace_back("Programmer");
    arow.emplace_back("01-JAN-2014");
    data.push_back(arow);

    arow.clear();
    arow.emplace_back(2);
    arow.emplace_back("David");
    arow.emplace_back("CEO");
    arow.emplace_back("01-JAN-2015");
    data.push_back(arow);

    arow.clear();
    arow.emplace_back(3);
    arow.emplace_back("Roger");
    arow.emplace_back("Bunny");
    arow.emplace_back("01-JAN-2016");
    data.push_back(arow);

    Strings columnNames("id,name,position_name,hire_date", ",");
    db->bulkInsert("gtest_temp_table", columnNames, data);

    selectData.open();
    Strings printRows;
    while (!selectData.eof())
    {
        Strings row;
        for (const auto& field: selectData.fields())
        {
            row.push_back(field->asString().trim());
        }
        printRows.push_back(row.join("|"));
        selectData.next();
    }
    selectData.close();

    if (printRows.size() > 3)
    {
        throw Exception(
            "Expected bulk insert result (3 rows) doesn't match table data (" + int2string(printRows.size()) + ")");
    }

    String actualResult(printRows.join(" # "));
    if (actualResult != expectedBulkInsertResult)
    {
        throw Exception("Expected bulk insert result doesn't match inserted data");
    }
}

void DatabaseTests::testInsertQuery(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();
    createTestTableWithSerial(db);
}

void DatabaseTests::testInsertQueryDirect(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();
    createTestTable(db, false, false);

    Query insert(db, "INSERT INTO gtest_temp_table(id, name, position_name, hire_date)"
                     "VALUES (1, 'John Doe', 'engineer', '2020-01-02')",
                 false);
    insert.exec();

    insert.sql("INSERT INTO gtest_temp_table(id, name, position_name, hire_date)"
               "VALUES (2, 'Jane Doe', 'CFO', '2020-02-03')");
    insert.exec();

    Query select(db, "SELECT * FROM gtest_temp_table ORDER BY 1", false);
    select.open();
#ifdef USE_GTEST
    size_t recordCount = 0;
    while (!select.eof())
    {
        ++recordCount;
        switch (recordCount)
        {
            case 1:
                EXPECT_EQ(1L, select[0].asInteger());
                EXPECT_STREQ("John Doe", select[1].asString().trim().c_str());
                break;
            case 2:
                EXPECT_EQ(2L, select[0].asInteger());
                EXPECT_STREQ("Jane Doe", select[1].asString().trim().c_str());
                break;
            default:
                break;
        }
        select.next();
    }
    EXPECT_EQ(2U, recordCount);
#endif
}

void DatabaseTests::testBulkInsertPerformance(const DatabaseConnectionString& connectionString, size_t recordCount)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();
    createTestTable(db, false, false);

    Query selectData(db, "SELECT * FROM gtest_temp_table");
    Query insertData(db, "INSERT INTO gtest_temp_table VALUES (:id, :name, :position, :hired)");

    vector<VariantVector> data;
    for (size_t i = 1; i <= recordCount; ++i)
    {
        VariantVector arow;
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
    for (auto& row: data)
    {
        idParam = row[0].asInteger();
        nameParam = row[1].asString();
        positionParam = row[2].asString();
        hiredParam = row[3].asString();
        insertData.exec();
        ++i;
    }
    transaction.commit();
    DateTime ended2("now");

    auto durationMS2 = static_cast<double>(duration_cast<milliseconds>(ended2 - started2).count());
    COUT(left << setw(25) << connectionString.driverName() + " insert:"
              << right << setw(4) << durationMS2 << " ms, "
              << setprecision(1) << fixed << setw(8) << static_cast<double>(data.size()) * 1000.0 / durationMS2 << " rec/sec" << endl)

    auto durationMS1 = static_cast<double>(duration_cast<milliseconds>(ended1 - started1).count());
    COUT(left << setw(25) << connectionString.driverName() + " bulk insert:"
              << right << setw(4) << durationMS1 << " ms, "
              << setprecision(1) << fixed << setw(8) << static_cast<double>(data.size()) * 1000.0 / durationMS1 << " rec/sec" << endl)
}

void DatabaseTests::testBatchSQL(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();
    createTestTable(db, false, false);

    Query selectData(db, "SELECT * FROM gtest_temp_table ORDER BY 1");

    Strings batchSQL {
        "INSERT INTO gtest_temp_table VALUES (1, 'Jonh', 'CEO', '2020-01-02');",
        "INSERT INTO gtest_temp_table VALUES (2, 'Jane', 'CFO', '2021-02-03');",
        "INSERT INTO gtest_temp_table VALUES (3, 'William', 'CIO', '2022-03-04');"};

    Strings invalidBatchSQL {
        "REMOVE INTO gtest_temp_table VALUES (2, 'Jane', 'CFO', '2021-02-03');",
        "INVENT INTO gtest_temp_table VALUES (3, 'William', 'CIO', '2022-03-04');"};

    Strings expectedResults {
        "1,Jonh,CEO,2020-01-02",
        "2,Jane,CFO,2021-02-03",
        "3,William,CIO,2022-03-04"};

#ifdef USE_GTEST
    EXPECT_THROW(db->executeBatchSQL(invalidBatchSQL), DatabaseException);
#endif

    db->executeBatchSQL(batchSQL);

    selectData.open();
    int i = 0;
    for (; i < 3 && !selectData.eof(); i++)
    {
        Strings row;
        for (size_t column = 0; column < selectData.fieldCount(); ++column)
        {
            row.push_back(selectData[column].asString().trim());
        }
#ifdef USE_GTEST
        EXPECT_STREQ(expectedResults[i].c_str(), row.join(",").c_str());
#endif
        selectData.next();
    }
#ifdef USE_GTEST
    EXPECT_EQ(i, 3);
#endif
}

void DatabaseTests::testSelect(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    testSelect(connectionPool);
}

void DatabaseTests::testSelect(DatabaseConnectionPool& connectionPool)
{
    DatabaseConnection db = connectionPool.getConnection();
    createTestTable(db, false, false);

    Query emptyQuery(db);
#ifdef USE_GTEST
    EXPECT_THROW(emptyQuery.exec(), DatabaseException);
#endif

    Query selectData(db, "SELECT * FROM gtest_temp_table");
    Query insertData(db, "INSERT INTO gtest_temp_table VALUES (:id, :name, :position, :hired)");

    Strings data;
    data.push_back(string("1\tAlex,'Doe'\tProgrammer\t01-JAN-2014"));
    data.push_back(string("2\tDavid\tCEO\t01-JAN-2015"));
    data.push_back(string("3\tRoger\tBunny\t01-JAN-2016"));

    for (const auto& row: data)
    {
        // Insert all nulls
        insertData.param("id").setNull(VariantDataType::VAR_INT);
        insertData.param("name").setNull(VariantDataType::VAR_STRING);
        insertData.param("position").setNull(VariantDataType::VAR_STRING);
        insertData.param("hired").setNull(VariantDataType::VAR_STRING);
        insertData.exec();

        // Insert data row
        Strings values(row, "\t");
        insertData.param("id") = string2int(values[0]);
        insertData.param("name") = values[1];
        insertData.param("position") = values[2];
        insertData.param("hired") = values[3];
        insertData.exec();
    }

#ifdef USE_GTEST
    EXPECT_THROW(selectData.next(), DatabaseException);
#endif

    selectData.open();
    Strings printRows;
    while (!selectData.eof())
    {
        // Check if all fields are NULLs
        int column = 0;
        for (const auto& field: selectData.fields())
        {
            if (!field->isNull())
            {
                throw Exception("Field " + field->fieldName() + " = [" + field->asString() + "] but null is expected");
            }
            ++column;
        }
        selectData.next();

        Strings row;
        for (const auto& field: selectData.fields())
        {
            row.push_back(field->asString().trim());
        }
        printRows.push_back(row.join("|"));
        selectData.next();
    }

#ifdef USE_GTEST
    EXPECT_THROW(selectData.next(), DatabaseException);
#endif

    selectData.close();

    if (printRows.size() > 3)
    {
        throw Exception(
            "Expected result (3 rows) doesn't match table data (" + int2string(printRows.size()) + ")");
    }

    String actualResult(printRows.join(" # "));
    if (actualResult != expectedBulkInsertResult)
    {
        throw Exception("Expected result doesn't match inserted data");
    }
}

size_t DatabaseTests::countRowsInTable(const DatabaseConnection& db, const String& table)
{
    Query select(db, "SELECT count(*) cnt FROM " + table);
    select.open();
    auto count = (size_t) select["cnt"].asInteger();
    select.close();

    return count;
}

void DatabaseTests::testBLOB(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    DatabaseConnection db = connectionPool.getConnection();
    createTestTable(db, true, true);

    constexpr size_t blobSize1 = 65536;

    Buffer testData1(blobSize1);
    Buffer testDataInv(blobSize1);
    for (size_t i = 0; i < blobSize1; i++)
    {
        testData1[i] = static_cast<uint8_t>(i % 256);
        testDataInv[i] = static_cast<uint8_t>(256 - i % 256);
    }
    testData1.bytes(blobSize1);
    testDataInv.bytes(blobSize1);

    Buffer testData2(testDataInv);
    testData2.append(testDataInv);

    Query insertQuery(db, "INSERT INTO gtest_temp_table(id,data1, data2) VALUES(:id, :data1, :data2)");
    insertQuery.param("id") = 1;
    insertQuery.param("data1") = testData1;
    insertQuery.param("data2") = testData2;
    insertQuery.exec();

    Query selectQuery(db, "SELECT id, data1, data2 FROM gtest_temp_table ORDER BY 1");
    selectQuery.open();

#ifdef USE_GTEST
    constexpr size_t blobSize2 = blobSize1 * 2;
    auto dataSize1 = selectQuery["data1"].dataSize();
    EXPECT_EQ(blobSize1, dataSize1);

    auto dataSize2 = selectQuery["data2"].dataSize();
    EXPECT_EQ(blobSize2, dataSize2);

    const auto* data = selectQuery["data1"].getText();
    for (size_t i = 0; i < blobSize1; i++)
    {
        EXPECT_EQ(char(i % 256), data[i]);
    }

    data = selectQuery["data2"].getText();
    for (size_t i = 0; i < blobSize2; i++)
    {
        EXPECT_EQ(char(256 - char(i % 256)), data[i]);
    }
#endif

    selectQuery.close();
}

DatabaseTests::DatabaseTests()
{
    escapeSQLString("x", false);
}

DatabaseTests& DatabaseTests::tests()
{
    return _databaseTests;
}

void DatabaseTests::createOracleAutoIncrement(const DatabaseConnection& db, const String& tableName,
                                              const String& columnName)
{
    string baseName = "id_" + tableName.substr(0, 27);
    string sequenceName = "sq_" + baseName;
    string triggerName = "tr_" + baseName;

    try
    {
        Query dropSequence(db, "DROP SEQUENCE " + sequenceName);
        dropSequence.exec();
    }
    catch (const Exception& e)
    {
        COUT(e.what() << endl)
    }

    Query createSequence(db, "CREATE SEQUENCE " + sequenceName + " START WITH 1 INCREMENT BY 1 NOMAXVALUE");
    createSequence.exec();

    try
    {
        Query createTrigger(db,
                            "CREATE OR REPLACE TRIGGER " + triggerName + "\n" +
                                "BEFORE INSERT ON " + tableName + "\n" +
                                "FOR EACH ROW\n" +
                                "BEGIN\n" +
                                "  IF :new." + columnName + " IS NULL THEN\n" +
                                "    :new." + columnName + " := " + sequenceName + ".nextval;\n" +
                                "  END IF;\n" +
                                "END;\n");
        createTrigger.exec();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
    }
}
