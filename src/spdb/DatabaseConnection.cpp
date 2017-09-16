/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DatabaseConnection.cpp - description                  ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/db/DatabaseConnection.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

DatabaseConnection::DatabaseConnection(const string& connectionString)
:   m_connString(connectionString), m_connType(DCT_UNKNOWN)
{
    m_inTransaction = false;
    m_log = nullptr;
}

DatabaseConnection::~DatabaseConnection()
{
    // To prevent the exceptions, if the database connection
    // is terminated already
    try {
        while (!m_queryList.empty()) {
            auto query = (Query *) m_queryList[0];
            query->disconnect();
        }
    }
    catch (...) {
    }
}

bool DatabaseConnection::linkQuery(Query *q)
{
    m_queryList.push_back(q);
    return true;
}

bool DatabaseConnection::unlinkQuery(Query *q)
{
    auto itor = find(m_queryList.begin(), m_queryList.end(), q);
    m_queryList.erase(itor);
    return true;
}

void DatabaseConnection::openDatabase(const string& newConnectionString)
{
    notImplemented("openDatabase");
}

void DatabaseConnection::open(string newConnectionString)
{
    openDatabase(newConnectionString);
    if (m_log != nullptr)
        *m_log << "Opened database: " << m_connString.str() << endl;
}

void DatabaseConnection::closeDatabase()
{
    //closeDatabase();
}

void DatabaseConnection::close()
{
    if (active()) {
        if (m_inTransaction) {
            rollbackTransaction();
            m_inTransaction = false;
        }

        for (auto query: m_queryList)
            query->closeQuery(true);

        closeDatabase();
        if (m_log != nullptr)
            *m_log << "Closed database: " << m_connString.str() << endl;
    }
}

void* DatabaseConnection::handle() const
{
    notImplemented("handle");
    return nullptr;
}

bool DatabaseConnection::active() const
{
    notImplemented("active");
    return true;
}

void DatabaseConnection::beginTransaction()
{
    if (m_log != nullptr)
        *m_log << "Begin transaction" << endl;
    driverBeginTransaction();
}

void DatabaseConnection::commitTransaction()
{
    if (m_log != nullptr)
        *m_log << "Commit transaction" << endl;
    driverEndTransaction(true);
}

void DatabaseConnection::rollbackTransaction()
{
    if (m_log != nullptr)
        *m_log << "Rollback transaction" << endl;
    driverEndTransaction(false);
}

//-----------------------------------------------------------------------------------------------

string DatabaseConnection::queryError(const Query *q) const
{
    notImplemented("queryError");
    return "";
}

void DatabaseConnection::querySetAutoPrep(Query *q, bool pf)
{
    q->m_autoPrepare = pf;
}

void DatabaseConnection::querySetStmt(Query *q, void *stmt)
{
    q->m_statement = stmt;
}

void DatabaseConnection::querySetConn(Query *q, void *conn)
{
    q->m_connection = conn;
}

void DatabaseConnection::querySetPrepared(Query *q, bool pf)
{
    q->m_prepared = pf;
}

void DatabaseConnection::querySetActive(Query *q, bool af)
{
    q->m_active = af;
}

void DatabaseConnection::querySetEof(Query *q, bool eof)
{
    q->m_eof = eof;
}

void DatabaseConnection::queryAllocStmt(Query *query)
{
    notImplemented("queryAllocStmt");
}

void DatabaseConnection::queryFreeStmt(Query *query)
{
    notImplemented("queryFreeStmt");
}

void DatabaseConnection::queryCloseStmt(Query *query)
{
    notImplemented("queryCloseStmt");
}

void DatabaseConnection::queryPrepare(Query *query)
{
    notImplemented("queryPrepare");
}

void DatabaseConnection::queryUnprepare(Query *query)
{
    queryFreeStmt(query);
}

void DatabaseConnection::queryExecute(Query *query)
{
    notImplemented("queryExecute");
}

int DatabaseConnection::queryColCount(Query *query)
{
    notImplemented("queryColCount");
    return 0;
}

void DatabaseConnection::queryColAttributes(Query *query, int16_t, int16_t, int32_t&)
{
    notImplemented("queryColAttributes");
}

void DatabaseConnection::queryColAttributes(Query *query, int16_t, int16_t, char *, int32_t)
{
    notImplemented("queryColAttributes");
}

void DatabaseConnection::queryBindParameters(Query *query)
{
    notImplemented("queryBindParameters");
}

void DatabaseConnection::queryOpen(Query *query)
{
    notImplemented("queryOpen");
}

void DatabaseConnection::queryFetch(Query *query)
{
    notImplemented("queryFetch");
}

void DatabaseConnection::notImplemented(const char *methodName) const
{
    throw DatabaseException("Method '" + string(methodName) + "' is not supported by this database driver.");
}

void *DatabaseConnection::queryHandle(Query *query) const
{
    return query->m_statement;
}

void DatabaseConnection::queryHandle(Query *query, void *handle)
{
    query->m_statement = handle;
}

string DatabaseConnection::paramMark(unsigned /*paramIndex*/)
{
    return "?";
}

void DatabaseConnection::logAndThrow(string method, string error)
{
    string errorText("Exception in " + method + ": " + error);
    if (m_log != nullptr)
        *m_log << "errorText" << endl;
    throw DatabaseException(errorText);
}

void DatabaseConnection::logFile(Logger *logFile)
{
    m_log = logFile;
}

/// @brief Returns a log file for the database operations.
/// @returns current log file ptr, ot NULL if log file isn't set
Logger* DatabaseConnection::logFile()
{
    return m_log;
}

void DatabaseConnection::driverBeginTransaction()
{
    notImplemented("driverBeginTransaction");
}

void DatabaseConnection::driverEndTransaction(bool /*commit*/)
{
    notImplemented("driverEndTransaction");
}

void DatabaseConnection::bulkInsert(const String& tableName, const Strings& columnNames, const Strings& data, const String& format)
{
    Query insertQuery(this,
                      "INSERT INTO " + tableName + "(" + columnNames.asString(",") +
                      ") VALUES (:" + columnNames.asString(",:") + ")");
    for (auto& row: data) {
        Strings rowData(row,"\t");
        for (unsigned i = 0; i < columnNames.size(); i++)
            insertQuery.param(i).setString(rowData[i]);
        insertQuery.exec();
    }
}

void DatabaseConnection::executeBatchFile(const String& batchFileName, Strings* errors)
{
    Strings batchFileContent;
    batchFileContent.loadFromFile(batchFileName);
    executeBatchSQL(batchFileContent, errors);
}

void DatabaseConnection::executeBatchSQL(const Strings& /*batchFile*/, Strings* /*errors*/)
{
    throw DatabaseException("Method executeBatchFile id not implemented for this database driver");
}
