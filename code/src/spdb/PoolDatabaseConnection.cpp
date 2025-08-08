/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/db/PoolDatabaseConnection.h"
#include "sptk5/db/BulkQuery.h"

#include <ranges>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

PoolDatabaseConnection::PoolDatabaseConnection(const String& connectionString, DatabaseConnectionType connectionType, chrono::seconds connectTimeout)
    : m_connType(connectionType)
    , m_connectionTimeout(connectTimeout)
{
    this->connectionString(DatabaseConnectionString(connectionString));
}

PoolDatabaseConnection::~PoolDatabaseConnection()
{
    disconnectAllQueries();
}

void PoolDatabaseConnection::connectionString(const DatabaseConnectionString& connectionString)
{
    m_connString = DatabaseConnectionString(connectionString);
}

void PoolDatabaseConnectionQueryMethods::disconnectAllQueries()
{
    for (const auto& query: views::keys(m_queryList))
    {
        try
        {
            query->closeQuery(true);
        }
        catch (const Exception& e)
        {
            CERR(e.what());
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

bool PoolDatabaseConnectionQueryMethods::linkQuery(Query* query)
{
    if (const auto itor = m_queryList.find(query);
        itor == m_queryList.end())
    {
        m_queryList[query] = nullptr;
    }
    return true;
}

bool PoolDatabaseConnectionQueryMethods::unlinkQuery(Query* query)
{
    m_queryList.erase(query);
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
    if (active())
    {
        m_inTransaction = false;
        disconnectAllQueries();
        closeDatabase();
    }
}

DBHandle PoolDatabaseConnection::handle() const
{
    notImplemented("handle");
}

bool PoolDatabaseConnection::active() const
{
    notImplemented("active");
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
    const String errorText("Exception in " + method + ": " + error);
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

String sptk::escapeSQLString(const String& str, bool tsv)
{
    String      output;
    const char* replaceChars = "'\t\n\r";
    if (tsv)
    {
        replaceChars = "\t\n\r";
    }
    const char* start = str.c_str();
    while (*start)
    {
        const char* end = strpbrk(start, replaceChars);
        if (end != nullptr)
        {
            output.append(start, end - start);
            switch (*end)
            {
                case '\'':
                    output += "''";
                    break;
                case '\t':
                    output += "\\t";
                    break;
                case '\r':
                    output += "\\r";
                    break;
                case '\n':
                    output += "\\n";
                    break;
                default:
                    break;
            }
            start = end + 1;
            if (*start == 0)
            {
                continue;
            }
        }
        else
        {
            output.append(start);
            break;
        }
    }
    return output;
}

void PoolDatabaseConnection::bulkInsert(const String& tableName, const String& autoIncrementColumnName, const Strings& columnNames,
                                        std::vector<VariantVector>& data, size_t groupSize, vector<int64_t>* insertedIds)
{
    const bool wasInTransaction = inTransaction();
    if (!wasInTransaction && connectionType() != DatabaseConnectionType::SQLITE3)
    {
        beginTransaction();
    }

    Strings columnNamesFinal = columnNames;
    if (insertedIds && !autoIncrementColumnName.empty() && (connectionType() == DatabaseConnectionType::ORACLE || connectionType() == DatabaseConnectionType::ORACLE_OCI))
    {
        if (int autoIncrementColumnIndex = columnNames.indexOf(autoIncrementColumnName);
            autoIncrementColumnIndex >= 0)
        {
            throw DatabaseException("The auto increment column can't be used in the column list");
        }
        columnNamesFinal.push_back(autoIncrementColumnName);
    }

    BulkQuery bulkQuery(this, tableName, autoIncrementColumnName, columnNamesFinal, groupSize);
    bulkQuery.insertRows(data, insertedIds);

    if (!wasInTransaction && connectionType() != DatabaseConnectionType::SQLITE3)
    {
        commitTransaction();
    }
}

void PoolDatabaseConnection::bulkDelete(const String& tableName, const String& keyColumnName, const VariantVector& keys)
{
    const bool wasInTransaction = inTransaction();
    if (!wasInTransaction)
    {
        beginTransaction();
    }

    BulkQuery bulkQuery(this, tableName, keyColumnName, {keyColumnName}, 50);
    bulkQuery.deleteRows(keys);

    if (!wasInTransaction)
    {
        commitTransaction();
    }
}

void PoolDatabaseConnection::executeBatchFile(const String& batchFileName, Strings* errors)
{
    Strings batchFileContent;
    batchFileContent.loadFromFile(batchFileName.c_str());
    executeBatchSQL(batchFileContent, errors);
}

void PoolDatabaseConnection::executeBatchSQL(const Strings& /*batchFile*/, Strings* /*errors*/)
{
    throw DatabaseException("Method executeBatchFile id not implemented for this database driver");
}

String PoolDatabaseConnection::tableSequenceName(const String& tableName)
{
    return tableName + "_seq";
}

String PoolDatabaseConnection::lastAutoIncrementSql(const String& tableName)
{
    using enum DatabaseConnectionType;
    switch (connectionType())
    {
        case MYSQL:
            return "SELECT LAST_INSERT_ID()";
        case MSSQL_ODBC:
            return "SELECT SCOPE_IDENTITY()";
        case POSTGRES:
            return "SELECT LASTVAL()";
        case ORACLE:
        case ORACLE_OCI:
            return ""; // Use Oracle-specific method override.
        case FIREBIRD:
            return "SELECT GEN_ID(" + tableName + ", 0) FROM RDB$DATABASE"; // Not tested.
        case SQLITE3:
            return "SELECT last_insert_rowid()";
        default:
            break;
    }
    return {};
}

void PoolDatabaseConnectionQueryMethods::querySetStmt(Query* query, const SStmtHandle& stmt)
{
    m_queryList[query] = stmt;
    query->setStatement(stmt.get());
}

void PoolDatabaseConnectionQueryMethods::querySetPrepared(Query* query, bool isPrepared)
{
    query->setPrepared(isPrepared);
}

void PoolDatabaseConnectionQueryMethods::querySetActive(Query* query, bool isActive)
{
    query->setActive(isActive);
}

void PoolDatabaseConnectionQueryMethods::querySetEof(Query* query, bool isEof)
{
    query->setEof(isEof);
}

void PoolDatabaseConnectionQueryMethods::notImplemented(const String& methodName)
{
    throw DatabaseException("Method '" + methodName + "' is not supported by this database driver.");
}

String PoolDatabaseConnectionQueryMethods::paramMark(unsigned /*paramIndex*/)
{
    return {"?"};
}
