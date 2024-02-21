/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/db/InsertQuery.h>

#ifdef USE_GTEST
#include <gtest/gtest.h>
#endif

#include <format>

using namespace std;
using namespace sptk;
using namespace chrono;

DatabaseTests DatabaseTests::_databaseTests;

vector<DatabaseConnectionString> DatabaseTests::connectionStrings() const
{
    vector<DatabaseConnectionString> connectionStrings;
    connectionStrings.reserve(m_connectionStrings.size());
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

    using enum sptk::DatabaseObjectType;
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
#ifdef USE_GTEST
    EXPECT_STREQ(databaseConnection->connectionString().toString().c_str(), connectionString.toString().c_str());
#endif
    databaseConnection->open();
#ifdef USE_GTEST
    EXPECT_TRUE(databaseConnection->active());
#endif

    constexpr size_t needInfoStrings = 10;
    if (const auto info = databaseConnection->driverDescription(); info.length() < needInfoStrings)
    {
        throw DatabaseException("Driver info is empty");
    }

    Strings objects;
    databaseConnection->objectList(TABLES, objects);
    databaseConnection->objectList(DATABASES, objects);
    databaseConnection->objectList(FUNCTIONS, objects);
    databaseConnection->objectList(PROCEDURES, objects);
    databaseConnection->objectList(VIEWS, objects);
    databaseConnection->close();
}

void DatabaseTests::testDDL(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();

    databaseConnection->open();

    Query createTable(databaseConnection, "CREATE TABLE gtest_temp_table(id INT, name VARCHAR(20))");
    Query dropTable(databaseConnection, "DROP TABLE gtest_temp_table");

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        const RegularExpression matchTableNotExists("not exist|unknown table", "i");
        if (!matchTableNotExists.matches(e.what()))
        {
            CERR(e.what() << endl);
        }
    }

    createTable.exec();
    dropTable.exec();

    databaseConnection->close();
}

static const DateTime testDateTime("2015-06-01 11:22:33");

static const vector<DatabaseTests::Row> rows = {
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

    const auto itor = fieldTypes->find(driverName);
    if (itor == fieldTypes->end())
    {
        throw Exception("Data type mapping is not defined for the test");
    }
    return itor->second;
}

void DatabaseTests::testQueryInsertDate(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();

    const auto itor = dateFieldTypes.find(connectionString.driverName());
    if (itor == dateFieldTypes.end())
    {
        throw Exception("Date data type mapping is not defined for the test");
    }

    const String dateTimeType = itor->second;

    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table(ts " << dateTimeType << " NULL)";

    databaseConnection->open();
    Query createTable(databaseConnection, createTableSQL.str());
    Query dropTable(databaseConnection, "DROP TABLE gtest_temp_table");

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl);
    }

    createTable.exec();

    const auto isOracle = databaseConnection->connectionType() == DatabaseConnectionType::ORACLE ||
                          databaseConnection->connectionType() == DatabaseConnectionType::ORACLE_OCI;
    const String testDate = isOracle ? "01-JUN-2015" : "2015-06-01";
    Query insert1(databaseConnection, "INSERT INTO gtest_temp_table VALUES('" + testDate + "')");
    insert1.exec();
    Query insert2(databaseConnection, "INSERT INTO gtest_temp_table VALUES(:dt)");

    const DateTime dateTime("2015-06-01");
    Variant date;
    date.setDateTime(dateTime, true);
    insert2.param("dt") = date;
    insert2.exec();

#ifdef USE_GTEST
    Query select(databaseConnection, "SELECT ts FROM gtest_temp_table");
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
    const DatabaseConnection databaseConnection = connectionPool.getConnection();

    const auto itor = dateTimeFieldTypes.find(connectionString.driverName());
    if (itor == dateTimeFieldTypes.end())
    {
        throw Exception("DateTime data type mapping is not defined for the test");
    }

    const String dateTimeType = itor->second;

    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table(ts " << dateTimeType << " NULL)";

    databaseConnection->open();
    Query createTable(databaseConnection, createTableSQL.str());
    Query dropTable(databaseConnection, "DROP TABLE gtest_temp_table");

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl);
    }

    createTable.exec();

    const DateTime testDate(2000, 01, 01);
    constexpr size_t dateAndTimeLength = 19;
    const auto testTimezone = testDate.isoDateTimeString().substr(dateAndTimeLength);

    const auto isOracle = databaseConnection->connectionType() == DatabaseConnectionType::ORACLE ||
                          databaseConnection->connectionType() == DatabaseConnectionType::ORACLE_OCI;
    const String testDateStr = isOracle ? "01-JUN-2015 11:22:33" : "2015-06-01 11:22:33";

    Query insert1(databaseConnection, "INSERT INTO gtest_temp_table VALUES('" + testDateStr + "')");
    insert1.exec();
    Query insert2(databaseConnection, "INSERT INTO gtest_temp_table VALUES(:dt)");
    insert2.param("dt") = DateTime("2015-06-01T11:22:33");
    insert2.exec();

    Query select(databaseConnection, "SELECT ts FROM gtest_temp_table");
    select.open();

    auto dateTimeStr = select["ts"].asDateTime().isoDateTimeString();

    DateTime testDateTime1(("2015-06-01T11:22:33" + testTimezone).c_str());
    const auto testDateTimeStr = testDateTime1.isoDateTimeString();
    EXPECT_STREQ(testDateTimeStr.c_str(), dateTimeStr.c_str());

    if (const auto result = select.next();
        !result)
    {
        FAIL() << "Expect two records in record set";
    }

    dateTimeStr = select["ts"].asDateTime().isoDateTimeString();
    testDateTime1 = DateTime(("2015-06-01T11:22:33" + testTimezone).c_str());
    EXPECT_STREQ(testDateTimeStr.c_str(), dateTimeStr.c_str());
    select.close();
}

void DatabaseTests::testQueryParameters(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();

    createTempTable(connectionString, databaseConnection);

    Buffer clob = createClob();

    Query insert(databaseConnection, "INSERT INTO gtest_temp_table VALUES(:id, :ssid, :name, :price, :ts, :enabled, :txt)");
    insertDataIntoTempTable(clob, insert);

    Variant ident;
    ident.setImagePtr(bit_cast<const uint8_t*>(&ident));
    insert.param("id") = ident;
    try
    {
        insert.exec();
        FAIL() << "Unsupported parameter type not detected";
    }
    catch (const DatabaseException& e)
    {
        const auto error = String(e.what());
        const auto errorWasExpected = error.startsWith("Unsupported parameter type") || error.startsWith("Parameter data type has changed.");
        if (!errorWasExpected)
        {
            FAIL() << e.what();
        }
    }

    verifyInsertedData(databaseConnection, clob);
}

Buffer DatabaseTests::createClob()
{
    Buffer clob;
    size_t counter = 0;
    const String textFragment("A text ");
    constexpr size_t sixtyFourKb = 65536;
    constexpr size_t lineLength = 72;
    while (clob.size() < sixtyFourKb)
    { // A size of the CLOB that is bigger than 64K
        clob.append(textFragment);
        counter += textFragment.length();
        if (counter > lineLength)
        {
            counter = 0;
            clob.append('\n');
        }
    }
    clob.append("END");
    return clob;
}

void DatabaseTests::insertDataIntoTempTable(Buffer& clob, Query& insert)
{
    for (const auto& row: rows)
    {
        insert.param("ts") = DateTime::Now();
        insert.param("id") = row.id;
        insert.param("ssid") = row.ssid;
        insert.param("name") = row.name;
        insert.param("price") = row.price;
        insert.param("enabled").setBool(true);
        insert.param("txt").setBuffer(bit_cast<const uint8_t*>(clob.data()), clob.size(), VariantDataType::VAR_TEXT);
        insert.exec();
    }
}

void DatabaseTests::verifyInsertedData(const DatabaseConnection& databaseConnection, const Buffer& clob)
{
    Query select(databaseConnection, "SELECT * FROM gtest_temp_table ORDER BY id");
    select.open();
    for (const auto& row: rows)
    {
        if (select.eof())
        {
            break;
        }

        verifyInsertedRow(row, clob, select);

        select.next();
    }
    select.close();
}

void DatabaseTests::verifyInsertedRow(const Row& row, const Buffer& clob, Query& select)
{
    EXPECT_EQ(row.id, select["id"].asInteger());
    EXPECT_EQ(row.ssid, select["ssid"].asInt64());
    EXPECT_STREQ(row.name.c_str(), select["name"].asString().c_str());
    EXPECT_FLOAT_EQ((float) row.price, (float) select["price"].asFloat());
    EXPECT_EQ(clob.size(), select["txt"].asString().length());

    const auto receivedClob = Buffer(select["txt"].asString());
    EXPECT_EQ(clob, receivedClob);
}

void DatabaseTests::createTempTable(const DatabaseConnectionString& connectionString, const DatabaseConnection& databaseConnection)
{
    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table( -- Create a test temp table" << endl;
    createTableSQL << "id INT, /* This is the unique id column */";
    createTableSQL << "ssid INT, ";
    createTableSQL << "name VARCHAR(20), price DECIMAL(10,2), ";
    createTableSQL << "ts " << fieldType("DATETIME", connectionString.driverName()) << " NULL, ";
    createTableSQL << "enabled " << fieldType("BOOL", connectionString.driverName()) << " NULL, ";
    createTableSQL << "txt " << fieldType("TEXT", connectionString.driverName()) << " NULL ";
    createTableSQL << ")";

    databaseConnection->open();
    Query createTable(databaseConnection, createTableSQL.str());
    Query dropTable(databaseConnection, "DROP TABLE gtest_temp_table");

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl);
    }

    createTable.exec();
}

void DatabaseTests::testTransaction(const DatabaseConnection& databaseConnection, bool commit)
{
    Query deleteRecords(databaseConnection, "DELETE FROM gtest_temp_table");
    deleteRecords.exec();

    Transaction transaction(databaseConnection);

    invalidTransactionStateThrows(transaction);

    transaction.begin();

    const size_t maxRecords = insertRecordsInTransaction(databaseConnection);

    if (commit)
    {
        transaction.commit();

        auto count = countRowsInTable(databaseConnection, "gtest_temp_table");
        if (count != maxRecords)
        {
            throw Exception(format("count != {} after commit", maxRecords));
        }
    }
    else
    {
        transaction.rollback();

        auto count = countRowsInTable(databaseConnection, "gtest_temp_table");
        if (count != 0)
        {
            throw Exception("count != 0 (after rollback)");
        }
    }

    invalidTransactionStateThrows(transaction);
}

void DatabaseTests::invalidTransactionStateThrows(Transaction& transaction)
{
    EXPECT_THROW(transaction.commit(), DatabaseException);
    EXPECT_THROW(transaction.rollback(), DatabaseException);
}

size_t DatabaseTests::insertRecordsInTransaction(const DatabaseConnection& databaseConnection)
{
    Query insert(databaseConnection, "INSERT INTO gtest_temp_table VALUES('1', 'pear')");

    const size_t maxRecords = 100;

    for (unsigned i = 0; i < maxRecords; ++i)
    {
        insert.exec();
    }


    if (auto count = countRowsInTable(databaseConnection, "gtest_temp_table");
        count != maxRecords)
    {
        throw Exception(format("count {} != {}", count, maxRecords));
    }
    return maxRecords;
}

void DatabaseTests::testTransaction(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();

    databaseConnection->open();
    Query createTable(databaseConnection, "CREATE TABLE gtest_temp_table(id INT, name VARCHAR(20))");
    Query dropTable(databaseConnection, "DROP TABLE gtest_temp_table");

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl);
    }

    createTable.exec();

    testTransaction(databaseConnection, false);
    testTransaction(databaseConnection, true);

    dropTable.exec();
}

DatabaseConnectionString DatabaseTests::connectionString(const String& driverName) const
{
    const auto itor = m_connectionStrings.find(driverName);
    return itor == m_connectionStrings.end() ? DatabaseConnectionString("") : itor->second;
}

void DatabaseTests::createTestTable(const DatabaseConnection& databaseConnection, bool autoPrepare, bool withBlob)
{
    const auto driverName = databaseConnection->connectionString().driverName();
    const auto itor = blobFieldTypes.find(driverName);
    if (itor == blobFieldTypes.end())
    {
        throw Exception("BLOB data type mapping is not defined for the test");
    }

    const String blobType = itor->second;

    Strings fields {"id INTEGER NULL", "name CHAR(40) NULL", "position_name CHAR(20) NULL", "hire_date CHAR(12) NULL"};

    if (withBlob)
    {
        fields.push_back("data1 " + blobType + " NULL");
        fields.push_back("data2 " + blobType + " NULL");
    }

    const String sql("CREATE TABLE gtest_temp_table(" + fields.join(", ") + ")");

    databaseConnection->open();
    Query createTable(databaseConnection, sql, autoPrepare);
    Query dropTable(databaseConnection, "DROP TABLE gtest_temp_table", autoPrepare);

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        const RegularExpression matchTableNotExists("not exist|unknown table", "i");
        if (!matchTableNotExists.matches(e.what()))
        {
            CERR(e.what() << endl);
        }
    }

    createTable.exec();
}

void DatabaseTests::createTestTableWithSerial(const DatabaseConnection& databaseConnection)
{
    const auto itor = dateTimeFieldTypes.find(databaseConnection->connectionString().driverName());
    if (itor == dateTimeFieldTypes.end())
    {
        throw Exception("DateTime data type mapping is not defined for the test");
    }

    const String dateTimeType = itor->second;

    databaseConnection->open();

    stringstream sql;
    String idDefinition;

    switch (databaseConnection->connectionType())
    {
        using enum sptk::DatabaseConnectionType;
        case MYSQL:
        case POSTGRES:
            idDefinition = "id serial";
            break;
        case MSSQL_ODBC:
            idDefinition = "id int identity";
            break;
        case ORACLE:
        case ORACLE_OCI:
            idDefinition = "id int";
            break;
        default:
            throw DatabaseException("InsertQuery doesn't support " + databaseConnection->driverDescription());
    }

    sql << "CREATE TABLE gtest_temp_table2(" << idDefinition << " primary key, name varchar(40))";

    Query createTable(databaseConnection, sql.str());
    Query dropTable(databaseConnection, "DROP TABLE gtest_temp_table2");

    try
    {
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        const RegularExpression matchTableNotExists("not exist|unknown table", "i");
        if (!matchTableNotExists.matches(e.what()))
        {
            CERR(e.what() << endl);
        }
    }

    createTable.exec();

    if (databaseConnection->connectionType() == DatabaseConnectionType::ORACLE ||
        databaseConnection->connectionType() == DatabaseConnectionType::ORACLE_OCI)
    {
        createOracleAutoIncrement(databaseConnection, "gtest_temp_table2", "id");
    }

    InsertQuery query(databaseConnection, "INSERT INTO gtest_temp_table2(name) VALUES(:name)");

    query.param("name") = "Alex";
    query.exec();
    auto recordId = query.id();
    EXPECT_EQ(recordId, uint64_t(1));

    query.param("name") = "David";
    query.exec();
    recordId = query.id();
    EXPECT_EQ(recordId, uint64_t(2));
}

static const string expectedBulkInsertResult(
    "1|Alex,'Doe'|Programmer|01-JAN-2014 # 2|David|CEO|01-JAN-2015 # 3|Roger|Bunny|01-JAN-2016 # 4|Teddy|Bear|01-JAN-2017 # 5|Santa|Claus|01-JAN-2018");

void DatabaseTests::testBulkInsert(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTable(databaseConnection, false, false);

    Query selectData(databaseConnection, "SELECT * FROM gtest_temp_table");

    vector<VariantVector> data;

    VariantVector aRow;

    constexpr int id1 = 1;
    constexpr int id2 = 2;
    constexpr int id3 = 3;
    constexpr int id4 = 4;
    constexpr int id5 = 5;

    aRow.emplace_back(id1);
    aRow.emplace_back("Alex,'Doe'");
    aRow.emplace_back("Programmer");
    aRow.emplace_back("01-JAN-2014");
    data.push_back(aRow);

    aRow.clear();
    aRow.emplace_back(id2);
    aRow.emplace_back("David");
    aRow.emplace_back("CEO");
    aRow.emplace_back("01-JAN-2015");
    data.push_back(aRow);

    aRow.clear();
    aRow.emplace_back(id3);
    aRow.emplace_back("Roger");
    aRow.emplace_back("Bunny");
    aRow.emplace_back("01-JAN-2016");
    data.push_back(aRow);

    aRow.clear();
    aRow.emplace_back(id4);
    aRow.emplace_back("Teddy");
    aRow.emplace_back("Bear");
    aRow.emplace_back("01-JAN-2017");
    data.push_back(aRow);

    aRow.clear();
    aRow.emplace_back(id5);
    aRow.emplace_back("Santa");
    aRow.emplace_back("Claus");
    aRow.emplace_back("01-JAN-2018");
    data.push_back(aRow);

    const Strings columnNames({"id", "name", "position_name", "hire_date"});
    databaseConnection->bulkInsert("gtest_temp_table", columnNames, data);

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

    if (constexpr int expectedRows = 5;
        printRows.size() != expectedRows)
    {
        throw Exception(
            "Expected bulk insert result (3 rows) doesn't match table data (" + int2string(printRows.size()) + ")");
    }

    if (const String actualResult(printRows.join(" # "));
        actualResult != expectedBulkInsertResult)
    {
        cout << "Actual result: " << actualResult << endl;
        cout << "Expected result: " << expectedBulkInsertResult << endl;
        throw Exception("Expected bulk insert result doesn't match inserted data");
    }

    const VariantVector keys({id1, id2, id3, id4, id5});
    databaseConnection->bulkDelete("gtest_temp_table", "id", keys);

    Query countRows(databaseConnection, "SELECT COUNT(*) FROM gtest_temp_table");
    countRows.open();
    const auto rowCount = countRows[0].asInteger();
    countRows.close();
    EXPECT_EQ(0, rowCount);
}

void DatabaseTests::testInsertQuery(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTableWithSerial(databaseConnection);
}

void DatabaseTests::testInsertQueryDirect(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTable(databaseConnection, false, false);

    Query insert(databaseConnection, "INSERT INTO gtest_temp_table(id, name, position_name, hire_date)"
                                     "VALUES (1, 'John Doe', 'engineer', '2020-01-02')",
                 false);
    insert.exec();

    insert.sql("INSERT INTO gtest_temp_table(id, name, position_name, hire_date)"
               "VALUES (2, 'Jane Doe', 'CFO', '2020-02-03')");
    insert.exec();

    verifyTableNoBlobs(databaseConnection);
}

void DatabaseTests::verifyTableNoBlobs(const DatabaseConnection& databaseConnection)
{
    Query select(databaseConnection, "SELECT * FROM gtest_temp_table ORDER BY 1", false);
    select.open();
    size_t recordCount = 0;
    while (!select.eof())
    {
        ++recordCount;
        EXPECT_EQ(recordCount, select[0].asInteger());
        const char* expectedName = recordCount == 1 ? "John Doe" : "Jane Doe";
        EXPECT_STREQ(expectedName, select[1].asString().trim().c_str());
        select.next();
    }
    EXPECT_EQ(2U, recordCount);
}

void DatabaseTests::testBulkInsertPerformance(const DatabaseConnectionString& connectionString, size_t recordCount)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTable(databaseConnection, false, false);

    const Query selectData(databaseConnection, "SELECT * FROM gtest_temp_table");
    Query insertData(databaseConnection, "INSERT INTO gtest_temp_table VALUES (:id, :name, :position, :hired)");

    vector<VariantVector> data;
    VariantVector keys;
    keys.reserve(recordCount);
    for (size_t i = 1; i <= recordCount; ++i)
    {
        keys.emplace_back((int) i);

        VariantVector arow;
        arow.emplace_back(int(i));
        arow.emplace_back("Alex,'Doe'");
        arow.emplace_back("Programmer");
        arow.emplace_back("01-JAN-2014");
        data.push_back(std::move(arow));
    }

    StopWatch stopWatch;
    stopWatch.start();
    const Strings columnNames({"id", "name", "position_name", "hire_date"});
    databaseConnection->bulkInsert("gtest_temp_table", columnNames, data);
    stopWatch.stop();
    const auto bulkInsertDurationMS = stopWatch.milliseconds();

    stopWatch.start();
    databaseConnection->bulkDelete("gtest_temp_table", "id", keys);
    stopWatch.stop();
    const auto bulkDeleteDurationMS = stopWatch.milliseconds();

    stopWatch.start();

    auto& idParam = insertData.param("id");
    auto& nameParam = insertData.param("name");
    auto& positionParam = insertData.param("position");
    auto& hiredParam = insertData.param("hired");

    Transaction transaction(databaseConnection);
    transaction.begin();
    constexpr int col0 = 0;
    constexpr int col1 = 1;
    constexpr int col2 = 2;
    constexpr int col3 = 3;
    for (auto& row: data)
    {
        idParam = row[col0].asInteger();
        nameParam = row[col1].asString();
        positionParam = row[col2].asString();
        hiredParam = row[col3].asString();
        insertData.exec();
    }
    transaction.commit();
    stopWatch.stop();
    const auto insertDurationMS = stopWatch.milliseconds();

    stopWatch.start();
    transaction.begin();
    Query deleteQuery(databaseConnection, "DELETE FROM gtest_temp_table WHERE id=:id");
    for (const auto& key: keys)
    {
        deleteQuery.param("id") = key;
        deleteQuery.exec();
    }
    transaction.commit();
    stopWatch.stop();
    const auto deleteDurationMS = stopWatch.milliseconds();

    auto printResults = [&](const String& operation, double durationMs) {
        COUT(left << fixed << setw(25) << connectionString.driverName() << setw(14) << operation
                  << right << setw(8) << setprecision(1) << durationMs << " ms, "
                  << setprecision(2) << fixed << setw(10) << static_cast<double>(data.size()) / durationMs << "K rec/sec" << endl);
    };

    printResults("insert", insertDurationMS);
    printResults("bulk insert", bulkInsertDurationMS);
    printResults("delete", deleteDurationMS);
    printResults("bulk delete", bulkDeleteDurationMS);
    COUT(endl);
}

void DatabaseTests::testBatchSQL(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTable(databaseConnection, false, false);

    Query selectData(databaseConnection, "SELECT * FROM gtest_temp_table ORDER BY 1");

    const Strings batchSQL {
        "INSERT INTO gtest_temp_table VALUES (1, 'Jonh', 'CEO', '2020-01-02');",
        "INSERT INTO gtest_temp_table VALUES (2, 'Jane', 'CFO', '2021-02-03');",
        "INSERT INTO gtest_temp_table VALUES (3, 'William', 'CIO', '2022-03-04');"};

    const Strings invalidBatchSQL {
        "REMOVE INTO gtest_temp_table VALUES (2, 'Jane', 'CFO', '2021-02-03');",
        "INVENT INTO gtest_temp_table VALUES (3, 'William', 'CIO', '2022-03-04');"};

    const Strings expectedResults {
        "1,Jonh,CEO,2020-01-02",
        "2,Jane,CFO,2021-02-03",
        "3,William,CIO,2022-03-04"};

    EXPECT_THROW(databaseConnection->executeBatchSQL(invalidBatchSQL), DatabaseException);

    databaseConnection->executeBatchSQL(batchSQL);

    verifyBatchInsertedData(selectData, expectedResults);
}

void DatabaseTests::verifyBatchInsertedData(Query& selectData, const Strings& expectedResults)
{
    selectData.open();
    int rowNumber = 0;
    constexpr int expectedRows = 3;
    for (; rowNumber < expectedRows && !selectData.eof(); ++rowNumber)
    {
        Strings row;
        for (size_t column = 0; column < selectData.fieldCount(); ++column)
        {
            row.push_back(selectData[column].asString().trim());
        }
        EXPECT_STREQ(expectedResults[rowNumber].c_str(), row.join(",").c_str());
        selectData.next();
    }
    EXPECT_EQ(rowNumber, 3);
}

void DatabaseTests::testSelect(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    testSelect(connectionPool);
}

void DatabaseTests::testSelect(DatabaseConnectionPool& connectionPool)
{
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTable(databaseConnection, false, false);

    Query emptyQuery(databaseConnection);

    EXPECT_THROW(emptyQuery.exec(), DatabaseException);

    Query selectNullData(databaseConnection, "SELECT * FROM gtest_temp_table WHERE id IS NULL");
    Query selectNotNullData(databaseConnection, "SELECT * FROM gtest_temp_table WHERE id IS NOT NULL");
    Query insertData(databaseConnection, "INSERT INTO gtest_temp_table VALUES (:id, :name, :position, :hired)");

    Strings data;
    data.push_back(string("1\tAlex,'Doe'\tProgrammer\t01-JAN-2014"));
    data.push_back(string("2\tDavid\tCEO\t01-JAN-2015"));
    data.push_back(string("3\tRoger\tBunny\t01-JAN-2016"));
    data.push_back(string("4\tTeddy\tBear\t01-JAN-2017"));
    data.push_back(string("5\tSanta\tClaus\t01-JAN-2018"));

    constexpr int col0 = 0;
    constexpr int col1 = 1;
    constexpr int col2 = 2;
    constexpr int col3 = 3;

    for (const auto& row: data)
    {
        using enum sptk::VariantDataType;

        // Insert all nulls
        insertData.param("id").setNull(VAR_INT);
        insertData.param("name").setNull(VAR_STRING);
        insertData.param("position").setNull(VAR_STRING);
        insertData.param("hired").setNull(VAR_STRING);
        insertData.exec();

        // Insert data row
        Strings values(row, "\t");
        insertData.param("id") = string2int(values[col0]);
        insertData.param("name") = values[col1];
        insertData.param("position") = values[col2];
        insertData.param("hired") = values[col3];
        insertData.exec();
    }

    try
    {
        selectNotNullData.next();
        FAIL() << "Expected to throw DatabaseException";
    }
    catch (const DatabaseException&)
    {
        COUT("");
    }

    constexpr size_t expectedRecordCount = 5;
    size_t recordCount = 0;

    selectNullData.open();
    while (!selectNullData.eof())
    {
        // Check if all fields are NULLs
        ranges::for_each(selectNullData.fields(),
                         [](const auto& field) {
                             if (!field->isNull())
                             {
                                 throw Exception(format("Field {} = [{}] but null is expected",
                                                        field->fieldName().c_str(), field->asString().c_str()));
                             }
                         });

        selectNullData.next();
        ++recordCount;
    }
    selectNullData.close();
    EXPECT_EQ(expectedRecordCount, recordCount);

    selectNotNullData.open();
    Strings printRows;
    recordCount = 0;
    while (!selectNotNullData.eof())
    {
        Strings row;
        for (const auto& field: selectNotNullData.fields())
        {
            if (field->isNull())
            {
                throw Exception("Field " + field->fieldName() + " is null but value is expected");
            }
            row.push_back(field->asString().trim());
        }
        printRows.push_back(row.join("|"));
        selectNotNullData.next();
        ++recordCount;
    }
    EXPECT_EQ(expectedRecordCount, recordCount);

    try
    {
        selectNotNullData.next();
        FAIL() << "Expected to throw DatabaseException";
    }
    catch (const DatabaseException&)
    {
        COUT("");
    }

    selectNotNullData.close();

    const String actualResult(printRows.join(" # "));
    if (actualResult != expectedBulkInsertResult)
    {
        throw Exception("Expected result doesn't match inserted data");
    }
}

size_t DatabaseTests::countRowsInTable(const DatabaseConnection& databaseConnection, const String& table)
{
    Query select(databaseConnection, "SELECT count(*) cnt FROM " + table);
    select.open();
    const auto count = (size_t) select["cnt"].asInteger();
    select.close();

    return count;
}

void DatabaseTests::testBLOB(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTable(databaseConnection, true, true);

    constexpr size_t blobSize1 = 65536;

    Buffer testData1(blobSize1);
    Buffer testDataInv(blobSize1);
    constexpr size_t Value256 = 256;
    for (size_t i = 0; i < blobSize1; ++i)
    {
        testData1[i] = static_cast<uint8_t>(i % Value256);
        testDataInv[i] = static_cast<uint8_t>(Value256 - i % Value256);
    }
    testData1.bytes(blobSize1);
    testDataInv.bytes(blobSize1);

    Buffer testData2(testDataInv);
    testData2.append(testDataInv);

    Query insertQuery(databaseConnection, "INSERT INTO gtest_temp_table(id,data1, data2) VALUES(:id, :data1, :data2)");
    insertQuery.param("id") = 1;
    insertQuery.param("data1") = testData1;
    insertQuery.param("data2") = testData2;
    insertQuery.exec();

    Query selectQuery(databaseConnection, "SELECT id, data1, data2 FROM gtest_temp_table ORDER BY 1");
    selectQuery.open();

    constexpr size_t blobSize2 = blobSize1 * 2;
    const auto dataSize1 = selectQuery["data1"].dataSize();
    EXPECT_EQ(blobSize1, dataSize1);

    const auto dataSize2 = selectQuery["data2"].dataSize();
    EXPECT_EQ(blobSize2, dataSize2);

    const auto* data = selectQuery["data1"].getText();
    for (size_t i = 0; i < blobSize1; ++i)
    {
        EXPECT_EQ(char(i % 256), data[i]);
    }

    data = selectQuery["data2"].getText();
    for (size_t i = 0; i < blobSize2; ++i)
    {
        EXPECT_EQ(char(256 - char(i % 256)), data[i]);
    }

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

void DatabaseTests::createOracleAutoIncrement(const DatabaseConnection& databaseConnection, const String& tableName,
                                              const String& columnName)
{
    const String baseName = "id_" + tableName.substr(0, 27);
    const String sequenceName = "sq_" + baseName;
    const String triggerName = "tr_" + baseName;

    try
    {
        Query dropSequence(databaseConnection, "DROP SEQUENCE " + sequenceName);
        dropSequence.exec();
    }
    catch (const Exception& e)
    {
        COUT(e.what() << endl);
    }

    Query createSequence(databaseConnection, "CREATE SEQUENCE " + sequenceName + " START WITH 1 INCREMENT BY 1 NOMAXVALUE");
    createSequence.exec();

    try
    {
        Query createTrigger(databaseConnection,
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
        CERR(e.what() << endl);
    }
}
