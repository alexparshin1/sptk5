/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDatabaseConnection.cpp - description                  ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/db/CDatabaseConnection.h>
#include <sptk5/db/CQuery.h>

using namespace std;
using namespace sptk;

CDatabaseConnection::CDatabaseConnection(string connectionString) :
        m_connString(connectionString)
{
    m_inTransaction = false;
    m_log = 0;
}

CDatabaseConnection::~CDatabaseConnection()
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

bool CDatabaseConnection::linkQuery(CQuery *q)
{
    m_queryList.push_back(q);
    return true;
}

bool CDatabaseConnection::unlinkQuery(CQuery *q)
{
    CQueryVector::iterator itor = find(m_queryList.begin(), m_queryList.end(), q);
    m_queryList.erase(itor);
    return true;
}

void CDatabaseConnection::openDatabase(string newConnectionString) THROWS_EXCEPTIONS
{
    notImplemented("openDatabase");
}

void CDatabaseConnection::open(string newConnectionString) THROWS_EXCEPTIONS
{
    openDatabase(newConnectionString);
    if (m_log)
        *m_log << "Opened database: " << m_connString.str() << endl;
}

void CDatabaseConnection::closeDatabase() THROWS_EXCEPTIONS
{
    //closeDatabase();
}

void CDatabaseConnection::close() THROWS_EXCEPTIONS
{
    if (active()) {
        if (m_inTransaction) {
            rollbackTransaction();
            m_inTransaction = false;
        }
        for (uint32_t i = 0; i < m_queryList.size(); i++) {
            CQuery& query = *m_queryList[i];
            query.closeQuery(true);
        }
        closeDatabase();
        if (m_log)
            *m_log << "Closed database: " << m_connString.str() << endl;
    }
}

void* CDatabaseConnection::handle() const
{
    notImplemented("handle");
    return 0;
}

bool CDatabaseConnection::active() const
{
    notImplemented("active");
    return true;
}

void CDatabaseConnection::beginTransaction() THROWS_EXCEPTIONS
{
    if (m_log)
        *m_log << "Begin transaction" << endl;
    driverBeginTransaction();
}

void CDatabaseConnection::commitTransaction() THROWS_EXCEPTIONS
{
    if (m_log)
        *m_log << "Commit transaction" << endl;
    driverEndTransaction(true);
}

void CDatabaseConnection::rollbackTransaction() THROWS_EXCEPTIONS
{
    if (m_log)
        *m_log << "Rollback transaction" << endl;
    driverEndTransaction(false);
}

//-----------------------------------------------------------------------------------------------

string CDatabaseConnection::queryError(const CQuery *) const
{
    notImplemented("queryError");
    return "";
}

void CDatabaseConnection::querySetAutoPrep(CQuery *q, bool pf)
{
    q->m_autoPrepare = pf;
}

void CDatabaseConnection::querySetStmt(CQuery *q, void *stmt)
{
    q->m_statement = stmt;
}

void CDatabaseConnection::querySetConn(CQuery *q, void *conn)
{
    q->m_connection = conn;
}

void CDatabaseConnection::querySetPrepared(CQuery *q, bool pf)
{
    q->m_prepared = pf;
}

void CDatabaseConnection::querySetActive(CQuery *q, bool af)
{
    q->m_active = af;
}

void CDatabaseConnection::querySetEof(CQuery *q, bool eof)
{
    q->m_eof = eof;
}

void CDatabaseConnection::queryAllocStmt(CQuery *)
{
    notImplemented("queryAllocStmt");
}

void CDatabaseConnection::queryFreeStmt(CQuery *)
{
    notImplemented("queryFreeStmt");
}

void CDatabaseConnection::queryCloseStmt(CQuery *)
{
    notImplemented("queryCloseStmt");
}

void CDatabaseConnection::queryPrepare(CQuery *)
{
    notImplemented("queryPrepare");
}

void CDatabaseConnection::queryUnprepare(CQuery *query)
{
    queryFreeStmt(query);
}

void CDatabaseConnection::queryExecute(CQuery *)
{
    notImplemented("queryExecute");
}

int CDatabaseConnection::queryColCount(CQuery *)
{
    notImplemented("queryColCount");
    return 0;
}

void CDatabaseConnection::queryColAttributes(CQuery *, int16_t, int16_t, int32_t&)
{
    notImplemented("queryColAttributes");
}

void CDatabaseConnection::queryColAttributes(CQuery *, int16_t, int16_t, char *, int32_t)
{
    notImplemented("queryColAttributes");
}

void CDatabaseConnection::queryBindParameters(CQuery *)
{
    notImplemented("queryBindParameters");
}

void CDatabaseConnection::queryOpen(CQuery *)
{
    notImplemented("queryOpen");
}

void CDatabaseConnection::queryFetch(CQuery *)
{
    notImplemented("queryFetch");
}

void CDatabaseConnection::notImplemented(const char *methodName) const
{
    throw CDatabaseException("Method '" + string(methodName) + "' is not supported by this database driver.");
}

void *CDatabaseConnection::queryHandle(CQuery *query) const
{
    return query->m_statement;
}

void CDatabaseConnection::queryHandle(CQuery *query, void *handle)
{
    query->m_statement = handle;
}

string CDatabaseConnection::paramMark(unsigned paramIndex)
{
    return "?";
}

void CDatabaseConnection::logAndThrow(string method, string error) THROWS_EXCEPTIONS
{
    string errorText("Exception in " + method + ": " + error);
    if (m_log)
        *m_log << "errorText" << endl;
    throw CDatabaseException(errorText);
}

void CDatabaseConnection::logFile(Logger *logFile)
{
    m_log = logFile;
}

/// @brief Returns a log file for the database operations.
/// @returns current log file ptr, ot NULL if log file isn't set
Logger* CDatabaseConnection::logFile()
{
    return m_log;
}

void CDatabaseConnection::driverBeginTransaction() THROWS_EXCEPTIONS
{
    notImplemented("driverBeginTransaction");
}

void CDatabaseConnection::driverEndTransaction(bool commit) THROWS_EXCEPTIONS
{
    notImplemented("driverEndTransaction");
}

void CDatabaseConnection::bulkInsert(std::string tableName, const CStrings& columnNames, const CStrings& data, std::string format) THROWS_EXCEPTIONS
{
    CQuery insertQuery(this,
                       "INSERT INTO " + tableName + "(" + columnNames.asString(",") + 
                       ") VALUES (:" + columnNames.asString(",:") + ")");
    for (CStrings::const_iterator row = data.begin(); row != data.end(); row++) {
        CStrings rowData(*row,"\t");
        for (unsigned i = 0; i < columnNames.size(); i++)
            insertQuery.param(i).setString(rowData[i]);
        insertQuery.exec();
    }
}

void CDatabaseConnection::executeBatchFile(std::string batchFile) THROWS_EXCEPTIONS
{
    throw CDatabaseException("Method executeBatchFile id not implemented for this database driver");
}
