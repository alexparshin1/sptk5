/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       PoolDatabaseConnection.cpp - description               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday October 28 2018                                 ║
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

#include <sptk5/cutils>
#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

PoolDatabaseConnection::PoolDatabaseConnection(const String& connectionString, DatabaseConnectionType connectionType)
: m_connString(connectionString), m_connType(connectionType)
{
    m_inTransaction = false;
}

PoolDatabaseConnection::~PoolDatabaseConnection()
{
    disconnectAllQueries();
}

void PoolDatabaseConnection::disconnectAllQueries()
{
    for (auto* query: m_queryList) {
        try {
            query->closeQuery(true);
        }
        catch (const Exception& e) {
            CERR(e.what() << endl);
        }
    }
    m_queryList.clear();
}

bool PoolDatabaseConnection::getInTransaction() const
{
    return m_inTransaction;
}

void PoolDatabaseConnection::setInTransaction(bool inTransaction)
{
    m_inTransaction = inTransaction;
}

bool PoolDatabaseConnection::linkQuery(Query *q)
{
    m_queryList.insert(q);
    return true;
}

bool PoolDatabaseConnection::unlinkQuery(Query *q)
{
    m_queryList.erase(q);
    return true;
}

void PoolDatabaseConnection::_openDatabase(const String&)
{
    notImplemented("openDatabase");
}

void PoolDatabaseConnection::open(const String& newConnectionString)
{
    _openDatabase(newConnectionString);
}

void PoolDatabaseConnection::closeDatabase()
{
    // Implemented in derived classes
}

void PoolDatabaseConnection::close()
{
    if (active()) {
        m_inTransaction = false;
        disconnectAllQueries();
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

void PoolDatabaseConnection::_bulkInsert(
        const String& tableName, const Strings& columnNames, const Strings& data, const String& /*format*/)
{
    Query insertQuery(this,
                      "INSERT INTO " + tableName + "(" + columnNames.join(",") +
                      ") VALUES (:" + columnNames.join(",:") + ")");
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

void PoolDatabaseConnection_QueryMethods::querySetStmt(Query *q, void *stmt)
{
    q->setStatement(stmt);
}

void PoolDatabaseConnection_QueryMethods::querySetPrepared(Query *q, bool pf)
{
    q->setPrepared(pf);
}

void PoolDatabaseConnection_QueryMethods::querySetActive(Query *q, bool af)
{
    q->setActive(af);
}

void PoolDatabaseConnection_QueryMethods::querySetEof(Query *q, bool eof)
{
    q->setEof(eof);
}

String PoolDatabaseConnection_QueryMethods::queryError(const Query*) const
{
    notImplemented("queryError");
    return String();
}

void PoolDatabaseConnection_QueryMethods::queryAllocStmt(Query*)
{
    notImplemented("queryAllocStmt");
}

void PoolDatabaseConnection_QueryMethods::queryFreeStmt(Query*)
{
    notImplemented("queryFreeStmt");
}

void PoolDatabaseConnection_QueryMethods::queryCloseStmt(Query*)
{
    notImplemented("queryCloseStmt");
}

void PoolDatabaseConnection_QueryMethods::queryPrepare(Query*)
{
    notImplemented("queryPrepare");
}

void PoolDatabaseConnection_QueryMethods::queryUnprepare(Query *query)
{
    queryFreeStmt(query);
}

void PoolDatabaseConnection_QueryMethods::queryExecute(Query*)
{
    notImplemented("queryExecute");
}

void PoolDatabaseConnection_QueryMethods::queryExecDirect(Query*)
{
    notImplemented("queryExecDirect");
}

int PoolDatabaseConnection_QueryMethods::queryColCount(Query*)
{
    notImplemented("queryColCount");
    return 0;
}

void PoolDatabaseConnection_QueryMethods::queryColAttributes(Query*, int16_t, int16_t, int32_t&)
{
    notImplemented("queryColAttributes");
}

void PoolDatabaseConnection_QueryMethods::queryColAttributes(Query*, int16_t, int16_t, char *, int32_t)
{
    notImplemented("queryColAttributes");
}

void PoolDatabaseConnection_QueryMethods::queryBindParameters(Query*)
{
    notImplemented("queryBindParameters");
}

void PoolDatabaseConnection_QueryMethods::queryOpen(Query*)
{
    notImplemented("queryOpen");
}

void PoolDatabaseConnection_QueryMethods::queryFetch(Query*)
{
    notImplemented("queryFetch");
}

void PoolDatabaseConnection_QueryMethods::notImplemented(const String& methodName) const
{
    throw DatabaseException("Method '" + methodName + "' is not supported by this database driver.");
}

String PoolDatabaseConnection_QueryMethods::paramMark(unsigned /*paramIndex*/)
{
    return String("?");
}

