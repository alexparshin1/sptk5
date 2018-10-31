/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       odbc_test.cpp - description                            ║
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

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <iostream>
#include <iomanip>

#include <sptk5/cdatabase>
#include <sptk5/cthreads>

using namespace std;
using namespace sptk;

int testTransactions(DatabaseConnection db, const string& tableName, bool rollback)
{
    try {
        Query step5Query(db, "DELETE FROM " + tableName);
        Query step6Query(db, "SELECT count(*) FROM " + tableName);

        cout << "\n        Begining the transaction ..";
        db->beginTransaction();
        cout << "\n        Deleting everything from the table ..";
        step5Query.exec();

        step6Query.open();
        int counter = step6Query[uint32_t(0)].asInteger();
        step6Query.close();
        cout << "\n        The table now has " << counter << " records ..";

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
        cout << "\n        The table now has " << counter << " records..";
    } catch (exception& e) {
        cout << "Error: " << e.what() << endl;
    }

    return true;
}

int main(int argc, const char* argv[])
{
    String connectString;
    if (argc == 1)
        connectString = "odbc://user:password@demo_odbc";
    else
        connectString = argv[1];

    try {
        if (!RegularExpression("^odbc://").matches(connectString)) {
            cout << "Syntax:" << endl << endl;
            cout << "odbc_test [connection string]" << endl << endl;
            cout << "Connection string has format: odbc://[user:password]@<odbc_dsn>," << endl;
            cout << "for instance:" << endl << endl;
            cout << "  odbc://alex:secret@mydsn" << endl;
            return 1;
        }

        DatabaseConnectionPool connectionPool(connectString);
        DatabaseConnection db = connectionPool.getConnection();

        cout << "Openning the database, using connection string " << connectString << ":" << endl;
        db->open();
        cout << "Ok.\nDriver description: " << db->driverDescription() << endl;

        DatabaseObjectType objectTypes[] = { DOT_TABLES, DOT_VIEWS, DOT_PROCEDURES };
        string objectTypeNames[] = { "tables", "views", "stored procedures" };

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

        // Defining the queries
        string tableName = "test_table";
        Query step1Query(db, "CREATE TABLE " + tableName + "(id INT,name CHAR(20),position CHAR(20))");
        Query step2Query(db, "INSERT INTO " + tableName + " VALUES(:person_id,:person_name,:position_name)");
        Query step3Query(db, "SELECT * FROM " + tableName + " WHERE id > :some_id");
        Query step4Query(db, "DROP TABLE " + tableName);

        cout << "Ok.\nStep 1: Creating the test table.. ";
        try {
            step1Query.exec();
        } catch (exception& e) {
            if (strstr(e.what(), "already exists") == nullptr)
                throw;
            cout << "Table already exists, ";
        }

        cout << "Ok.\nStep 2: Inserting data into the test table.. ";

        // The following example shows how to use the paramaters,
        // addressing them by name
        step2Query.param("person_id") = 1;
        step2Query.param("person_name") = "John Doe";
        step2Query.param("position_name") = "CIO";
        step2Query.exec();

        // Here is the example of using parameters as a stream.
        // That method is several times faster than access by field name
        // Params should come in the exact order as they are defined in the query
        // Here is the example of using parameters by index.
        // This is the even faster than stream
        step2Query.param(uint32_t(0)) = 3;
        step2Query.param(uint32_t(1)) = "Anonymous";
        step2Query.param(uint32_t(2)) = "Manager";
        step2Query.exec();

        // And, finally - the fastest method: using CParam& variables.
        // If you have to call the same query multiple times with the different parameters,
        // that method gives you some extra gain.
        // So, lets define the parameter variables
        QueryParameter& id_param = step2Query.param("person_id");
        QueryParameter& name_param = step2Query.param("person_name");
        QueryParameter& position_param = step2Query.param("position_name");
        // Now, we can use these variables
        id_param = 4;
        name_param = "Buffy";
        position_param = "Vampire slayer";
        step2Query.exec();
        // .. and use these variables again for the next insert
        id_param = 5;
        name_param = "Donald Duck";
        position_param.setNull(); // This is the way to set field to NULL
        step2Query.exec();

        cout << "Ok.\nStep 3: Selecting the information the slow way .." << endl;
        step3Query.param("some_id") = 1;
        step3Query.open();

        while (!step3Query.eof()) {

            // getting data from the query by the field name
            int id = step3Query["id"];

            // another method - getting data by the column number
            string name = step3Query[1];
            string position = step3Query[2];

            cout << setw(4) << id << " | " << setw(20) << name << " | " << position << endl;

            step3Query.fetch();
        }
        step3Query.close();

        cout << "Ok.\nStep 4: Selecting the information the fast way .." << endl;
        step3Query.param("some_id") = 1;
        step3Query.open();

        // First, find the field references by name or by number
        Field& idField = step3Query[uint32_t(0)];
        Field& nameField = step3Query["name"];
        Field& positionField = step3Query["position"];

        while (!step3Query.eof()) {

            int id = idField;
            string name = nameField;
            string position = positionField;

            cout << setw(4) << id << " | " << setw(20) << name << " | " << position << endl;

            step3Query.fetch();
        }
        step3Query.close();

        cout << "Ok.\n***********************************************\nTesting the transactions.";

        testTransactions(db, tableName, true);
        testTransactions(db, tableName, false);

        step4Query.exec();

        cout << "Ok.\nStep 5: Closing the database.. ";
        db->close();
        cout << "Ok." << endl;
    } catch (exception& e) {
        cout << "\nError: " << e.what() << endl;
        cout << "\nSorry, you have to fix your database connection." << endl;
        cout << "Please, read the README.txt for more information." << endl;
    }

    this_thread::sleep_for(chrono::milliseconds(1000));

    return 0;
}
