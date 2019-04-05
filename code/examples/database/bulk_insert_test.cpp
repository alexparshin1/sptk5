/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       bulk_insert_test.cpp - description                     ║
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

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <sptk5/cutils>
#include <sptk5/cdatabase>

using namespace std;
using namespace sptk;

void createTable(DatabaseConnection db, const String& tableName)
{
    Query step1Query(db, "CREATE TABLE " + tableName +
                         "(id INT,name CHAR(40),position_name CHAR(20),hire_date TIMESTAMP)", true);
    try {
        step1Query.exec();
    } catch (const Exception& e) {
        if (strstr(e.what(), "exist") == nullptr)
            throw;
        COUT("Table already exists, ");
    }
}

int main()
{
    try {
        //DatabaseConnectionPool connectionPool("postgresql://localhost/test");
        //DatabaseConnectionPool connectionPool("mysql://localhost/test");
        DatabaseConnectionPool connectionPool("oracle://protis:xxxxx@theater/XE");
        DatabaseConnection db = connectionPool.getConnection();

        COUT("Openning the database.. ");
        db->open();

        // Defining the queries
        string tableName = "test_table";
        Query step3Query(db, "SELECT * FROM " + tableName + " WHERE id > :some_id OR id IS NULL", true);
        Query step4Query(db, "DROP TABLE " + tableName, true);

        COUT("Ok.\nStep 1: Creating the test table.. ");
        createTable(db, tableName);

        COUT("Ok.\nStep 2: Inserting data into the test table.. ");
        Strings columnNames("id,name,position_name,hire_date", ",");

        vector<VariantVector> data;
        VariantVector arow;

        arow.emplace_back(1);
        arow.emplace_back("Alex");
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

        db->bulkInsert(tableName, columnNames, data);

        COUT("Ok.\nStep 3: Selecting the information through the field iterator .." << endl);
        step3Query.param("some_id") = 1;
        step3Query.open();

        while (!step3Query.eof()) {

            int id = 0;
            String name;
            String position_name;
            String hire_date;

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

            COUT(setw(4) << id << " | " << setw(20) << name << " | " << position_name << " | " << hire_date << endl);

            step3Query.fetch();
        }
        step3Query.close();

        step4Query.open();
        COUT("Ok." << endl);
    } catch (const Exception& e) {
        CERR("\nError: " << e.what() << endl);
        CERR("\nSorry, you have to fix your database connection." << endl);
        CERR("Please, read the README.txt for more information." << endl);
    }

    return 0;
}

