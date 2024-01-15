/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/db/OracleOciConnection.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

OracleOciConnection::OracleOciConnection(const String& connectionString, chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::ORACLE_OCI, connectTimeout)
{
}

OracleOciConnection::~OracleOciConnection()
{
    try
    {
        if (getInTransaction() && OracleOciConnection::active())
        {
            rollbackTransaction();
        }
        disconnectAllQueries();
        close();
    }
    catch (const sptk::Exception& e)
    {
        CERR(e.what() << endl);
    }
    catch (const ocilib::Exception& e)
    {
        CERR(e.what() << endl);
    }
}

OCI_Connection* OracleOciConnection::connection() const
{
    return nullptr;
}

void OracleOciConnection::_openDatabase(const String& connectionString)
{
    PoolDatabaseConnection::_openDatabase(connectionString);
}

void OracleOciConnection::closeDatabase()
{
    PoolDatabaseConnection::closeDatabase();
}

void OracleOciConnection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    PoolDatabaseConnection::executeBatchSQL(batchSQL, errors);
}

bool OracleOciConnection::active() const
{
    return PoolDatabaseConnection::active();
}

DBHandle OracleOciConnection::handle() const
{
    return PoolDatabaseConnection::handle();
}

String OracleOciConnection::driverDescription() const
{
    return PoolDatabaseConnection::driverDescription();
}

void OracleOciConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
}

void OracleOciConnection::driverBeginTransaction()
{
    PoolDatabaseConnection::driverBeginTransaction();
}

void OracleOciConnection::driverEndTransaction(bool commit)
{
    PoolDatabaseConnection::driverEndTransaction(commit);
}

void OracleOciConnection::queryAllocStmt(Query* query)
{
}

void OracleOciConnection::queryFreeStmt(Query* query)
{
}

void OracleOciConnection::queryCloseStmt(Query* query)
{
}

void OracleOciConnection::queryPrepare(Query* query)
{
}

void OracleOciConnection::queryExecute(Query* query)
{
}

int OracleOciConnection::queryColCount(Query* query)
{
    return 0;
}

void OracleOciConnection::queryBindParameters(Query* query)
{
}

void OracleOciConnection::queryOpen(Query* query)
{
}

void OracleOciConnection::queryFetch(Query* query)
{
}

void OracleOciConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value)
{
}

void OracleOciConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, char* buff, int len)
{
}

String OracleOciConnection::paramMark(unsigned int paramIndex)
{
    return PoolDatabaseConnectionQueryMethods::paramMark(paramIndex);
}
