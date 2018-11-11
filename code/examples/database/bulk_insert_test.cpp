/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       bulk_insert_test.cpp - description                     ║
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

using namespace std;
using namespace sptk;

int main()
{
    try {
        //DatabaseConnectionPool connectionPool("postgresql://localhost/test");
        //DatabaseConnectionPool connectionPool("mysql://localhost/test");
        DatabaseConnectionPool connectionPool("oracle://protis:xxxxx@theater/XE");
        DatabaseConnection db = connectionPool.getConnection();

        cout << "Openning the database.. ";
        db->open();

        // Defining the queries
        // Using __FILE__ in query constructor __LINE__ is optional and used for printing statistics only
        string tableName = "test_table";
        Query step1Query(db, "CREATE TABLE " + tableName +
                             "(id INT,name CHAR(40),position_name CHAR(20),hire_date TIMESTAMP)", true, __FILE__,
                         __LINE__);
        Query step3Query(db, "SELECT * FROM " + tableName + " WHERE id > :some_id OR id IS NULL", true, __FILE__,
                         __LINE__);
        Query step4Query(db, "DROP TABLE " + tableName, true, __FILE__, __LINE__);

        cout << "Ok.\nStep 1: Creating the test table.. ";
        try {
            step1Query.exec();
        } catch (exception& e) {
            if (strstr(e.what(), "exist") == nullptr)
                throw;
            cout << "Table already exists, ";
        }

        cout << "Ok.\nStep 2: Inserting data into the test table.. ";
        Strings columnNames("id,name,position_name,hire_date", ",");

        Strings data;
        data.push_back(string("1\tAlex\tProgrammer\t01-JAN-2014"));
        data.push_back(string("2\tDavid\tCEO\t01-JAN-2014"));
        data.push_back(string("3\tRoger\tBunny\t01-JAN-2014"));

        db->bulkInsert(tableName, columnNames, data);

        cout << "Ok.\nStep 3: Selecting the information through the field iterator .." << endl;
        step3Query.param("some_id") = 1;
        step3Query.open();

        while (!step3Query.eof()) {

            int id = 0;
            String name, position_name, hire_date;

            int fieldIndex = 0;
            for (Field* field: step3Query.fields()) {
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

            cout << setw(4) << id << " | " << setw(20) << name << " | " << position_name << " | " << hire_date << endl;

            step3Query.fetch();
        }
        step3Query.close();

        step4Query.open();
        cout << "Ok." << endl;
    } catch (exception& e) {
        cout << "\nError: " << e.what() << endl;
        cout << "\nSorry, you have to fix your database connection." << endl;
        cout << "Please, read the README.txt for more information." << endl;
    }

    return 0;
}

