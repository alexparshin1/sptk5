/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       odbc_test2.cpp - description                           ║
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

#include <stdio.h>
#include <stdlib.h>

#include <sptk5/CException.h>

#include <sptk5/db/CODBCConnection.h>
#include <sptk5/db/CQuery.h>

using namespace std;
using namespace sptk;

// This function tries to execute the query. If the query
// was prepared earlier and the resourse it is using was
// dropped or altered, it switches to no-auto-prepare mode.
// Then the execution attempt is repeated.
void smartQueryExec(CQuery& query)
{
    try {
        query.exec();
        return;
    }
    catch (exception& e) {
        if (!query.autoPrepare())
            throw;   // Query is already in no-auto-prepare mode
        if (strstr(e.what(), "altered") == 0L)
            throw;   // Table was not altered - something else is wrong
    }
    query.autoPrepare(false);
    query.unprepare();
    query.exec();
}

// This function tries to open the query. If the query
// was prepared earlier and the resourse it is using was
// dropped or altered, it switches to no-auto-prepare mode.
// Then the open attempt is repeated.
void smartQueryOpen(CQuery& query)
{
    try {
        query.open();
        return;
    }
    catch (exception& e) {
        if (!query.autoPrepare())
            throw;   // Query is already in no-auto-prepare mode
        if (strstr(e.what(), "altered") == 0L)
            throw;   // Table was not altered - something else is wrong
    }
    query.autoPrepare(false);
    query.unprepare();
    query.open();
}

// This function creates the temporary table, fills it with data,
// prints the data from the table, and deletes the table.
// At the first call ALL the queries are automatically auto-prepared.
// The second call makes the auto-prepared queries to try to work 
// with destroyed and created table.
void testPrepareAndUnprepare(
        CQuery& step1Query,
        CQuery& step2Query,
        CQuery& step3Query,
        CQuery& step4Query
)
{
    printf("Ok.\nStep 1: Creating the temp table.. ");
    smartQueryExec(step1Query);

    printf("Ok.\nStep 2: Inserting data into the temp table.. ");

    // The following example shows how to use the paramaters
    step2Query.param("person_id") = 1;
    step2Query.param("person_name") = "John Doe";
    smartQueryExec(step2Query);

    step2Query.param("person_id") = 2;
    step2Query.param("person_name") = "Jane Doe";
    smartQueryExec(step2Query);

    printf("Ok.\nStep 3: Selecting the information from the table ..\n");
    //step3Query.param("some_id") = 1;
    smartQueryOpen(step3Query);

    while (!step3Query.eof()) {

        // getting data from the query by the field name
        int id = step3Query["id"].asInteger();

        // another method - getting data by the column number
        string name = step3Query["name"].asString();

        printf("\t%i\t%s\n", id, name.c_str());

        step3Query.fetch();
    }
    step3Query.close();

    smartQueryExec(step4Query);

    puts("Ok.\n***********************************************");
}

int main()
{

    puts("This program tests how the prepare()/unprepare() methods work\n");

    // If you want to test the database abilities of the data controls
    // you have to setup the ODBC database connection.
    // Typical connect string is something like: "DSN=odbc_demo;UID=user;PWD=password".
    // If UID or PWD are omitted they are read from the datasource settings.
    CODBCConnection db("DSN=odbc_demo");

    for (unsigned i = 0; i < 2; i++) {
        try {
            printf("Openning the database.. ");
            db.open();
            printf("Ok.\nDriver description: %s\n", db.driverDescription().c_str());

            string tableName = "test_odbc";
            string isTemp = "TEMP";
            if (db.driverDescription() == "SQL Server") {
                tableName = "##test_odbc";
                isTemp = "";
            }
            // Defining the queries
            CQuery step1Query(&db, "CREATE " + isTemp + " TABLE " + tableName + "(id INT,name CHAR(20))");
            CQuery step2Query(&db, "INSERT INTO " + tableName + " VALUES ( :person_id, :person_name )");
            CQuery step3Query(&db, "SELECT * FROM " + tableName);
            CQuery step4Query(&db, "DROP TABLE " + tableName);

            puts("Pass 1 of the test");
            testPrepareAndUnprepare(
                    step1Query, step2Query, step3Query, step4Query);

            puts("Pass 2 of the test");
            testPrepareAndUnprepare(step1Query, step2Query, step3Query, step4Query);

            puts("Pass 3 of the test");
            testPrepareAndUnprepare(step1Query, step2Query, step3Query, step4Query);

            printf("Ok.\nStep 6: Closing the database.. ");
            db.close();
            printf("Ok.\n");
        }
        catch (exception& e) {
            db.close();
            printf("\nError: %s\n", e.what());
            puts("\nSorry, you have to fix your database connection.");
            puts("Please, read the README.txt for more information.");
        }
    }

    return 0;
}
