/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          postgresql_test.cpp  -  description
                             -------------------
    begin                : September 20, 2007
    copyright            : (C) 2008-2012 by Alexey S.Parshin
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

#include <sptk5/db/CPostgreSQLDatabase.h>
//#include <sptk5/CODBCDatabase.h>
#include <sptk5/cdatabase>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

int testTransactions(CDatabase&db,string tableName,bool rollback) {
    try {
        CQuery  step5Query(&db,"DELETE FROM "+tableName,__FILE__,__LINE__);
        CQuery  step6Query(&db,"SELECT count(*) FROM "+tableName,__FILE__,__LINE__);

        cout << "\n        Begining the transaction ..";
        db.beginTransaction();
        cout << "\n        Deleting everything from the table ..";
        step5Query.exec();

        step6Query.open();
        int counter = step6Query[uint32_t(0)].asInteger();
        step6Query.close();
        cout << "\n        The table now has " << counter << " records ..";

        if (rollback) {
            cout << "\n        Rolling back the transaction ..";
            db.rollbackTransaction();
        } else {
            cout << "\n        Commiting the transaction ..";
            db.commitTransaction();
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

// This function returns field content as a string, or "<NULL>" is field
// contains a NULL value
string fieldToString(const CField& field) {
    if (field.isNull())
        return "<NULL>";
    // Returned field is automatically converted to string
    return field; 
}

int main() {

    // If you want to test the database abilities of the data controls
    // you have to setup the PostgreSQL database, and verify connection.
    // Typical connect string is something like: "dbname='mydb' host='myhostname' port=5142" and so on.
    // For mor information please refer to:
    // http://www.postgresql.org/docs/current/interactive/libpq-connect.html
    // If connect string is empty the default database with the name equal to user name is used.
    string              connectString("dbname='practica'");
    CPostgreSQLDatabase db(connectString);
    //CODBCDatabase 	db("DSN=uu");
    CFileLog            logFile("postgresql_test.log");

    db.logFile(&logFile);
    logFile.reset();

    try {
        cout << "Openning the database.. ";
        db.open();
        cout << "Ok.\nDriver description: " << db.driverDescription() << endl;

        CDbObjectType objectTypes[] = { DOT_TABLES, DOT_VIEWS, DOT_PROCEDURES };
        string    objectTypeNames[] = { "tables", "views", "stored procedures"};

        for (unsigned i = 0; i < 3; i++) {
            cout << "-------------------------------------------------" << endl;
            cout << "First 10 " << objectTypeNames[i] << " in the database:" << endl;
            CStrings objectList;
            try {
                db.objectList(objectTypes[i],objectList);
            }
            catch (exception& e) {
                cout << e.what() << endl;
            }
            for (unsigned i = 0; i < objectList.size() && i < 10; i++)
                cout << "  " << objectList[i] << endl;
        }
        cout << "-------------------------------------------------" << endl;

        // Defining the queries
	// Using __FILE__ in query constructor __LINE__ is optional and used for printing statistics only
        string tableName = "test_table";
        CQuery step1Query(&db,"CREATE TABLE "+tableName+"(id INT8,name CHAR(40),position CHAR(20),date TIMESTAMP)",__FILE__,__LINE__);
        CQuery step2Query(&db,"INSERT INTO "+tableName+" VALUES(:person_id,:person_name,:position_name,:date)",__FILE__,__LINE__);
        CQuery step3Query(&db,"SELECT * FROM "+tableName+" WHERE id > :some_id OR id IS NULL",__FILE__,__LINE__);
        CQuery step4Query(&db,"DROP TABLE "+tableName,__FILE__,__LINE__);

        cout << "Ok.\nStep 1: Creating the test table.. ";
        try {
            step1Query.exec();
        }
        catch (exception& e) {
            if (strstr(e.what(),"already exists") == NULL)
                throw;
            cout << "Table already exists, ";
        }

        cout << "Ok.\nStep 2: Inserting data into the test table.. ";

        // The following example shows how to use the paramaters,
        // addressing them by name
        CDateTime start,end;

        step2Query.param("person_id") = 1;
        step2Query.param("person_name") = "John Doe";
        step2Query.param("position_name") = "CIO";
        step2Query.param("date") = CDateTime::Now();
        step2Query.exec();

        // Here is the example of using parameters as a stream.
        // That method is several times faster than access by field name
        // Params should come in the exact order as they are defined in the query
        step2Query.params() << 2 << "Jane Doe" << "Vise President";
        step2Query.exec();

        // Here is the example of using parameters by index.
        // This is the even faster than stream
        step2Query.param(uint32_t(0)) = 3;
        step2Query.param(uint32_t(1)) = "UTF-8: тестик (Russian, 6 chars)";
        step2Query.param(uint32_t(2)) = "Manager";
        step2Query.exec();

        // And, finally - the fastest method: using CParam& variables.
        // If you have to call the same query multiple times with the different parameters,
        // that method gives you some extra gain.
        // So, lets define the parameter variables
        CParam& id_param = step2Query.param("person_id");
        CParam& name_param = step2Query.param("person_name");
        CParam& position_param = step2Query.param("position_name");
        
        // Now, we can use these variables
        id_param = 4;
        name_param = "Buffy";
        position_param = "Vampire slayer";
        step2Query.exec();
        
        // .. and use these variables again for the next insert
        // This is the way to set fields to NULL:
        id_param.setNull();
        name_param.setNull();
        position_param.setNull(); 
        step2Query.exec();

        cout << "Ok.\nStep 3: Selecting the information the slow way .." << endl;
        step3Query.param("some_id") = 1;
        step3Query.open();

        while ( ! step3Query.eof() ) {

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
            string position = fieldToString(step3Query[2]);
            string date = fieldToString(step3Query[3]);
            
            cout << " | "  << setw(40) << name << " | " << setw(20) << position << " | " << date << endl;

            step3Query.fetch();
        }
        step3Query.close();

        cout << "Ok.\nStep 4: Selecting the information through the stream .." << endl;
        step3Query.param("some_id") = 1;
        step3Query.open();

        while ( ! step3Query.eof() ) {

            int id;
            string name, position, date;

            step3Query.fields() >> id >> name >> position >> date;

            cout << setw(4) << id << " | "  << setw(20) << name << " | " << position <<  " | " << date << endl;

            step3Query.fetch();
        }
        step3Query.close();           

        cout << "Ok.\nStep 5: Selecting the information the fast way .." << endl;
        step3Query.param("some_id") = 1;
        step3Query.open(); 

        // First, find the field references by name or by number
        CField& idField = step3Query[uint32_t(0)];
        CField& nameField = step3Query["name"];
        CField& positionField = step3Query["position"];
        CField& dateField = step3Query["date"];

        while ( ! step3Query.eof() ) {

            int64_t id = idField;
            string name = nameField;
            string position = positionField;
            string date = dateField;

            cout << setw(4) << id << " | "  << setw(20) << name << " | " << position << endl;

            step3Query.fetch();
        }
        step3Query.close();

        cout << "Ok.\n***********************************************\nTesting the transactions.";

        testTransactions(db,tableName,true);
        testTransactions(db,tableName,false);

        step4Query.exec();

        cout << "Ok.\nStep 6: Closing the database.. ";
        db.close();
	
        cout << "Ok.\n***********************************************\nPrinting the query statistics." << endl;

	CCallStatisticMap::const_iterator itor = db.callStatistics().begin();
	CCallStatisticMap::const_iterator etor = db.callStatistics().end();
	for ( ; itor != etor; itor++ ) {
	    const CCallStatistic& cs = itor->second;
	    cout << setw(60) << cs.m_sql << ": " << cs.m_calls << " calls, " << cs.m_duration << " seconds total" << endl;
	}
	
        cout << "Ok." << endl;
    } catch (exception& e) {
        cout << "\nError: " << e.what() << endl;
        cout << "\nSorry, you have to fix your database connection." << endl;
        cout << "Please, read the README.txt for more information." << endl;
    }

    return 0;
}

