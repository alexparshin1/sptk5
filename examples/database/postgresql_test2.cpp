/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          postgresql_test2.cpp  -  description
                             -------------------
    begin                : September 20, 2007
    copyright            : (C) 1999-2013 by Alexey S.Parshin
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

#include <sptk5/db/CODBCConnection.h>
#include <sptk5/db/CPostgreSQLConnection.h>
#include <sptk5/cdatabase>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

unsigned idOffset = 1;

void testPerformance (CDatabaseConnection& db)
{
    string tableName("test_table");
    try {
        cout << "Openning the database.. ";
        db.open();
        cout << "Ok.\nDriver description: " << db.driverDescription() << endl;

        // Defining the queries
        string tableNdbdbame = "test_table2";
        CQuery step1Query (&db, "CREATE TABLE " + tableName + "(id INT PRIMARY KEY,name CHAR(20),position CHAR(20),salary DECIMAL(10,2),description text,salary2 float,name2 CHAR(20),dt date,ts timestamp)");
        CQuery step2Query (&db, "INSERT INTO " + tableName + " VALUES(:person_id,:person_name,:position_name,:salary,:description,:salary,:person_name,:dt,:ts)");
        CQuery step3Query (&db, "SELECT * FROM " + tableName + " WHERE id=:person_id");
        //CQuery step3Query(&db,"SELECT dt,ts FROM "+tableName+" WHERE id=:person_id");
        CQuery step4Query (&db, "DROP TABLE " + tableName);
        CQuery deleteQuery (&db, "DELETE FROM " + tableName);

        cout << "Ok.\nStep 1: Creating the test table.. ";
        try {
            step1Query.exec();
        } catch (exception& e) {
            if (strstr (e.what(), "already exists") == NULL)
                throw;
            cout << "Table already exists, remove all the records: ";
            deleteQuery.exec();
        }

        unsigned rows = 50000;
        cout << "Ok.\nStep 2: CPostgreSQLConnection benchmark (binary binding)" << endl;
        cout << "  * Executing " << rows << " inserts : ";

        // The following example shows how to use the paramaters,
        // addressing them by name
        CDateTime start, end;
        double seconds;

        unsigned baseId = idOffset;

        start  = CDateTime::Now();

        CParam& id_param = step2Query.param ("person_id");
        CParam& name_param = step2Query.param ("person_name");
        CParam& position_param = step2Query.param ("position_name");
        CParam& salary_param = step2Query.param ("salary");
        CParam& description_param = step2Query.param ("description");
        CParam& date_param = step2Query.param ("dt");
        CParam& time_param = step2Query.param ("ts");
        CDateTime now = CDateTime::Now();
        for (unsigned i = 0; i < rows; i++) {
            id_param = idOffset++;
            name_param.setExternalString ("Buffy");
            position_param.setExternalString ("Vampire slayer");
            salary_param = 70000.01 + i * 10;
            description_param.setExternalString ("This is a description");
            date_param.setDateTime (now);
            time_param.setDateTime (now);
            step2Query.exec();
        }

        end = CDateTime::Now();
        seconds = (end - start) * 24 * 60 * 60;
        cout << seconds << " seconds (" << int (rows / seconds) << " rows/sec)" << endl;

        cout << "  * Executing " << rows << " selects : ";
        start  = CDateTime::Now();

        CParam& personIdParam = step3Query.param ("person_id");
        string name, position, description;
        double salary;
        int id;
        for (unsigned i = 0; i < rows; i++) {
            personIdParam = i + baseId;
            step3Query.open();
            id = step3Query["id"].getInteger();
            name = step3Query["name"].getString();
            position = step3Query["position"].getString();
            salary = step3Query["salary"].getFloat();
            description = step3Query["description"].getString();
            step3Query.close();
        }

        end = CDateTime::Now();
        seconds = (end - start) * 24 * 60 * 60;
        cout << seconds << " seconds (" << int (rows / seconds) << " rows/sec)" << endl;

        deleteQuery.exec();

        cout << "Step 3: Libpq direct calls (AD-HOC SQL)" << endl;
        cout << "  * Executing " << rows << " inserts : ";

        baseId = idOffset;
        start  = CDateTime::Now();

        PGconn *conn = ( (CPostgreSQLConnection*) & db)->connection();

        for (unsigned i = 0; i < rows; i++) {
            string sql = "INSERT INTO " + tableName
                         + " VALUES(" + int2string (idOffset++) + ",'Buffy','Vampire slayer',70000.01,'This is a description',70000.01,'Buffy','2007-10-02','2007-10-02 10:00:00')";
            PGresult* res = PQexec (conn, sql.c_str());
            if (PQresultStatus (res) != PGRES_COMMAND_OK) {
                string error = "insert failed: ";
                error += PQerrorMessage (conn);
                PQclear (res);
                throw CException (error);
            }
            PQclear (res);
        }

        end = CDateTime::Now();
        seconds = (end - start) * 24 * 60 * 60;
        cout << seconds << " seconds (" << int (rows / seconds) << " rows/sec)" << endl;

        cout << "  * Executing " << rows << " selects : ";

        start  = CDateTime::Now();

        for (unsigned i = 0; i < rows; i++) {
            string sql = "SELECT * FROM " + tableName  + " WHERE id=" + int2string (i + baseId);
            PGresult* res = PQexec (conn, sql.c_str());
            if (PQresultStatus (res) != PGRES_TUPLES_OK) {
                string error = "select failed: ";
                error += PQerrorMessage (conn);
                PQclear (res);
                throw CException (error);
            }
            //int rowCount = PQntuples (res);
            //int fieldCount = PQnfields (res);
            id = atoi (PQgetvalue (res, 0, 0));
            name = PQgetvalue (res, 0, 1);
            position = PQgetvalue (res, 0, 2);
            salary = atof (PQgetvalue (res, 0, 3));
            description = PQgetvalue (res, 0, 4);
            PQclear (res);
        }

        end = CDateTime::Now();
        seconds = (end - start) * 24 * 60 * 60;
        cout << seconds << " seconds (" << int (rows / seconds) << " rows/sec)" << endl;

        cout << "Ok.\nLast Step: Closing the database.. ";
        db.close();
        cout << "Ok." << endl;
    } catch (exception& e) {
        cout << "\nError: " << e.what() << endl;
        cout << "\nSorry, you have to fix your database connection." << endl;
        cout << "Please, read the README.txt for more information." << endl;
    }
}

int main()
{
    CDatabaseConnectionPool connectionPool("postgresql://localhost/test");
    CDatabaseConnection* db = connectionPool.createConnection();

    testPerformance(*db);
}
