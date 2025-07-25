/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       all_drivers_test.cpp - description                     ║
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

#include <iostream>
#include <iomanip>

#include <sptk5/cdatabase>
#include <sptk5/Printer.h>

using namespace std;
using namespace sptk;

bool testTransactions(DatabaseConnection db, const string& tableName, bool rollback)
{
    try {
        Query step5Query(db, "DELETE FROM " + tableName, true);
        Query step6Query(db, "SELECT count(*) FROM " + tableName, true);

        step6Query.open();
        int counter = step6Query[uint32_t(0)].asInteger();
        step6Query.close();
        COUT("\n        The table has " << counter << " records.");

        COUT("\n        Begining the transaction ..");
        db->beginTransaction();
        COUT("\n        Deleting everything from the table ..");
        step5Query.exec();

        step6Query.open();
        counter = step6Query[uint32_t(0)].asInteger();
        step6Query.close();
        COUT("\n        The table now has " << counter << " records.");

        if (rollback) {
            COUT("\n        Rolling back the transaction ..");
            db->rollbackTransaction();
        } else {
            COUT("\n        Commiting the transaction ..");
            db->commitTransaction();
        }
        step6Query.open();
        counter = step6Query[uint32_t(0)].asInteger();
        step6Query.close();
        COUT("\n        The table now has " << counter << " records.");
    } catch (const Exception& e) {
        CERR("Error: " << e.what() << endl);
    }

    return true;
}

// This function returns field content as a string, or "<NULL>" is field
// contains a NULL value
String fieldToString(const Field& field)
{
    if (field.isNull())
        return "<NULL>";
    // Returned field is automatically converted to string
    return field.asString();
}

void testBLOBs(PoolDatabaseConnection* db)
{
    Query createTableQuery(db, "CREATE TABLE sptk_blob_test(id INT, data CLOB)", true);
    try {
        createTableQuery.exec();
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
    }

    Query createBlobQuery(db, "INSERT INTO sptk_blob_test VALUES(:id, :data)", true);

    for (unsigned i = 0; i < 1000; i++) {
        createBlobQuery.param("id").setInteger(i);
        String text("This is a test " + to_string(i));
        createBlobQuery.param("data").setBuffer(text.c_str(), text.size());
        createBlobQuery.exec();
    }

    Query selectBlobsQuery(db, "SELECT id, data FROM sptk_blob_test WHERE id < 10", true);
    selectBlobsQuery.open();
    while (!selectBlobsQuery.eof()) {
        COUT(selectBlobsQuery["id"].asInteger()
                     << ": "
                     << selectBlobsQuery["data"].asString() << endl);
        selectBlobsQuery.fetch();
    }
    selectBlobsQuery.close();

    Query dropTableQuery(db, "DROP TABLE sptk_blob_test", true);
    dropTableQuery.exec();
}

void printDatabaseObjects(DatabaseConnection db)
{
    DatabaseObjectType objectTypes[] = {DOT_TABLES, DOT_VIEWS, DOT_PROCEDURES};
    string objectTypeNames[] = {"tables", "views", "stored procedures"};

    for (unsigned i = 0; i < 3; i++) {
        COUT("-------------------------------------------------" << endl);
        COUT("First 10 " << objectTypeNames[i] << " in the database:" << endl);
        Strings objectList;
        try {
            db->objectList(objectTypes[i], objectList);
        } catch (const Exception& e) {
            CERR(e.what() << endl);
        }
        for (unsigned j = 0; j < objectList.size() && j < 10; j++) COUT("  " << objectList[j] << endl);
    }
    COUT("-------------------------------------------------" << endl);
}

void createTempTable(DatabaseConnection db, const String& tableName)
{
    Query createTempTableQuery(db);
    if (db->driverDescription().find("Microsoft") != string::npos)
        createTempTableQuery.sql(
                "CREATE TABLE " + tableName + "("
                                              "id             INT NULL, "
                                              "name           NCHAR(80) NULL, "
                                              "position_name  NCHAR(80) NULL, "
                                              "hire_date      DATETIME NULL, "
                                              "rate           NUMERIC(16,10) NULL)");
    else
        createTempTableQuery.sql(
                "CREATE TABLE " + tableName + "("
                                              "id             INT, "
                                              "name           CHAR(80), "
                                              "position_name  CHAR(80), "
                                              "hire_date      TIMESTAMP, "
                                              "rate           NUMERIC(16,10))");

    try {
        createTempTableQuery.exec();
        if (db->connectionType() == DCT_FIREBIRD)
            db->commitTransaction(); // Some databases don't recognize table existense until it is committed
    } catch (const Exception& e) {
        if (strstr(e.what(), " already ") == nullptr)
            throw;
        COUT("Table already exists, ");
        Query deleteAll(db, "delete from " + tableName);
        deleteAll.exec();
    }
}

void dropTempTable(const DatabaseConnection& db, const string& tableName)
{
    Query dropTempTableQuery(db, "DROP TABLE " + tableName, false);
    try {
        dropTempTableQuery.exec();
    }
    catch (const Exception& e) {
        COUT("Couldn't drop temp table " << tableName << ": " << e.what());
    }
}

int testDatabase(const string& connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString);
    DatabaseConnection db = connectionPool.getConnection();

    try {
        COUT("==========================================\n");
        COUT("Connection string: " << connectionString << "\n");
        COUT("Openning the database.. ");
        db->open();
        COUT("Ok.\nDriver description: " << db->driverDescription() << endl);
        printDatabaseObjects(db);

        // Defining the statements
        String tableName = "test_table";

        Query insertRecordQuery(db, "INSERT INTO " + tableName +
                                    " VALUES(:person_id,:person_name,:position_name,:hire_date,:rate)", true);
        Query selectRecordsQuery(db, "SELECT * FROM " + tableName + " WHERE id >= 1 OR id IS NULL", false);

        dropTempTable(db, tableName);

        COUT("Ok.\nStep 1: Creating the test table.. ");
        createTempTable(db, tableName);

        COUT("Ok.\nStep 2: Inserting data into the test table.. ");

        // The following example shows how to use the paramaters,
        // addressing them by name
        insertRecordQuery.param("person_id") = 1;
        insertRecordQuery.param("person_name") = "John Doe";
        insertRecordQuery.param("position_name") = "CIO";
        insertRecordQuery.param("hire_date") = DateTime::Now();
        insertRecordQuery.param("rate") = 0.0000123;
        insertRecordQuery.exec();

        // Here is the example of using parameters by index.
        // This is the even faster than stream
        insertRecordQuery.param(uint32_t(0)) = 3;
        insertRecordQuery.param(1) = "UTF-8: тестик (Russian, 6 chars)";
        insertRecordQuery.param(2).setNull(VAR_STRING);
        insertRecordQuery.param(3).setDateTime(DateTime::Now(), true);
        insertRecordQuery.param(4) = 12340.001234;
        insertRecordQuery.exec();

        // And, finally - the fastest method: using CParam& variables.
        // If you have to call the same query multiple times with the different parameters,
        // that method gives you some extra gain.
        // So, lets define the parameter variables
        QueryParameter& id_param = insertRecordQuery.param("person_id");
        QueryParameter& name_param = insertRecordQuery.param("person_name");
        QueryParameter& position_param = insertRecordQuery.param("position_name");
        QueryParameter& hire_date_param = insertRecordQuery.param("hire_date");
        QueryParameter& rate_param = insertRecordQuery.param("rate");

        // Now, we can use these variables, re-defining their values before each .exec() if needed:
        id_param = 4;
        name_param = "Buffy";
        position_param = "Fearless fiction vampire slayer";
        hire_date_param.setDateTime(DateTime::Now());
        rate_param = 81.2345;
        insertRecordQuery.exec();

        // Now, we can use these variables
        id_param = 5;
        name_param = "Buffy 2";
        position_param = "Fearless fiction vampire slayer";
        rate_param = 82.3456;
        insertRecordQuery.exec();

        // .. and use these variables again for the next insert
        // This is the way to set fields to NULL:
        id_param.setNull();
        name_param.setNull();
        position_param.setNull();
        hire_date_param.setNull();
        rate_param.setNull();
        insertRecordQuery.exec();

        COUT("Ok.\nStep 3: Selecting the information the slow way .." << endl);
        selectRecordsQuery.open();

        while (!selectRecordsQuery.eof()) {

            // getting data from the query by the field name
            auto id = selectRecordsQuery["id"].asInteger();

            // we can check field for NULL value
            if (selectRecordsQuery["id"].isNull()) {
                COUT(setw(7) << "<NULL>");
            } else {
                COUT(setw(7) << id);
            }

            // Another method: getting data by the column number.
            // For printing values we use custom function implemented above
            string name = fieldToString(selectRecordsQuery[1]);
            string position_name = fieldToString(selectRecordsQuery[2]);
            string date = fieldToString(selectRecordsQuery[3]);
            string rate = fieldToString(selectRecordsQuery[4]);

            COUT(" | " << setw(40) << name << " | " << setw(20) << position_name << " | " << date << " | " << rate <<
                       endl);

            selectRecordsQuery.fetch();
        }
        selectRecordsQuery.close();

        COUT("Ok.\nStep 4: Selecting the information through the field iterator .." << endl);
        selectRecordsQuery.open();

        while (!selectRecordsQuery.eof()) {

            int id = 0;
            String name;
            String position_name;
            String hire_date;

            int fieldIndex = 0;
            for (Field* field: selectRecordsQuery.fields()) {
                switch (fieldIndex) {
                    case 0:
                        id = field->asInteger();
                        break;
                    case 1:
                        name = field->asString();
                        break;
                    case 2:
                        position_name = field->asString();
                        break;
                    case 3:
                        hire_date = field->asString();
                        break;
                    default:
                        break;
                }
                fieldIndex++;
            }

            COUT(setw(4) << id << " | " << setw(20) << name << " | " << position_name << " | " << hire_date << endl);

            selectRecordsQuery.fetch();
        }
        selectRecordsQuery.close();

        COUT("Ok.\nStep 5: Selecting the information the fast way .." << endl);
        selectRecordsQuery.open();

        // First, find the field references by name or by number
        Field& idField = selectRecordsQuery[uint32_t(0)];
        Field& nameField = selectRecordsQuery["name"];
        Field& positionNameField = selectRecordsQuery["position_name"];
        Field& dateField = selectRecordsQuery["hire_date"];
        Field& rateField = selectRecordsQuery["rate"];

        while (!selectRecordsQuery.eof()) {

            auto id = idField.asInteger();
            auto name = nameField.asString();
            auto position_name = positionNameField.asString();
            auto hire_date = dateField.asString();
            auto rate = rateField.asString();

            COUT(setw(7) << id << " | " << setw(40) << name << " | " << setw(20) << position_name << " | " <<
                         hire_date << " | " << rate << endl);

            selectRecordsQuery.fetch();
        }
        selectRecordsQuery.close();

        COUT("Ok.\n***********************************************\nTesting the transactions.\n");

        COUT(endl);
        COUT("- Start transaction, delete all records from test table, then rollback:" << endl);
        COUT("  After the test, table should have same number of records as before test." << endl);
        testTransactions(db, tableName, true);

        COUT(endl << endl);
        COUT("- Start transaction, delete all records from test table, then rollback:" << endl);
        COUT("  After the test, table should have no records." << endl);
        testTransactions(db, tableName, false);
        COUT(endl);

        COUT("Ok.\nStep 6: Closing the database.. ");
        db->close();
    } catch (const Exception& e) {
        CERR("\nError: " << e.what() << endl);
        return 1;
    }

    return 0;
}

// If a single command line parameter supplied,
// use it as database connection string
int main(int argc, const char* argv[])
{
    try {
        string connectionString;
        if (argc == 2)
            connectionString = argv[1];
        else {
            //connectionString = "oracle://protis:pass@oracledb/protis";
            connectionString = "mssql://gtest:test#123@dsn_mssql/gtest";
            //connectionString = "postgresql://localhost/test";
        }

        if (connectionString.empty()) {
            Strings databaseTypes;
            const char* availableDatabaseTypes[] = {
#if HAVE_MYSQL == 1
                    "mysql",
#endif
#if HAVE_ORACLE == 1
                    "oracle",
#endif
#if HAVE_POSTGRESQL == 1
                    "postgres",
#endif
#if HAVE_ODBC == 1
                    "odbc",
                    "mssql",
#endif
#if HAVE_FIREBIRD == 1
                    "firebird",
#endif
                    nullptr};

            for (size_t i = 0; availableDatabaseTypes[i] != nullptr; i++)
                databaseTypes.push_back(string(availableDatabaseTypes[i]));

            String dbtype;
            String dbname;
            String username;
            String password;
            String hostOrDSN;

            for (;;) {
                COUT("Please select database type (" << databaseTypes.join(",") << ")> ");
                cin >> dbtype;
                if (databaseTypes.indexOf(dbtype) != -1)
                    break;
            }

            if (dbtype == "odbc") {
                COUT("DSN name > ");
                cin >> hostOrDSN;
            } else {
                COUT("hostname (or localhost) > ");
                cin >> hostOrDSN;
                COUT("Database name > ");
                cin >> dbname;
            }

            COUT("User name > ");
            cin >> username;

            COUT("Password > ");
            cin >> password;

            // Creating connection string in the following format:
            // <dbtype>://[username[:password]@]<host_or_DSN>[:port_number][/dbname]

            connectionString = dbtype + "://";
            if (!username.empty()) {
                connectionString += username;
                if (!password.empty())
                    connectionString += ":" + password;
                connectionString += "@";
            }

            connectionString += hostOrDSN;
            if (!dbname.empty()) {
                if (dbname[0] == '/')
                    connectionString += dbname;
                else
                    connectionString += "/" + dbname;
            }
        }

        COUT("Connection string: " << connectionString << endl);

        return testDatabase(connectionString);
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
        return 1;
    }
}
