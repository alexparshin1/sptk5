/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       sqlite3_test.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/Printer.h>
#include <sptk5/cdatabase>
#include <sptk5/db/SQLite3Connection.h>

#include <iomanip>

using namespace std;
using namespace sptk;

int testTransactions(DatabaseConnection db, const String& tableName, bool rollback)
{
    try
    {
        Query step5Query(db, "DELETE FROM " + tableName);
        Query step6Query(db, "SELECT count(*) FROM " + tableName);

        COUT("\n        Begining the transaction ..");
        db->beginTransaction();
        COUT("\n        Deleting everything from the temp table ..");
        step5Query.exec();

        step6Query.open();
        int counter = step6Query[static_cast<uint32_t>(0)].asInteger();
        step6Query.close();
        COUT("\n        The temp table now has " << counter << " records ..");

        if (rollback)
        {
            COUT("\n        Rolling back the transaction ..");
            db->rollbackTransaction();
        }
        else
        {
            COUT("\n        Commiting the transaction ..");
            db->commitTransaction();
        }
        step6Query.open();
        counter = step6Query[static_cast<uint32_t>(0)].asInteger();
        step6Query.close();
        COUT("\n        The temp table now has " << counter << " records..\n");
    }
    catch (const Exception& e)
    {
        CERR("Error: " << e.what());
    }

    return true;
}

int main()
{
    DatabaseConnectionPool connectionPool("sqlite3://localhost/tmp/demo_db.sqlite3");
    DatabaseConnection     db = connectionPool.getConnection();

    try
    {
        COUT("Openning the database.. ");
        db->open();
        COUT("Ok.\nDriver description: " << db->driverDescription() << '\n');

        Strings tableList;
        db->objectList(DatabaseObjectType::TABLES, tableList);
        COUT("First 10 tables in the database:\n");
        for (unsigned i = 0; i < tableList.size() && i < 10; i++)
            COUT("  Table: " << tableList[i] << '\n');

        // Defining the queries
        Query step1Query(db, "CREATE TABLE test(id INT PRIMARY KEY,name CHAR(20),position CHAR(20))");
        Query step2Query(db, "INSERT INTO test VALUES(:person_id,:person_name,:position_name)");
        Query step3Query(db, "SELECT * FROM test WHERE id > :some_id");
        Query step4Query(db, "DROP TABLE test");

        COUT("Ok.\nStep 1: Creating the table.. ");
        step1Query.exec();

        COUT("Ok.\nStep 2: Inserting data into the table.. ");

        // The following example shows how to use the parameters,
        // addressing them by name
        step2Query.param("person_id") = 1;
        step2Query.param("person_name") = "John Doe";
        step2Query.param("position_name") = "CIO";
        step2Query.exec();

        // Here is the example of using parameters by index.
        // This is the even faster than stream
        step2Query.param(static_cast<size_t>(0)) = 3;
        step2Query.param(1) = "Anonymous";
        step2Query.param(2) = "Manager";
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

        COUT("Ok.\nStep 3: Selecting the information the slow way ..\n");
        step3Query.param("some_id") = 1;
        step3Query.open();

        while (!step3Query.eof())
        {

            // getting data from the query by the field name
            int64_t id = step3Query["id"].asInt64();

            // another method - getting data by the column number
            String name = step3Query[1].asString();
            String position = step3Query[2].asString();

            COUT(setw(10) << id << setw(20) << name << setw(20) << position << '\n');

            step3Query.fetch();
        }
        step3Query.close();


        COUT("Ok.\nStep 4: Selecting the information the fast way ..\n");
        step3Query.param("some_id") = 1;
        step3Query.open();

        // First, find the field references by name or by number
        Field& idField = step3Query[static_cast<uint32_t>(0)];
        Field& nameField = step3Query["name"];
        Field& positionField = step3Query["position"];

        while (!step3Query.eof())
        {

            int  id = idField.asInteger();
            auto name = nameField.asString();
            auto position = positionField.asString();

            COUT(setw(10) << id << setw(20) << name << setw(20) << position << '\n');

            step3Query.fetch();
        }
        step3Query.close();

        COUT("Ok.\n***********************************************\nTesting the transactions.");

        testTransactions(db, "test", true);
        testTransactions(db, "test", false);

        step4Query.exec();

        COUT("Ok.\nStep 6: Closing the database.. ");
        db->close();
        COUT("Ok.\n");
    }
    catch (const Exception& e)
    {
        CERR("\nError: " << e.what());
        CERR("Sorry, you have to fix your database or database connection.");
        CERR("Please, read the README.txt for more information.");
    }

    return 0;
}
