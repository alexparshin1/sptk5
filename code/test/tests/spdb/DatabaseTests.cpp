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
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <ranges>
#include <sptk5/Printer.h>
#include <sptk5/cutils>
#include <sptk5/db/DatabaseConnectionPool.h>
#include <sptk5/db/DatabaseTests.h>
#include <sptk5/db/InsertQuery.h>
#include <sptk5/threads/Flag.h>

#ifdef USE_GTEST
#include <gtest/gtest.h>
#endif

using namespace std;
using namespace sptk;
using namespace chrono;

DatabaseTests DatabaseTests::_databaseTests;

vector<DatabaseConnectionString> DatabaseTests::connectionStrings() const
{
    vector<DatabaseConnectionString> connectionStrings;
    connectionStrings.reserve(m_connectionStrings.size());
    for (const auto& connectionString: m_connectionStrings | views::values)
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

void DatabaseTests::dropTable(const DatabaseConnection& databaseConnection, const String& tableName)
{
    try
    {
        String dropTableSql("DROP TABLE " + tableName);
        switch (databaseConnection->connectionType())
        {
            case DatabaseConnectionType::POSTGRES:
                dropTableSql += " CASCADE";
                break;
            case DatabaseConnectionType::ORACLE:
            case DatabaseConnectionType::ORACLE_OCI:
                dropTableSql += " CASCADE CONSTRAINTS";
                break;
            default:
                break;
        }

        Query dropTable(databaseConnection, dropTableSql);
        dropTable.exec();
    }
    catch (const Exception& e)
    {
        const RegularExpression matchTableNotExists("not exist|unknown table|no such table", "i");
        if (!matchTableNotExists.matches(e.what()))
        {
            CERR(e.what());
        }
    }
}

void DatabaseTests::testDDL(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();

    databaseConnection->open();

    dropTable(databaseConnection, "gtest_temp_table");

    Query createTable(databaseConnection, "CREATE TABLE gtest_temp_table(id INT, name VARCHAR(20))");
    createTable.exec();

    dropTable(databaseConnection, "gtest_temp_table");

    databaseConnection->close();
}

namespace {
const DateTime testDateTime("2015-06-01 11:22:33");

const vector<DatabaseTests::Row> rows = {
    {1, 1234567, "apple", 1.5, testDateTime},
    {2, 1234567, "pear", 3.1, testDateTime},
    {3, 1234567, "melon", 1.05, testDateTime},
    {4, 1234567, "watermelon", 0.85, testDateTime},
    {5, 1234567, "lemon", 5.5, testDateTime}};

const map<String, String> dateFieldTypes = {
    {"mysql", "DATE"},
    {"postgresql", "DATE"},
    {"mssql", "DATE"},
    {"oracle", "DATE"},
    {"sqlite3", "VARCHAR(10)"}};

const map<String, String> dateTimeFieldTypes = {
    {"mysql", "TIMESTAMP"},
    {"postgresql", "TIMESTAMP"},
    {"mssql", "DATETIME2"},
    {"oracle", "TIMESTAMP"},
    {"sqlite3", "VARCHAR(30)"}};

const map<String, String> boolFieldTypes = {
    {"mysql", "BOOL"},
    {"postgresql", "BOOL"},
    {"mssql", "BIT"},
    {"oracle", "NUMBER(1)"},
    {"sqlite3", "INT"},
};

const map<String, String> textFieldTypes = {
    {"mysql", "LONGTEXT"},
    {"postgresql", "TEXT"},
    {"mssql", "NVARCHAR(MAX)"},
    {"oracle", "CLOB"},
    {"sqlite3", "TEXT"}};

const map<String, String> blobFieldTypes = {
    {"mysql", "LONGBLOB"},
    {"postgresql", "BYTEA"},
    {"mssql", "VARBINARY(MAX)"},
    {"oracle", "BLOB"},
    {"sqlite3", "TEXT"}};

String fieldType(const String& fieldType, const String& driverName)
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

    const auto iterator = fieldTypes->find(driverName);
    if (iterator == fieldTypes->end())
    {
        throw Exception("Data type mapping is not defined for the test");
    }
    return iterator->second;
}
} // namespace

void DatabaseTests::testQueryInsertDate(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();

    const auto iterator = dateFieldTypes.find(connectionString.driverName());
    if (iterator == dateFieldTypes.end())
    {
        throw Exception("Date data type mapping is not defined for the test");
    }

    const String dateTimeType = iterator->second;

    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table(ts " << dateTimeType << " NULL)";

    databaseConnection->open();
    Query createTable(databaseConnection, createTableSQL.str());

    dropTable(databaseConnection, "gtest_temp_table");

    createTable.exec();

    const auto isOracle = databaseConnection->connectionType() == DatabaseConnectionType::ORACLE ||
                          databaseConnection->connectionType() == DatabaseConnectionType::ORACLE_OCI;
    const String testDate = isOracle ? "01-JUN-2015" : "2015-06-01";
    Query        insert1(databaseConnection, "INSERT INTO gtest_temp_table VALUES('" + testDate + "')");
    insert1.exec();
    Query insert2(databaseConnection, "INSERT INTO gtest_temp_table VALUES(:dt)");

    const DateTime dateTime("2015-06-01");
    Variant        date;
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
    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();

    const auto iterator = dateTimeFieldTypes.find(connectionString.driverName());
    if (iterator == dateTimeFieldTypes.end())
    {
        throw Exception("DateTime data type mapping is not defined for the test");
    }

    const String dateTimeType = iterator->second;

    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table(ts " << dateTimeType << " NULL)";

    databaseConnection->open();
    Query createTable(databaseConnection, createTableSQL.str());

    dropTable(databaseConnection, "gtest_temp_table");

    createTable.exec();

    const DateTime   testDate(2000, 01, 01);
    constexpr size_t dateAndTimeLength = 19;
    const auto       testTimezone = testDate.isoDateTimeString().substr(dateAndTimeLength);

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

    DateTime   testDateTime1(("2015-06-01T11:22:33" + testTimezone).c_str());
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
    DatabaseConnectionPool   connectionPool(connectionString.toString());
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
    Buffer           clob;
    size_t           counter = 0;
    const String     textFragment("A text ");
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
    EXPECT_FLOAT_EQ(static_cast<float>(row.price), static_cast<float>(select["price"].asFloat()));
    EXPECT_EQ(clob.size(), select["txt"].asString().length());

    const auto receivedClob = Buffer(select["txt"].asString());
    EXPECT_EQ(clob, receivedClob);
}

void DatabaseTests::createTempTable(const DatabaseConnectionString& connectionString, const DatabaseConnection& databaseConnection)
{
    stringstream createTableSQL;
    createTableSQL << "CREATE TABLE gtest_temp_table( -- Create a test temp table\n";
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
        CERR(e.what());
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

        const auto count = countRowsInTable(databaseConnection, "gtest_temp_table");
        if (count != maxRecords)
        {
            throw Exception("count != " + to_string(maxRecords) + "after commit");
        }
    }
    else
    {
        transaction.rollback();

        const auto count = countRowsInTable(databaseConnection, "gtest_temp_table");
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

    constexpr size_t maxRecords = 100;

    for (unsigned i = 0; i < maxRecords; ++i)
    {
        insert.exec();
    }


    if (const auto count = countRowsInTable(databaseConnection, "gtest_temp_table");
        count != maxRecords)
    {
        throw Exception("count " + to_string(count) + " != " + to_string(maxRecords));
    }
    return maxRecords;
}

void DatabaseTests::testTransaction(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool   connectionPool(connectionString.toString());
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
        CERR(e.what());
    }

    createTable.exec();

    testTransaction(databaseConnection, false);
    testTransaction(databaseConnection, true);

    dropTable.exec();
}

DatabaseConnectionString DatabaseTests::connectionString(const String& driverName) const
{
    const auto iterator = m_connectionStrings.find(driverName);
    return iterator == m_connectionStrings.end() ? DatabaseConnectionString("") : iterator->second;
}

String DatabaseTests::serialColumnDefinition(DatabaseConnectionType connectionType)
{
    String idColumn;
    switch (connectionType)
    {
        using enum DatabaseConnectionType;
        case POSTGRES:
            idColumn = "id SERIAL PRIMARY KEY";
            break;
        case MYSQL:
            idColumn = "id INT AUTO_INCREMENT PRIMARY KEY";
            break;
        case MSSQL_ODBC:
            idColumn = "id INT IDENTITY(1,1) PRIMARY KEY";
            break;
        case ORACLE_OCI:
        case ORACLE:
            idColumn = "id INT GENERATED BY DEFAULT ON NULL AS IDENTITY PRIMARY KEY";
            break;
        case FIREBIRD:
            idColumn = "id INTEGER GENERATED BY DEFAULT ON NULL AS IDENTITY PRIMARY KEY";
            break;
        case SQLITE3:
            idColumn = "id INTEGER PRIMARY KEY AUTOINCREMENT";
            break;
        case GENERIC_ODBC:
            throw DatabaseException("Auto increment test isn't supported for this connection type");
    }
    return idColumn;
}

void DatabaseTests::createTestTable(const DatabaseConnection& databaseConnection, bool autoPrepare, bool withBlob)
{
    databaseConnection->open();

    const auto driverName = databaseConnection->connectionString().driverName();
    const auto iterator = blobFieldTypes.find(driverName);
    if (iterator == blobFieldTypes.end())
    {
        throw Exception("BLOB data type mapping is not defined for the test");
    }

    const String blobType = iterator->second;

    String  idColumn = serialColumnDefinition(databaseConnection->connectionType());
    Strings fields {idColumn, "name CHAR(40) NULL", "position_name CHAR(20) NULL", "hire_date CHAR(12) NULL"};

    if (withBlob)
    {
        fields.push_back("data1 " + blobType + " NULL");
        fields.push_back("data2 " + blobType + " NULL");
    }

    const String sql("CREATE TABLE gtest_temp_table(" + fields.join(", ") + ")");

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
            CERR(e.what());
        }
    }

    createTable.exec();
}

void DatabaseTests::createTestTableWithSerial(const DatabaseConnection& databaseConnection)
{
    const auto iterator = dateTimeFieldTypes.find(databaseConnection->connectionString().driverName());
    if (iterator == dateTimeFieldTypes.end())
    {
        throw Exception("DateTime data type mapping is not defined for the test");
    }

    const String dateTimeType = iterator->second;

    databaseConnection->open();

    stringstream sql;
    String       idDefinition = serialColumnDefinition(databaseConnection->connectionType());

    sql << "CREATE TABLE gtest_temp_table2(" << idDefinition << ", name varchar(40))";

    Query createTable(databaseConnection, sql.str());

    dropTable(databaseConnection, "gtest_temp_table2");

    createTable.exec();

    InsertQuery query(databaseConnection, "INSERT INTO gtest_temp_table2(name) VALUES(:name)");

    query.param("name") = "Alex";
    query.exec();
    auto recordId = query.id();
    EXPECT_EQ(recordId, static_cast<uint64_t>(1));

    query.param("name") = "David";
    query.exec();
    recordId = query.id();
    EXPECT_EQ(recordId, static_cast<uint64_t>(2));
}

static const string expectedBulkInsertResult(
    "Alex,'Doe'|Programmer|01-JAN-2014 # David|CEO|01-JAN-2015 # Roger|Bunny|01-JAN-2016 # Teddy|Bear|01-JAN-2017 # Santa|Claus|01-JAN-2018");

void DatabaseTests::testBulkInsert(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();

    createTestTable(databaseConnection, false, false);

    Query selectData(databaseConnection, "SELECT * FROM gtest_temp_table");

    vector<VariantVector> data;

    VariantVector aRow;

    aRow.emplace_back("Alex,'Doe'");
    aRow.emplace_back("Programmer");
    aRow.emplace_back("01-JAN-2014");
    data.push_back(aRow);

    aRow.clear();
    aRow.emplace_back("David");
    aRow.emplace_back("CEO");
    aRow.emplace_back("01-JAN-2015");
    data.push_back(aRow);

    aRow.clear();
    aRow.emplace_back("Roger");
    aRow.emplace_back("Bunny");
    aRow.emplace_back("01-JAN-2016");
    data.push_back(aRow);

    aRow.clear();
    aRow.emplace_back("Teddy");
    aRow.emplace_back("Bear");
    aRow.emplace_back("01-JAN-2017");
    data.push_back(aRow);

    aRow.clear();
    aRow.emplace_back("Santa");
    aRow.emplace_back("Claus");
    aRow.emplace_back("01-JAN-2018");
    data.push_back(aRow);

    // Insert these columns in two steps to verify the inserted ids. Note that the auto-increment key isn't included here.
    // The name of the auto-increment column (if any) is provided in the bulkInsert() call.
    const Strings columnNames({"name", "position_name", "hire_date"});

    auto insertedIds1 = databaseConnection->bulkInsert("gtest_temp_table", "id", columnNames, data);
    EXPECT_EQ(5, insertedIds1.size());
    for (uint64_t i = 1; i <= 5; ++i)
    {
        EXPECT_EQ(i, insertedIds1[i - 1]);
    }

    auto insertedIds2 = databaseConnection->bulkInsert("gtest_temp_table", "id", columnNames, data);
    EXPECT_EQ(5, insertedIds2.size());
    for (uint64_t i = 1; i <= 5; ++i)
    {
        EXPECT_EQ(i + 5, insertedIds2[i - 1]);
    }

    selectData.open();
    Strings printRows;
    size_t  rowCount = 0;
    while (!selectData.eof())
    {
        Strings row;
        size_t  index = 0;
        for (const auto& field: selectData.fields())
        {
            if (index > 0)
            {
                row.push_back(field->asString().trim());
            }

            ++index;
        }

        if (printRows.size() >= 5)
        {
            break;
        }

        printRows.push_back(row.join("|"));

        selectData.next();
        ++rowCount;
    }
    selectData.close();

    if (const String actualResult(printRows.join(" # "));
        actualResult != expectedBulkInsertResult)
    {
        cout << "Actual result: " << actualResult << "\n";
        cout << "Expected result: " << expectedBulkInsertResult << "\n";
        throw Exception("Expected bulk insert result doesn't match inserted data");
    }

    VariantVector keys;
    for (const auto& id: insertedIds1)
    {
        keys.emplace_back(static_cast<int64_t>(id));
    }
    for (const auto& id: insertedIds2)
    {
        keys.emplace_back(static_cast<int64_t>(id));
    }

    databaseConnection->bulkDelete("gtest_temp_table", "id", keys);

    Query countRows(databaseConnection, "SELECT COUNT(*) FROM gtest_temp_table");
    countRows.open();
    rowCount = countRows[0].asInteger();
    countRows.close();
    EXPECT_EQ(0, rowCount);
}

void DatabaseTests::testParallelBulkInsert(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString.toString());

    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTable(databaseConnection, false, false);

    COUT(connectionString.driverName() << " bulk insert with two threads:");

    vector<VariantVector> data;

    VariantVector aRow;
    aRow.emplace_back("Alex,'Doe'");
    aRow.emplace_back("Programmer");
    aRow.emplace_back("01-JAN-2014");

    constexpr int dataRows = 10000;
    constexpr int batchSize = 100;

    for (int i = 0; i < dataRows; ++i)
    {
        data.push_back(aRow);
    }

    auto connectionThread = [&data, &connectionString](int threadNumber, vector<int64_t>* insertedIds)
    {
        vector inputData(data);

        string operation;
        try
        {
            operation = "Create connection";
            DatabaseConnectionPool   connectionPool(connectionString.toString());
            const Strings            columnNames({"name", "position_name", "hire_date"});
            const DatabaseConnection databaseConnection = connectionPool.getConnection();

            operation = "Open connection";
            databaseConnection->open();

            operation = "bulkInsert";
            StopWatch sw;
            sw.start();
            *insertedIds = databaseConnection->bulkInsert("gtest_temp_table", "id", columnNames, inputData, batchSize);
            sw.stop();

            COUT("Thread " << threadNumber << " inserted " << insertedIds->size() << " for " << fixed << setprecision(2) << sw.milliseconds() << "ms (" << insertedIds->size() / sw.milliseconds() << "K/sec)");
        }
        catch (const Exception& e)
        {
            CERR(operation << ": " << e.what());
        }
    };

    StopWatch sw;
    sw.start();

    vector<int64_t> insertedIds1;
    auto            thread1 = jthread(connectionThread, 1, &insertedIds1);

    vector<int64_t> insertedIds2;
    auto            thread2 = jthread(connectionThread, 2, &insertedIds2);

    thread1.join();
    thread2.join();
    sw.stop();

    set<int64_t> uniqueIds;
    for (const auto& id: insertedIds1)
    {
        uniqueIds.insert(id);
    }
    for (const auto& id: insertedIds2)
    {
        uniqueIds.insert(id);
    }

    COUT("All inserted      " << uniqueIds.size() << sw.milliseconds() << "ms (" << uniqueIds.size() / sw.milliseconds() << "K/sec)");

    Query selectData(databaseConnection, "SELECT COUNT(*) FROM gtest_temp_table");
    int   counter = selectData.scalar().asInteger();
    EXPECT_EQ(dataRows * 2, counter);
    EXPECT_EQ(dataRows * 2, uniqueIds.size());
}

void DatabaseTests::testInsertQuery(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTableWithSerial(databaseConnection);
}

void DatabaseTests::testInsertQueryDirect(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTable(databaseConnection, false, false);

    Query insert(databaseConnection, "INSERT INTO gtest_temp_table(name, position_name, hire_date)"
                                     "VALUES ('John Doe', 'engineer', '2020-01-02')",
                 false);
    insert.exec();

    insert.sql("INSERT INTO gtest_temp_table(name, position_name, hire_date)"
               "VALUES ('Jane Doe', 'CFO', '2020-02-03')");
    insert.exec();

    verifyTableNoBlobs(databaseConnection);
}

void DatabaseTests::verifyTableNoBlobs(const DatabaseConnection& databaseConnection)
{
    Query select(databaseConnection, "SELECT * FROM gtest_temp_table ORDER BY 1", false);
    select.open();
    int recordCount = 0;
    while (!select.eof())
    {
        ++recordCount;
        EXPECT_EQ(recordCount, select[0].asInteger());
        const char* expectedName = recordCount == 1 ? "John Doe" : "Jane Doe";
        EXPECT_STREQ(expectedName, select[1].asString().trim().c_str());
        select.next();
    }
    EXPECT_EQ(2, recordCount);
}

void DatabaseTests::testBulkInsertPerformance(const DatabaseConnectionString& connectionString, size_t recordCount)
{
    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTable(databaseConnection, false, false);

    const Query selectData(databaseConnection, "SELECT * FROM gtest_temp_table");
    InsertQuery insertData(databaseConnection, "INSERT INTO gtest_temp_table (name, position_name, hire_date) VALUES (:name, :position, :hired)");

    vector<VariantVector> data;
    VariantVector         keys;
    keys.reserve(recordCount);
    for (size_t i = 1; i <= recordCount; ++i)
    {
        keys.emplace_back(static_cast<int>(i));

        VariantVector row;
        row.emplace_back("Alex,'Doe'");
        row.emplace_back("Programmer");
        row.emplace_back("01-JAN-2014");
        data.push_back(std::move(row));
    }

    StopWatch stopWatch;
    stopWatch.start();
    const Strings columnNames({"name", "position_name", "hire_date"});
    databaseConnection->bulkInsert("gtest_temp_table", "id", columnNames, data);
    stopWatch.stop();
    const auto bulkInsertDurationMS = stopWatch.milliseconds();

    stopWatch.start();
    databaseConnection->bulkDelete("gtest_temp_table", "id", keys);
    stopWatch.stop();
    const auto bulkDeleteDurationMS = stopWatch.milliseconds();

    stopWatch.start();

    auto& nameParam = insertData.param("name");
    auto& positionParam = insertData.param("position");
    auto& hiredParam = insertData.param("hired");

    Transaction transaction(databaseConnection);
    transaction.begin();
    for (auto& row: data)
    {
        constexpr int col0 = 0;
        constexpr int col1 = 1;
        constexpr int col2 = 2;

        nameParam = row[col0].asString();
        positionParam = row[col1].asString();
        hiredParam = row[col2].asString();
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

    auto printResults = [&](const String& operation, double durationMs)
    {
        COUT(left << fixed << setw(25) << connectionString.driverName() << setw(14) << operation
                  << right << setw(8) << setprecision(1) << durationMs << " ms, "
                  << setprecision(2) << fixed << setw(10) << static_cast<double>(data.size()) / durationMs << "K rec/sec");
    };

    printResults("insert", insertDurationMS);
    printResults("bulk insert", bulkInsertDurationMS);
    printResults("delete", deleteDurationMS);
    printResults("bulk delete", bulkDeleteDurationMS);
    COUT("");
}

void DatabaseTests::testBatchSQL(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTable(databaseConnection, false, false);

    Query selectData(databaseConnection, "SELECT * FROM gtest_temp_table ORDER BY 1");

    const Strings batchSQL {
        "INSERT INTO gtest_temp_table(name, position_name, hire_date) VALUES ('John', 'CEO', '2020-01-02');",
        "INSERT INTO gtest_temp_table(name, position_name, hire_date) VALUES ('Jane', 'CFO', '2021-02-03');",
        "INSERT INTO gtest_temp_table(name, position_name, hire_date) VALUES ('William', 'CIO', '2022-03-04');"};

    const Strings invalidBatchSQL {
        "REMOVE INTO gtest_temp_table VALUES (2, 'Jane', 'CFO', '2021-02-03');",
        "INVENT INTO gtest_temp_table VALUES (3, 'William', 'CIO', '2022-03-04');"};

    const Strings expectedResults {
        "1,John,CEO,2020-01-02",
        "2,Jane,CFO,2021-02-03",
        "3,William,CIO,2022-03-04"};

    EXPECT_THROW(databaseConnection->executeBatchSQL(invalidBatchSQL), DatabaseException);

    databaseConnection->executeBatchSQL(batchSQL);

    verifyBatchInsertedData(selectData, expectedResults);
}

void DatabaseTests::verifyBatchInsertedData(Query& selectData, const Strings& expectedResults)
{
    selectData.open();
    int           rowNumber = 0;
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

    Query selectNullData(databaseConnection, "SELECT name, position_name, hire_date FROM gtest_temp_table WHERE name IS NULL");
    Query selectNotNullData(databaseConnection, "SELECT * FROM gtest_temp_table WHERE name IS NOT NULL");
    Query insertData(databaseConnection, "INSERT INTO gtest_temp_table (name, position_name, hire_date) VALUES (:name, :position, :hired)");

    Strings data;
    data.push_back(string("Alex,'Doe'\tProgrammer\t01-JAN-2014"));
    data.push_back(string("David\tCEO\t01-JAN-2015"));
    data.push_back(string("Roger\tBunny\t01-JAN-2016"));
    data.push_back(string("Teddy\tBear\t01-JAN-2017"));
    data.push_back(string("Santa\tClaus\t01-JAN-2018"));

    for (const auto& row: data)
    {
        constexpr int col0 = 0;
        constexpr int col1 = 1;
        constexpr int col2 = 2;
        using enum VariantDataType;

        // Insert all nulls
        insertData.param("name").setNull(VAR_STRING);
        insertData.param("position").setNull(VAR_STRING);
        insertData.param("hired").setNull(VAR_STRING);
        insertData.exec();

        // Insert data row
        Strings values(row, "\t");
        insertData.param("name") = values[col0];
        insertData.param("position") = values[col1];
        insertData.param("hired") = values[col2];
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
    size_t           recordCount = 0;

    selectNullData.open();
    while (!selectNullData.eof())
    {
        // Check if all fields are NULLs
        ranges::for_each(selectNullData.fields(),
                         [](const auto& field)
                         {
                             if (!field->isNull())
                             {
                                 throw Exception("Field " + field->fieldName() + " = [" + field->asString() + "] but null is expected");
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
        size_t  index = 0;
        for (const auto& field: selectNotNullData.fields())
        {
            if (field->isNull())
            {
                throw Exception("Field " + field->fieldName() + " is null but value is expected");
            }
            if (index > 0)
            {
                row.push_back(field->asString().trim());
            }
            ++index;
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
    const auto count = static_cast<size_t>(select["cnt"].asInteger());
    select.close();

    return count;
}

void DatabaseTests::testBLOB(const DatabaseConnectionString& connectionString)
{
    DatabaseConnectionPool   connectionPool(connectionString.toString());
    const DatabaseConnection databaseConnection = connectionPool.getConnection();
    createTestTable(databaseConnection, true, true);

    constexpr size_t blobSize1 = 65536;

    Buffer testData1(blobSize1);
    Buffer testDataInv(blobSize1);
    for (size_t i = 0; i < blobSize1; ++i)
    {
        constexpr size_t Value256 = 256;
        testData1[i] = static_cast<uint8_t>(i % Value256);
        testDataInv[i] = static_cast<uint8_t>(Value256 - i % Value256);
    }
    testData1.bytes(blobSize1);
    testDataInv.bytes(blobSize1);

    Buffer testData2(testDataInv);
    testData2.append(testDataInv);

    Query insertQuery(databaseConnection, "INSERT INTO gtest_temp_table(data1, data2) VALUES(:data1, :data2)");
    insertQuery.param("data1") = testData1;
    insertQuery.param("data2") = testData2;
    insertQuery.exec();

    Query selectQuery(databaseConnection, "SELECT id, data1, data2 FROM gtest_temp_table ORDER BY 1");
    selectQuery.open();

    constexpr size_t blobSize2 = blobSize1 * 2;
    const auto       dataSize1 = selectQuery["data1"].dataSize();
    EXPECT_EQ(blobSize1, dataSize1);

    const auto dataSize2 = selectQuery["data2"].dataSize();
    EXPECT_EQ(blobSize2, dataSize2);

    const auto* data = selectQuery["data1"].getText();
    for (size_t i = 0; i < blobSize1; ++i)
    {
        EXPECT_EQ(static_cast<char>(i % 256), data[i]);
    }

    data = selectQuery["data2"].getText();
    for (size_t i = 0; i < blobSize2; ++i)
    {
        EXPECT_EQ(static_cast<char>(256 - static_cast<char>(i % 256)), data[i]);
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
