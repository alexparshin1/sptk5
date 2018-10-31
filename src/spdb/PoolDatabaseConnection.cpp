/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       PoolDatabaseConnection.cpp - description               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday October 28 2018                                 ║
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

#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/db/Query.h>
#include "../../sptk5/String.h"

using namespace std;
using namespace sptk;

PoolDatabaseConnection::PoolDatabaseConnection(const String& connectionString)
: m_connString(connectionString), m_connType(DCT_UNKNOWN)
{
    m_inTransaction = false;
}

PoolDatabaseConnection::~PoolDatabaseConnection()
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

bool PoolDatabaseConnection::linkQuery(Query *q)
{
    m_queryList.push_back(q);
    return true;
}

bool PoolDatabaseConnection::unlinkQuery(Query *q)
{
    auto itor = find(m_queryList.begin(), m_queryList.end(), q);
    m_queryList.erase(itor);
    return true;
}

void PoolDatabaseConnection::_openDatabase(const String& newConnectionString)
{
    notImplemented("openDatabase");
}

void PoolDatabaseConnection::open(const String& newConnectionString)
{
    _openDatabase(newConnectionString);
}

void PoolDatabaseConnection::closeDatabase()
{
    //closeDatabase();
}

void PoolDatabaseConnection::close()
{
    if (active()) {
        if (m_inTransaction) {
            rollbackTransaction();
            m_inTransaction = false;
        }

        for (auto query: m_queryList)
            query->closeQuery(true);

        closeDatabase();
    }
}

void* PoolDatabaseConnection::handle() const
{
    notImplemented("handle");
    return nullptr;
}

bool PoolDatabaseConnection::active() const
{
    notImplemented("active");
    return true;
}

void PoolDatabaseConnection::beginTransaction()
{
    driverBeginTransaction();
}

void PoolDatabaseConnection::commitTransaction()
{
    driverEndTransaction(true);
}

void PoolDatabaseConnection::rollbackTransaction()
{
    driverEndTransaction(false);
}

//-----------------------------------------------------------------------------------------------

String PoolDatabaseConnection::queryError(const Query*) const
{
    notImplemented("queryError");
    return String();
}

void PoolDatabaseConnection::querySetAutoPrep(Query *q, bool pf)
{
    q->m_autoPrepare = pf;
}

void PoolDatabaseConnection::querySetStmt(Query *q, void *stmt)
{
    q->m_statement = stmt;
}

void PoolDatabaseConnection::querySetConn(Query *q, void *conn)
{
    q->m_connection = conn;
}

void PoolDatabaseConnection::querySetPrepared(Query *q, bool pf)
{
    q->m_prepared = pf;
}

void PoolDatabaseConnection::querySetActive(Query *q, bool af)
{
    q->m_active = af;
}

void PoolDatabaseConnection::querySetEof(Query *q, bool eof)
{
    q->m_eof = eof;
}

void PoolDatabaseConnection::queryAllocStmt(Query*)
{
    notImplemented("queryAllocStmt");
}

void PoolDatabaseConnection::queryFreeStmt(Query*)
{
    notImplemented("queryFreeStmt");
}

void PoolDatabaseConnection::queryCloseStmt(Query*)
{
    notImplemented("queryCloseStmt");
}

void PoolDatabaseConnection::queryPrepare(Query*)
{
    notImplemented("queryPrepare");
}

void PoolDatabaseConnection::queryUnprepare(Query *query)
{
    queryFreeStmt(query);
}

void PoolDatabaseConnection::queryExecute(Query*)
{
    notImplemented("queryExecute");
}

int PoolDatabaseConnection::queryColCount(Query*)
{
    notImplemented("queryColCount");
    return 0;
}

void PoolDatabaseConnection::queryColAttributes(Query*, int16_t, int16_t, int32_t&)
{
    notImplemented("queryColAttributes");
}

void PoolDatabaseConnection::queryColAttributes(Query*, int16_t, int16_t, char *, int32_t)
{
    notImplemented("queryColAttributes");
}

void PoolDatabaseConnection::queryBindParameters(Query*)
{
    notImplemented("queryBindParameters");
}

void PoolDatabaseConnection::queryOpen(Query*)
{
    notImplemented("queryOpen");
}

void PoolDatabaseConnection::queryFetch(Query*)
{
    notImplemented("queryFetch");
}

void PoolDatabaseConnection::notImplemented(const String& methodName) const
{
    throw DatabaseException("Method '" + methodName + "' is not supported by this database driver.");
}

void *PoolDatabaseConnection::queryHandle(Query *query) const
{
    return query->m_statement;
}

void PoolDatabaseConnection::queryHandle(Query *query, void *handle)
{
    query->m_statement = handle;
}

String PoolDatabaseConnection::paramMark(unsigned /*paramIndex*/)
{
    return String("?");
}

void PoolDatabaseConnection::logAndThrow(const String& method, const String& error)
{
    String errorText("Exception in " + method + ": " + error);
    throw DatabaseException(errorText);
}

void PoolDatabaseConnection::driverBeginTransaction()
{
    notImplemented("driverBeginTransaction");
}

void PoolDatabaseConnection::driverEndTransaction(bool /*commit*/)
{
    notImplemented("driverEndTransaction");
}

void PoolDatabaseConnection::_bulkInsert(const String& tableName, const Strings& columnNames, const Strings& data,
                                     const String& format)
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

void PoolDatabaseConnection::_executeBatchFile(const String& batchFileName, Strings* errors)
{
    Strings batchFileContent;
    batchFileContent.loadFromFile(batchFileName);
    _executeBatchSQL(batchFileContent, errors);
}

void PoolDatabaseConnection::_executeBatchSQL(const Strings& /*batchFile*/, Strings* /*errors*/)
{
    throw DatabaseException("Method executeBatchFile id not implemented for this database driver");
}
