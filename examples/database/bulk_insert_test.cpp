/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          postgresql_test.cpp  -  description
                             -------------------
    begin                : September 20, 2007
    copyright            : (C) 1999-2016 by Alexey S.Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <iostream>
#include <iomanip>

#include <sptk5/cdatabase>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

int main()
{
    //CDatabaseConnectionPool connectionPool("postgresql://localhost/test");
    //CDatabaseConnectionPool connectionPool("mysql://localhost/test");
    CDatabaseConnectionPool connectionPool("oracle://protis:wsxedc@theater/XE");
    CDatabaseConnection* db = connectionPool.createConnection();

    CFileLog logFile("postgresql_test.log");

    db->logFile(&logFile);
    logFile.reset();

    try {
        cout << "Openning the database.. ";
        db->open();

        // Defining the queries
        // Using __FILE__ in query constructor __LINE__ is optional and used for printing statistics only
        string tableName = "test_table";
        CQuery step1Query(db, "CREATE TABLE " + tableName + "(id INT,name CHAR(40),position_name CHAR(20),hire_date TIMESTAMP)", true, __FILE__, __LINE__);
        CQuery step3Query(db, "SELECT * FROM " + tableName + " WHERE id > :some_id OR id IS NULL", true, __FILE__, __LINE__);
        CQuery step4Query(db, "DROP TABLE " + tableName, true, __FILE__, __LINE__);

        cout << "Ok.\nStep 1: Creating the test table.. ";
        try {
            step1Query.exec();
        } catch (exception& e) {
            if (strstr(e.what(), "exist") == NULL)
                throw;
            cout << "Table already exists, ";
        }

        cout << "Ok.\nStep 2: Inserting data into the test table.. ";
        CStrings columnNames("id,name,position_name,hire_date", ",");

        CStrings data;
        data.push_back(string("1\tAlex\tProgrammer\t01-JAN-2014"));
        data.push_back(string("2\tDavid\tCEO\t01-JAN-2014"));
        data.push_back(string("3\tRoger\tBunny\t01-JAN-2014"));

        db->bulkInsert(tableName, columnNames, data);

        cout << "Ok.\nStep 3: Selecting the information through the stream .." << endl;
        step3Query.param("some_id") = 1;
        step3Query.open();

        while (!step3Query.eof()) {

            int id;
            string name, position_name, hire_date;

            step3Query.fields() >> id >> name >> position_name >> hire_date;

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

