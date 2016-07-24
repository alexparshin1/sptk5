/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       all_drivers_test.cpp - description                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

int testTransactions(DatabaseConnection* db, string tableName, bool rollback)
{
    try {
        Query step5Query(db, "DELETE FROM " + tableName, true, __FILE__, __LINE__);
        Query step6Query(db, "SELECT count(*) FROM " + tableName, true, __FILE__, __LINE__);

        step6Query.open();
        int counter = step6Query[uint32_t(0)].asInteger();
        step6Query.close();
        cout << "\n        The table has " << counter << " records.";

        cout << "\n        Begining the transaction ..";
        db->beginTransaction();
        cout << "\n        Deleting everything from the table ..";
        step5Query.exec();

        step6Query.open();
        counter = step6Query[uint32_t(0)].asInteger();
        step6Query.close();
        cout << "\n        The table now has " << counter << " records.";

        if (rollback) {
            cout << "\n        Rolling back the transaction ..";
            db->rollbackTransaction();
        } else {
            cout << "\n        Commiting the transaction ..";
            db->commitTransaction();
        }
        step6Query.open();
        counter = step6Query[uint32_t(0)].asInteger();
        step6Query.close();
        cout << "\n        The table now has " << counter << " records.";
    } catch (exception& e) {
        cout << "Error: " << e.what() << endl;
    }

    return true;
}

// This function returns field content as a string, or "<NULL>" is field
// contains a NULL value
string fieldToString(const Field& field)
{
    if (field.isNull())
        return "<NULL>";
    // Returned field is automatically converted to string
    return field;
}

void testBLOBs(DatabaseConnection* db)
{
    Query createTableQuery(db, "CREATE TABLE sptk_blob_test(id INT, data CLOB)", true, __FILE__, __LINE__);
    try {
        createTableQuery.exec();
    }
    catch (exception& e) {
        cout << e.what() << endl;
    }

    Query createBlobQuery(db, "INSERT INTO sptk_blob_test VALUES(:id, :data)", true, __FILE__, __LINE__);

    for (unsigned i = 0; i < 1000; i++) {
        createBlobQuery.param("id").setInteger(i);
        createBlobQuery.param("data").setText("This is a test " + int2string(i));
        createBlobQuery.exec();
    }

    Query selectBlobsQuery(db, "SELECT id, data FROM sptk_blob_test WHERE id < 10", true, __FILE__, __LINE__);
    selectBlobsQuery.open();
    while (!selectBlobsQuery.eof()) {
        cout << selectBlobsQuery["id"].asInteger()
        << ": "
        << selectBlobsQuery["data"].asString() << endl;
        selectBlobsQuery.fetch();
    }
    selectBlobsQuery.close();

    Query dropTableQuery(db, "DROP TABLE sptk_blob_test", true, __FILE__, __LINE__);
    dropTableQuery.exec();
}

int testDatabase(string connectionString)
{
    DatabaseConnectionPool connectionPool(connectionString);
    DatabaseConnection* db = connectionPool.createConnection();

    try {
        cout << "==========================================\n";
        cout << "Connection string: " << connectionString << "\n";
        cout << "Openning the database.. ";
        db->open();
        cout << "Ok.\nDriver description: " << db->driverDescription() << endl;

        //testBLOBs(db);

        DatabaseObjectType objectTypes[] = {DOT_TABLES, DOT_VIEWS, DOT_PROCEDURES};
        string objectTypeNames[] = {"tables", "views", "stored procedures"};

        for (unsigned i = 0; i < 3; i++) {
            cout << "-------------------------------------------------" << endl;
            cout << "First 10 " << objectTypeNames[i] << " in the database:" << endl;
            Strings objectList;
            try {
                db->objectList(objectTypes[i], objectList);
            } catch (exception& e) {
                cout << e.what() << endl;
            }
            for (unsigned j = 0; j < objectList.size() && j < 10; j++)
                cout << "  " << objectList[j] << endl;
        }
        cout << "-------------------------------------------------" << endl;

        // Defining the statements
        // Using __FILE__ in query constructor __LINE__ is optional and used for printing statistics only
        string tableName = "test_table";

        // TIMESTAMP type isn't the same for different servers
        string timestampTypeName = "TIMESTAMP";
        if (db->driverDescription().find("Microsoft") != string::npos)
            timestampTypeName = "DATETIME";

        Query step1Query(db,
                          "CREATE TABLE " + tableName + "(id INT, name CHAR(80), position_name CHAR(80), hire_date " +
                          timestampTypeName + ", rate NUMERIC(16,10))", false, __FILE__, __LINE__);
        Query step2Query(db, "INSERT INTO " + tableName +
                              " VALUES(:person_id,:person_name,:position_name,:hire_date,:rate)", true, __FILE__, __LINE__);
        Query step3Query(db, "SELECT * FROM " + tableName + " WHERE id > 1 OR id IS NULL", false, __FILE__, __LINE__);
        Query step4Query(db, "DROP TABLE " + tableName, false, __FILE__, __LINE__);

        cout << "Ok.\nStep 1: Creating the test table.. ";
        try {
            step1Query.exec();
            if (db->connectionType() == DatabaseConnection::DCT_FIREBIRD)
                db->commitTransaction(); // Some databases don't recognize table existense until it is committed
        } catch (exception& e) {
            if (strstr(e.what(), " already ") == NULL)
                throw;
            cout << "Table already exists, ";
        }

        cout << "Ok.\nStep 2: Inserting data into the test table.. ";

        // The following example shows how to use the paramaters,
        // addressing them by name
        step2Query.param("person_id") = 1;
        step2Query.param("person_name") = "John Doe";
        step2Query.param("position_name") = "CIO";
        step2Query.param("hire_date") = DateTime::Now();
        step2Query.param("rate") = 0.0000123;
        step2Query.exec();

        // Here is the example of using parameters by index.
        // This is the even faster than stream
        step2Query.param(uint32_t(0)) = 3;
        step2Query.param(1) = "UTF-8: тестик (Russian, 6 chars)";
        step2Query.param(2).setNull(VAR_STRING);
        step2Query.param(3).setDate(DateTime::Now());
        step2Query.param(4) = 12340.001234;
        step2Query.exec();
        /*
        step2Query.param(uint32_t(0)) = 3;
        step2Query.param(1) = "UTF-8: тестик (Russian, 6 chars)";
        step2Query.param(2) = "Manager";
        step2Query.param(3).setDate(DateTime::Now());
        step2Query.param(4).setFloat(12340.001234);
        step2Query.exec();
        */

        // And, finally - the fastest method: using CParam& variables.
        // If you have to call the same query multiple times with the different parameters,
        // that method gives you some extra gain.
        // So, lets define the parameter variables
        QueryParameter& id_param = step2Query.param("person_id");
        QueryParameter& name_param = step2Query.param("person_name");
        QueryParameter& position_param = step2Query.param("position_name");
        QueryParameter& hire_date_param = step2Query.param("hire_date");
        QueryParameter& rate_param = step2Query.param("rate");

        // Now, we can use these variables, re-defining their values before each .exec() if needed:
        id_param = 4;
        name_param = "Buffy";
        position_param = "Fearless fiction vampire slayer";
        hire_date_param.setDateTime(DateTime::Now());
        rate_param = 81.2345;
        step2Query.exec();

        // Now, we can use these variables
        id_param = 5;
        name_param = "Buffy 2";
        position_param = "Fearless fiction vampire slayer";
        rate_param = 82.3456;
        step2Query.exec();

        // .. and use these variables again for the next insert
        // This is the way to set fields to NULL:
        id_param.setNull();
        name_param.setNull();
        position_param.setNull();
        hire_date_param.setNull();
        rate_param.setNull();
        step2Query.exec();

        cout << "Ok.\nStep 3: Selecting the information the slow way .." << endl;
        step3Query.open();

        while (!step3Query.eof()) {

            // getting data from the query by the field name
            int64_t id = step3Query["id"];

            // we can check field for NULL value
            if (step3Query["id"].isNull())
                cout << setw(7) << "<NULL>";
            else
                cout << setw(7) << id;

            // Another method: getting data by the column number.
            // For printing values we use custom function implemented above
            string name = fieldToString(step3Query[1]);
            string position_name = fieldToString(step3Query[2]);
            string date = fieldToString(step3Query[3]);
            string rate = fieldToString(step3Query[4]);

            cout << " | " << setw(40) << name << " | " << setw(20) << position_name << " | " << date << " | " << rate <<
            endl;

            step3Query.fetch();
        }
        step3Query.close();

        cout << "Ok.\nStep 4: Selecting the information through the field iterator .." << endl;
        step3Query.param("some_id") = 1;
        step3Query.open();

        while (!step3Query.eof()) {

            int id;
            string name, position_name, hire_date;

            int fieldIndex = 0;
            for (Field* field: step3Query.fields()) {
                switch (fieldIndex) {
                    case 0: id = field->asInteger(); break;
                    case 1: name = field->asString(); break;
                    case 2: position_name = field->asString(); break;
                    case 3: hire_date = field->asString(); break;
                }
                fieldIndex++;
            }

            cout << setw(4) << id << " | " << setw(20) << name << " | " << position_name << " | " << hire_date << endl;

            step3Query.fetch();
        }
        step3Query.close();

        cout << "Ok.\nStep 5: Selecting the information the fast way .." << endl;
        step3Query.open();

        // First, find the field references by name or by number
        Field& idField = step3Query[uint32_t(0)];
        Field& nameField = step3Query["name"];
        Field& positionNameField = step3Query["position_name"];
        Field& dateField = step3Query["hire_date"];
        Field& rateField = step3Query["rate"];

        while (!step3Query.eof()) {

            int64_t id = idField;
            string name = nameField;
            string position_name = positionNameField;
            string hire_date = dateField;
            string rate = rateField;

            cout << setw(7) << id << " | " << setw(40) << name << " | " << setw(20) << position_name << " | " <<
            hire_date << " | " << rate << endl;

            step3Query.fetch();
        }
        step3Query.close();

        cout << "Ok.\n***********************************************\nTesting the transactions.\n";

        cout << endl;
        cout << "- Start transaction, delete all records from test table, then rollback:" << endl;
        cout << "  After the test, table should have same number of records as before test." << endl;
        testTransactions(db, tableName, true);

        cout << endl << endl;
        cout << "- Start transaction, delete all records from test table, then rollback:" << endl;
        cout << "  After the test, table should have no records." << endl;
        testTransactions(db, tableName, false);
        cout << endl;

        step4Query.exec();

        cout << "Ok.\nStep 6: Closing the database.. ";
        db->close();
    } catch (exception& e) {
        cout << "\nError: " << e.what() << endl;
        return 1;
    }

    return 0;
}

// If a single command line parameter supplied,
// use it as database connection string
int main(int argc, const char* argv[])
{
    string connectionString;
    if (argc == 2)
        connectionString = argv[1];
    else {
        connectionString = "mssql://protis:wsxedc@Protis/protis";
        //connectionString = "postgresql://localhost/protis_global";
    }

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
            NULL};

    if (connectionString.empty()) {
        Strings databaseTypes;
        for (size_t i = 0; availableDatabaseTypes[i] != NULL; i++)
            databaseTypes.push_back(string(availableDatabaseTypes[i]));

        string dbtype, dbname, username, password, hostOrDSN;
        for (; ;) {
            cout << "Please select database type (" << databaseTypes.asString(",") << ")> ";
            cin >> dbtype;
            if (databaseTypes.indexOf(dbtype) != -1)
                break;
        }

        if (dbtype == "odbc") {
            cout << "DSN name > ";
            cin >> hostOrDSN;
        } else {
            cout << "hostname (or localhost) > ";
            cin >> hostOrDSN;
            cout << "Database name > ";
            cin >> dbname;
        }

        cout << "User name > ";
        cin >> username;

        cout << "Password > ";
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

    cout << "Connection string: " << connectionString << endl;

    testDatabase(connectionString);
}
