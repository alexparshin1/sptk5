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

#include <format>
#include <sptk5/cutils>
#include <sptk5/db/OracleOciConnection.h>
#include <sptk5/db/OracleOciStatement.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;
using namespace ocilib;

OracleOciConnection::OracleOciConnection(const String& connectionString, chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::ORACLE_OCI, connectTimeout)
{
    static mutex initializeMutex;
    scoped_lock lock(initializeMutex);
    if (!ocilib::Environment::Initialized())
    {
        ocilib::Environment::Initialize();
    }
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

Connection* OracleOciConnection::connection() const
{
    return m_connection.get();
}

void OracleOciConnection::_openDatabase(const String& newConnectionString)
{
    if (!active())
    {
        setInTransaction(false);
        if (!newConnectionString.empty())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        //Statement* createLobTable = nullptr;
        try
        {
            DatabaseConnectionString dbConnectionString = connectionString();
            auto oracleService = format("{}:{}/{}", dbConnectionString.hostName().c_str(),
                                        dbConnectionString.portNumber(), dbConnectionString.databaseName().c_str());
            m_connection = make_shared<ocilib::Connection>(oracleService, dbConnectionString.userName(), dbConnectionString.password());
            m_connection->SetAutoCommit(true);
        }
        catch (const ocilib::Exception& e)
        {
            if (strstr(e.what(), "already used") == nullptr)
            {
                if (m_connection)
                {
                    //m_connection->terminateStatement(createLobTable);
                    m_connection.reset();
                }
                throw DatabaseException(string("Can't create connection: ") + e.what());
            }
        }
    }
}

void OracleOciConnection::closeDatabase()
{
    try
    {
        m_connection.reset();
    }
    catch (const ocilib::Exception& e)
    {
        throw DatabaseException(string("Can't close connection: ") + e.what());
    }
}

void OracleOciConnection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    PoolDatabaseConnection::executeBatchSQL(batchSQL, errors);
}

bool OracleOciConnection::active() const
{
    return (bool) m_connection;
}

DBHandle OracleOciConnection::handle() const
{
    return PoolDatabaseConnection::handle();
}

String OracleOciConnection::driverDescription() const
{
    auto driverVersion = format("OracleOci {}.{}", Environment::GetCompileMajorVersion(), Environment::GetCompileMinorVersion());
    return driverVersion;
}

void OracleOciConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
}

void OracleOciConnection::driverBeginTransaction()
{
    m_connection->SetAutoCommit(false);
    PoolDatabaseConnection::driverBeginTransaction();
}

void OracleOciConnection::driverEndTransaction(bool commit)
{
    if (commit)
    {
        m_connection->Commit();
    }
    else
    {
        m_connection->Rollback();
    }
    m_connection->SetAutoCommit(true);
}

void OracleOciConnection::queryAllocStmt(Query* query)
{
    auto stmt = make_shared<OracleOciStatement>(this, query->sql());
    querySetStmt(query, reinterpret_pointer_cast<uint8_t>(stmt));
}

void OracleOciConnection::queryFreeStmt(Query* query)
{
    const scoped_lock lock(m_mutex);
    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
}

void OracleOciConnection::queryCloseStmt(Query* query)
{
    const scoped_lock lock(m_mutex);
    auto* statement = bit_cast<OracleOciStatement*>(query->statement());
    if (statement)
    {
        statement->close();
    }
}

void OracleOciConnection::queryPrepare(Query* query)
{
    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<OracleOciStatement*>(query->statement());
    statement->enumerateParams(query->params());
    /*
    if (query->bulkMode())
    {
        const CParamVector& enumeratedParams = statement->enumeratedParams();
        Statement* stmt = statement->stmt();
        const auto* bulkInsertQuery = dynamic_cast<OracleBulkInsertQuery*>(query);
        if (bulkInsertQuery == nullptr)
        {
            throw Exception("Not a bulk query");
        }
        const QueryColumnTypeSizeMap& columnTypeSizes = bulkInsertQuery->columnTypeSizes();
        setMaxParamSizes(enumeratedParams, stmt, columnTypeSizes);
        stmt->setMaxIterations((unsigned) bulkInsertQuery->batchSize());
    }
     */
    querySetPrepared(query, true);
}

void OracleOciConnection::queryExecute(Query* query)
{
    try
    {
        auto* statement = bit_cast<OracleOciStatement*>(query->statement());
        if (statement == nullptr)
        {
            throw Exception("Query is not prepared");
        }
        /*
        if (query->bulkMode())
        {
            const auto* bulkInsertQuery = dynamic_cast<OracleBulkInsertQuery*>(query);
            if (bulkInsertQuery == nullptr)
            {
                throw Exception("Query is not COracleBulkInsertQuery");
            }
            statement->execBulk(getInTransaction(), bulkInsertQuery->lastIteration());
        }
        else
         */
        {
            statement->execute(getInTransaction());
        }
    }
    catch (const ocilib::Exception& e)
    {
        throw DatabaseException(e.what());
    }
}

int OracleOciConnection::queryColCount(Query* query)
{
    const auto* statement = bit_cast<OracleOciStatement*>(query->statement());
    if (statement == nullptr)
    {
        throw DatabaseException("Query not opened");
    }
    const auto resultSet = statement->statement()->GetResultset();
    return resultSet.GetColumnCount();
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

String OracleOciConnection::queryError(const Query* query) const
{
    return String();
}
