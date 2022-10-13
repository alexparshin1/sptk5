/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

PoolDatabaseConnection::PoolDatabaseConnection(const String& _connectionString, DatabaseConnectionType connectionType, chrono::seconds connectTimeout)
    : m_connType(connectionType)
    , m_connectionTimeout(connectTimeout)
{
    connectionString(DatabaseConnectionString(_connectionString));
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
    for (const auto& [query, statement]: m_queryList)
    {
        try
        {
            query->closeQuery(true);
        }
        catch (const Exception& e)
        {
            CERR(e.what() << endl)
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

bool PoolDatabaseConnectionQueryMethods::linkQuery(Query* q)
{
    if (auto itor = m_queryList.find(q);
        itor == m_queryList.end())
    {
        m_queryList[q] = nullptr;
    }
    return true;
}

bool PoolDatabaseConnectionQueryMethods::unlinkQuery(Query* q)
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

String sptk::escapeSQLString(const String& str, bool tsv)
{
    String output;
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

// Note: end row is not included
void PoolDatabaseConnection::bulkInsertRecords(
    const String& tableName, const Strings& columnNames,
    const vector<VariantVector>::const_iterator& begin, const vector<VariantVector>::const_iterator& end)
{
    stringstream sql;
    sql << "INSERT INTO " << tableName << " (" << columnNames.join(",") << ") VALUES ";
    for (auto itor = begin; itor != end; ++itor)
    {
        const VariantVector& row = *itor;
        if (itor != begin)
        {
            sql << ", ";
        }
        sql << "(";
        bool firstValue = true;
        for (const auto& value: row)
        {
            if (firstValue)
            {
                firstValue = false;
            }
            else
            {
                sql << ", ";
            }
            if (value.isNull())
            {
                sql << "NULL";
                continue;
            }
            switch (value.dataType())
            {
                case VariantDataType::VAR_BOOL:
                    sql << "true";
                    break;
                case VariantDataType::VAR_STRING:
                case VariantDataType::VAR_TEXT:
                    sql << "'" << escapeSQLString(value.asString(), false) << "'";
                    break;
                case VariantDataType::VAR_DATE_TIME:
                    sql << "'" << value.asDateTime().dateString(DateTime::PF_RFC_DATE) << " "
                        << value.asDateTime().timeString(0, DateTime::PrintAccuracy::MILLISECONDS) << "'";
                    break;
                default:
                    sql << "'" << value.asString() << "'";
                    break;
            }
        }
        sql << ")";
    }
    sql << ";" << endl;
    Query insertRows(this, sql.str(), false);
    insertRows.exec();
}

void PoolDatabaseConnection::_bulkInsert(const String& tableName, const Strings& columnNames,
                                         const vector<VariantVector>& data)
{
    const auto recordsInBatch = 16;
    auto begin = data.begin();
    auto end = data.begin();
    for (; end != data.end(); ++end)
    {
        if (end - begin > recordsInBatch)
        {
            bulkInsertRecords(tableName, columnNames, begin, end);
            begin = end;
        }
    }

    if (begin != end)
    {
        bulkInsertRecords(tableName, columnNames, begin, end);
    }
}

void PoolDatabaseConnection::_executeBatchFile(const String& batchFileName, Strings* errors)
{
    Strings batchFileContent;
    batchFileContent.loadFromFile(batchFileName.c_str());
    _executeBatchSQL(batchFileContent, errors);
}

void PoolDatabaseConnection::_executeBatchSQL(const Strings& /*batchFile*/, Strings* /*errors*/)
{
    throw DatabaseException("Method executeBatchFile id not implemented for this database driver");
}

void PoolDatabaseConnectionQueryMethods::querySetStmt(Query* q, SStmtHandle stmt)
{
    m_queryList[q] = stmt;
    q->setStatement(stmt.get());
}

void PoolDatabaseConnectionQueryMethods::querySetPrepared(Query* q, bool pf)
{
    q->setPrepared(pf);
}

void PoolDatabaseConnectionQueryMethods::querySetActive(Query* q, bool af)
{
    q->setActive(af);
}

void PoolDatabaseConnectionQueryMethods::querySetEof(Query* q, bool eof)
{
    q->setEof(eof);
}

void PoolDatabaseConnectionQueryMethods::notImplemented(const String& methodName) const
{
    throw DatabaseException("Method '" + methodName + "' is not supported by this database driver.");
}

String PoolDatabaseConnectionQueryMethods::paramMark(unsigned /*paramIndex*/)
{
    return String("?");
}

