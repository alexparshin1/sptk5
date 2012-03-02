/***************************************************************************
 SIMPLY POWERFUL TOOLKIT (SPTK)
 DEMO PROGRAMS SET
 odbc_test3.cpp  -  description

 This test creates several threads with the independent database
 connection per thread.

 -------------------
 begin                : October 15, 2005
 copyright            : (C) 2005-2012 by Alexey S.Parshin
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

#include <sptk5/sptk.h>
#include <iostream>

#include <sptk5/db/CODBCDatabase.h>
#include <sptk5/db/CQuery.h>
#include <sptk5/threads/CThread.h>

using namespace std;
using namespace sptk;

/// Opens a database connection and inserts
/// 100 of records to the database. After that
/// verifies that all the records are inserted
/// properly.
class CInsertThread: public CThread
{
    static int threadIndex;

    CODBCDatabase m_db;
    CQuery m_query;
    int m_threadIndex;
    int m_rowsToInsert;

public:
    CInsertThread(std::string connectString, int rowsToInsert);
    virtual void threadFunction();
};

int CInsertThread::threadIndex = 0;

CInsertThread::CInsertThread(std::string connectString, int rowsToInsert) :
    CThread("Thread " + int2string(threadIndex)),
    m_db(connectString),
    m_query(&m_db, ""),
    m_threadIndex(threadIndex)
{
    threadIndex++;
    m_rowsToInsert = rowsToInsert;
}

void CInsertThread::threadFunction()
{
    try {
        // Opening the database
        m_db.open();

        m_query.sql("INSERT INTO test_table ("
                "record_id,"
                "iEventEntryId,"
                "iAcqEntryId,"
                "dtOrigAcqDate, "
                "dtAcqDate, "
                "dtTradeDate,"
                "dtEventDate,"
                "dUnits,"
                "decIndexedCB,"
                "decReducedCB,"
                "decAdjustedCB,"
                "decIndexedGain,"
                "decAdjustedGain,"
                "decReducedGain,"
                "decDiscountable,"
                "bExempt,"
                "bIndexedOK,"
                "bAdjustedOK,"
                "bReducedOK,"
                "bDiscountOK,"
                "decBaseCostPerUnit,"
                "decCostPerUnit"
                ") VALUES ("
                ":record_id,"
                ":iEventEntryId,"
                ":iAcqEntryId,"
                ":dtOrigAcqDate,"
                ":dtAcqDate,"
                ":dtTradeDate,"
                ":dtEventDate,"
                ":dUnits,"
                ":decIndexedCB,"
                ":decReducedCB,"
                ":decAdjustedCB,"
                ":decIndexedGain,"
                ":decAdjustedGain,"
                ":decReducedGain,"
                ":decDiscountable,"
                ":bExempt,"
                ":bIndexedOK,"
                ":bAdjustedOK,"
                ":bReducedOK,"
                ":bDiscountOK,"
                ":decBaseCostPerUnit,"
                ":decCostPerUnit)");

        CParam& record_id = m_query.param("record_id");
        CParam& iEventEntryId = m_query.param("iEventEntryId");
        CParam& iAcqEntryId = m_query.param("iAcqEntryId");
        CParam& dtOrigAcqDate = m_query.param("dtOrigAcqDate");
        CParam& dtAcqDate = m_query.param("dtAcqDate");
        CParam& dtTradeDate = m_query.param("dtTradeDate");
        CParam& dtEventDate = m_query.param("dtEventDate");
        CParam& dUnits = m_query.param("dUnits");
        CParam& decIndexedCB = m_query.param("decIndexedCB");
        CParam& decReducedCB = m_query.param("decReducedCB");
        CParam& decAdjustedCB = m_query.param("decAdjustedCB");
        CParam& decIndexedGain = m_query.param("decIndexedGain");
        CParam& decAdjustedGain = m_query.param("decAdjustedGain");
        CParam& decReducedGain = m_query.param("decReducedGain");
        CParam& decDiscountable = m_query.param("decDiscountable");
        CParam& bExempt = m_query.param("bExempt");
        CParam& bIndexedOK = m_query.param("bIndexedOK");
        CParam& bAdjustedOK = m_query.param("bAdjustedOK");
        CParam& bReducedOK = m_query.param("bReducedOK");
        CParam& bDiscountOK = m_query.param("bDiscountOK");
        CParam& decBaseCostPerUnit = m_query.param("decBaseCostPerUnit");
        CParam& decCostPerUnit = m_query.param("decCostPerUnit");

        CDateTime dt = (int) CDateTime::Now();
        for (int i = 1; i <= m_rowsToInsert; i++) {
            int id = m_threadIndex * m_rowsToInsert + i;
            record_id = id;
            iEventEntryId = 1;
            iAcqEntryId = 1;

            dtOrigAcqDate = dt;
            dtAcqDate = dt;
            dtTradeDate = dt;
            dtEventDate = dt;

            dUnits = 1;
            decIndexedCB = 1;
            decReducedCB = 1;
            decAdjustedCB = 1;
            decIndexedGain = 1;
            decAdjustedGain = 1;
            decReducedGain = 1;
            decDiscountable = 1;
            bExempt = 1;
            bIndexedOK = 1;
            bAdjustedOK = 1;
            bReducedOK = 1;
            bDiscountOK = 1;
            decBaseCostPerUnit = 1;
            decCostPerUnit = 1;

            m_query.exec();
        }
        printf("Thread %i completed\n", m_threadIndex);
        m_db.close();
    } catch (exception& e) {
        printf("Error in thread %i: %s\n", m_threadIndex, e.what());
    } catch (...) {
        printf("Unknown error in thread %i\n", m_threadIndex);
    }
}

int main()
{
    unsigned threadNumber;
    unsigned maxThreads;

    cout << "Number of threads: ";
    cout.flush();
    cin >> maxThreads;
    cout << "Beginning test " << endl;

    CODBCDatabase db1("DSN=odbc_demo");

    try {

        db1.open();
        string driver = lowerCase(db1.driverDescription());
        string dateType("TIMESTAMP");

        cout << "Connected to [" << db1.driverDescription() << "]" << endl;

        if (driver.find("microsoft") != string::npos)
            dateType = "DATETIME";
        else if (driver.find("informix") != string::npos)
            dateType = "DATETIME YEAR TO SECOND";

        CQuery createTable(
                &db1,
                "CREATE TABLE test_table ("
                        "record_id INT,"
                        "iEventEntryId INT,"
                        "iAcqEntryId INT,"
                        "dtOrigAcqDate " + dateType + ", "
                        "dtAcqDate " + dateType + ", "
                        "dtTradeDate " + dateType + ", "
                        "dtEventDate " + dateType + ", "
                        "dUnits DECIMAL(18,6),"
                        "decIndexedCB DECIMAL(18,2),"
                        "decReducedCB DECIMAL(18,2),"
                        "decAdjustedCB DECIMAL(18,2),"
                        "decIndexedGain DECIMAL(18,2),"
                        "decAdjustedGain DECIMAL(18,2),"
                        "decReducedGain DECIMAL(18,2),"
                        "decDiscountable DECIMAL(18,2),"
                        "bExempt INT DEFAULT 0,"
                        "bIndexedOK INT DEFAULT 0,"
                        "bAdjustedOK INT DEFAULT 0,"
                        "bReducedOK INT DEFAULT 0,"
                        "bDiscountOK INT DEFAULT 0,"
                        "decBaseCostPerUnit DECIMAL(18,6),"
                        "decCostPerUnit DECIMAL(18,6),"
                        "CONSTRAINT PK_test_table PRIMARY KEY (record_id, iEventEntryId, iAcqEntryId))");
        CQuery dropTable(&db1, "DROP TABLE test_table");
        CQuery cleanTable(&db1, "DELETE FROM test_table");
        CQuery countTable(&db1, "SELECT count(*) FROM test_table");

        cleanTable.exec();

        vector<CInsertThread *> threads;

        CDateTime started = CDateTime::Now();

        int recordsTotal = (10000 / maxThreads) * maxThreads;
        int recordsPerThread = recordsTotal / maxThreads;
        for (threadNumber = 0; threadNumber < maxThreads; threadNumber++) {
            CInsertThread *thread = new CInsertThread(db1.connectionString(),
                    recordsPerThread);
            threads.push_back(thread);
            thread->run();
        }

        //CThread::msleep(1000);

        for (threadNumber = 0; threadNumber < maxThreads; threadNumber++) {
            cout << "Waiting for thread " << threadNumber << endl;
            try {
                CInsertThread *thread = threads[threadNumber];
                while (thread->running())
                    CThread::msleep(100);
                delete thread;
            } catch (exception& e) {
                printf("exception deleting thread %i: %s\n", threadNumber,
                        e.what());
            } catch (...) {
                printf("exception deleting thread %i: %s\n", threadNumber,
                        "Unknown exception");
            }
        }

        CDateTime ended = CDateTime::Now();
        double durationSec = (ended - started) * 24 * 3600;

        countTable.open();
        recordsTotal = countTable[uint32_t(0)];
        countTable.close();

        cout << "Total of " << recordsTotal << " inserted for " << durationSec
                << " seconds." << endl;
        cout << "Average for " << maxThreads << " threads is "
                << recordsTotal / durationSec << " recs/sec." << endl;

        CThread::msleep(3000);
    } catch (exception& e) {
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "Unknown exception" << endl;
    }
    return 0;
}
