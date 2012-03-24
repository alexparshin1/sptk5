/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDatabaseDriver.cpp  -  description
                             -------------------
    begin                : Wed Dec 15 1999
    copyright            : (C) 1999-2012 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/db/CDatabaseDriver.h>
#include <sptk5/db/CQuery.h>
#include <sptk5/CException.h>

#include <algorithm>

using namespace std;
using namespace sptk;

CDatabaseDriver::CDatabaseDriver(string connectionString) :
        m_connString(connectionString)
{
    m_inTransaction = false;
    m_log = 0;
}

CDatabaseDriver::~CDatabaseDriver()
{
    // To prevent the exceptions, if the database connection
    // is terminated already
    try {
        while (m_queryList.size()) {
            CQuery *query = (CQuery *) m_queryList[0];
            query->disconnect();
        }
    }
    catch (...) {
    }
}

bool CDatabaseDriver::linkQuery(CQuery *q)
{
    m_queryList.push_back(q);
    return true;
}

bool CDatabaseDriver::unlinkQuery(CQuery *q)
{
    CQueryVector::iterator itor = find(m_queryList.begin(), m_queryList.end(), q);
    m_queryList.erase(itor);
    return true;
}

void CDatabaseDriver::openDatabase(string newConnectionString) throw (CException)
{
    notImplemented("openDatabase");
}

void CDatabaseDriver::open(string newConnectionString) throw (CException)
{
    clearStatistics();
    openDatabase(newConnectionString);
    if (m_log)
        *m_log << "Opened database: " << m_connString << endl;
}

void CDatabaseDriver::closeDatabase() throw (CException)
{
    notImplemented("closeDatabase");
}

void CDatabaseDriver::close() throw (CException)
{
    if (active()) {
        if (m_inTransaction) {
            rollbackTransaction();
            m_inTransaction = false;
        }
        for (uint32_t i = 0; i < m_queryList.size(); i++) {
            CQuery& query = *m_queryList[i];
            query.storeStatistics();
            query.closeQuery(true);
        }
        closeDatabase();
        if (m_log)
            *m_log << "Closed database: " << m_connString << endl;
    }
}

void* CDatabaseDriver::handle() const
{
    notImplemented("handle");
    return 0;
}

bool CDatabaseDriver::active() const
{
    notImplemented("active");
    return true;
}

void CDatabaseDriver::beginTransaction() throw (CException)
{
    if (m_log)
        *m_log << "Begin transaction" << endl;
    driverBeginTransaction();
}

void CDatabaseDriver::commitTransaction() throw (CException)
{
    if (m_log)
        *m_log << "Commit transaction" << endl;
    driverEndTransaction(true);
}

void CDatabaseDriver::rollbackTransaction() throw (CException)
{
    if (m_log)
        *m_log << "Rollback transaction" << endl;
    driverEndTransaction(false);
}

//-----------------------------------------------------------------------------------------------

string CDatabaseDriver::queryError(const CQuery *) const
{
    notImplemented("queryError");
    return "";
}

void CDatabaseDriver::querySetAutoPrep(CQuery *q, bool pf)
{
    q->m_autoPrepare = pf;
}

void CDatabaseDriver::querySetStmt(CQuery *q, void *stmt)
{
    q->m_statement = stmt;
}

void CDatabaseDriver::querySetConn(CQuery *q, void *conn)
{
    q->m_connection = conn;
}

void CDatabaseDriver::querySetPrepared(CQuery *q, bool pf)
{
    q->m_prepared = pf;
}

void CDatabaseDriver::querySetActive(CQuery *q, bool af)
{
    q->m_active = af;
}

void CDatabaseDriver::querySetEof(CQuery *q, bool eof)
{
    q->m_eof = eof;
}

void CDatabaseDriver::queryAllocStmt(CQuery *)
{
    notImplemented("queryAllocStmt");
}

void CDatabaseDriver::queryFreeStmt(CQuery *)
{
    notImplemented("queryFreeStmt");
}

void CDatabaseDriver::queryCloseStmt(CQuery *)
{
    notImplemented("queryCloseStmt");
}

void CDatabaseDriver::queryPrepare(CQuery *)
{
    notImplemented("queryPrepare");
}

void CDatabaseDriver::queryUnprepare(CQuery *query)
{
    queryFreeStmt(query);
}

void CDatabaseDriver::queryExecute(CQuery *)
{
    notImplemented("queryExecute");
}

int CDatabaseDriver::queryColCount(CQuery *)
{
    notImplemented("queryColCount");
    return 0;
}

void CDatabaseDriver::queryColAttributes(CQuery *, int16_t, int16_t, int32_t&)
{
    notImplemented("queryColAttributes");
}

void CDatabaseDriver::queryColAttributes(CQuery *, int16_t, int16_t, char *, int32_t)
{
    notImplemented("queryColAttributes");
}

void CDatabaseDriver::queryBindParameters(CQuery *)
{
    notImplemented("queryBindParameters");
}

void CDatabaseDriver::queryOpen(CQuery *)
{
    notImplemented("queryOpen");
}

void CDatabaseDriver::queryFetch(CQuery *)
{
    notImplemented("queryFetch");
}

void CDatabaseDriver::notImplemented(const char *methodName) const
{
    throw CException("Method '" + string(methodName) + "' is not supported by this database driver.");
}

void *CDatabaseDriver::queryHandle(CQuery *query) const
{
    return query->m_statement;
}

void CDatabaseDriver::queryHandle(CQuery *query, void *handle)
{
    query->m_statement = handle;
}

void CDatabaseDriver::logAndThrow(string method, string error) throw (CException)
{
    string errorText("Exception in " + method + ": " + error);
    if (m_log)
        *m_log << "errorText" << endl;
    throw CException(errorText);
}

void CDatabaseDriver::clearStatistics()
{
    m_queryStatisticMap.clear();
}

void CDatabaseDriver::addStatistics(const std::string& location, double totalDuration, unsigned totalCalls, const std::string& sql)
{
    CCallStatisticMap::iterator itor = m_queryStatisticMap.find(location);
    if (itor != m_queryStatisticMap.end()) {
        CCallStatistic& stat = itor->second;
        stat.m_duration += totalDuration;
        stat.m_calls += totalCalls;
        stat.m_sql = sql;
    } else {
        CCallStatistic stat(totalDuration, totalCalls, sql);
        m_queryStatisticMap[location] = stat;
    }
}
