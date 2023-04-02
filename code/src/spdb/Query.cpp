/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
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
#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

void QueryStatementManagement::setDatabase(PoolDatabaseConnection* db)
{
    m_db = db;
}

bool QueryStatementManagement::bulkMode() const
{
    return m_bulkMode;
}

void QueryStatementManagement::closeStmt(bool freeStatement)
{
    if (database() != nullptr && statement() != nullptr)
    {
        if (freeStatement)
        {
            database()->queryFreeStmt(dynamic_cast<Query*>(this));
            setPrepared(false);
        }
        else
        {
            database()->queryCloseStmt(dynamic_cast<Query*>(this));
        }
        setActive(false);
    }
}

void QueryStatementManagement::closeQuery(bool releaseStatement)
{
    setEof(true);
    if (statement() != nullptr)
    {
        closeStmt(releaseStatement);
    }
}

void QueryStatementManagement::notImplemented(const String& functionName) const
{
    throw DatabaseException(functionName + " isn't implemented", source_location::current(), getSQL());
}

void QueryStatementManagement::connect(PoolDatabaseConnection* _db)
{
    if (database() == _db)
    {
        return;
    }
    disconnect();
    setDatabase(_db);
    database()->linkQuery(dynamic_cast<Query*>(this));
}

void QueryStatementManagement::disconnect()
{
    closeQuery(true);
    if (database() != nullptr)
    {
        database()->unlinkQuery(dynamic_cast<Query*>(this));
    }
    setDatabase(nullptr);
}

void Query::execute()
{
    if (database() != nullptr && statement() != nullptr)
    {
        messages().clear();
        database()->queryExecute(this);
    }
}

//==============================================================================
Query::Query() noexcept
    : QueryStatementManagement(true)
    , m_fields(false)
{
}

Query::Query(const DatabaseConnection& db, const String& sql, bool autoPrepare)
    : QueryStatementManagement(autoPrepare)
    , m_fields(false)
{
    if (db)
    {
        setDatabase(db->connection());
        database()->linkQuery(this);
    }
    Query::sql(sql);
}

Query::Query(PoolDatabaseConnection* db, const String& sql, bool autoPrepare)
    : QueryStatementManagement(autoPrepare)
    , m_fields(false)
{
    if (db != nullptr)
    {
        setDatabase(db);
        database()->linkQuery(this);
    }
    Query::sql(sql);
}

Query::~Query()
{
    try
    {
        closeQuery(true);
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl);
    }
    if (database() != nullptr)
    {
        database()->unlinkQuery(this);
    }
}

bool skipToNextParameter(const char*& paramStart, const char*& paramEnd, String& sql)
{
    // Looking up for SQL parameters
    const char* delimiters = "':-/";

    // Find param start
    paramStart = strpbrk(paramEnd, delimiters);
    if (paramStart == nullptr)
    {
        return false;
    } // No more parameters

    bool rc = false;
    if (*paramStart == '\'')
    {
        // Started string constant
        const char* nextQuote = strchr(paramStart + 1, '\'');
        if (nextQuote == nullptr)
        {
            // Quote opened but never closed?
            paramEnd = nullptr;
        }
        else
        {
            sql += string(paramEnd, nextQuote - paramEnd + 1);
            paramEnd = nextQuote + 1;
        }
    }
    else if (*paramStart == '-' && paramStart[1] == '-')
    {
        // Started inline comment '--comment text', jump to the end of comment
        const char* endOfRow = strchr(paramStart + 1, '\n');
        if (endOfRow == nullptr)
        {
            // Comment at the end of last row
            paramEnd = nullptr;
        }
        else
        {
            sql += string(paramEnd, endOfRow - paramEnd + 1);
            paramEnd = endOfRow + 1;
        }
    }
    else if (*paramStart == '/' && paramStart[1] == '*')
    {
        // Started C-style block comment, jump to the end of comment
        const char* endOfRow = strstr(paramStart + 1, "*/");
        if (endOfRow == nullptr)
        {
            // Comment never closed
            paramEnd = nullptr;
        }
        else
        {
            sql += string(paramEnd, endOfRow - paramEnd + 2);
            paramEnd = endOfRow + 2;
        }
    }
    else if (*paramStart == '/' || paramStart[1] == ':' || paramStart[1] == '=')
    {
        // Started PostgreSQL type qualifier '::' or assignment ':='
        sql += string(paramEnd, paramStart - paramEnd + 2);
        paramEnd = paramStart + 2;
    }
    else
    {
        sql += string(paramEnd, paramStart - paramEnd);
        rc = true;
    }

    if (!rc)
    {
        return false;
    }

    paramEnd = paramStart + 1;
    if (*paramStart != ':')
    {
        sql += *paramStart;
        rc = false;
    }

    return rc;
}

void Query::sqlParseParameter(const char* paramStart, const char* paramEnd, int& paramNumber, String& sql)
{
    const string paramName(paramStart + 1, paramEnd - paramStart - 1);
    auto param = m_params.find(paramName.c_str());
    if (!param)
    {
        param = make_shared<QueryParameter>(paramName.c_str());
        m_params.add(param);
    }
    param->bindAdd(uint32_t(paramNumber));
    sql += database()->paramMark(uint32_t(paramNumber));
    ++paramNumber;
}

void Query::sql(const String& _sql)
{
    if (database() == nullptr)
    {
        throw DatabaseException("Query isn't connected to the database");
    }

    const String sql = parseParameters(_sql);

    for (int i = (int) m_params.size() - 1; i >= 0; --i)
    {
        if (m_params[i].bindCount() == 0)
        {
            m_params.remove(uint32_t(i));
        }
    }

    if (getSQL() != sql)
    {
        closeQuery(true);
        setSQL(sql);
        m_fields.clear();
    }
}

const char* Query::readParameter(String& sql, int& paramNumber, const char* paramStart, const char* paramEnd)
{
    for (;; ++paramEnd)
    {

        if (isalnum(*paramEnd) != 0)
        {
            continue;
        }

        if (*paramEnd == '_')
        {
            continue;
        }

        if (*paramEnd == '.')
        {
            // Oracle ':new.' or ':old.'
            sql += string(paramStart, paramEnd - paramStart + 1);
            ++paramEnd;
            return paramEnd;
        }

        sqlParseParameter(paramStart, paramEnd, paramNumber, sql);
        break;
    }
    return paramEnd;
}

String Query::parseParameters(const String& _sql)
{
    const char* paramStart {};
    const char* paramEnd = _sql.c_str();

    m_params.clear();
    String sql;

    int paramNumber = 0;
    for (;;)
    {
        if (!skipToNextParameter(paramStart, paramEnd, sql))
        {
            if (paramStart == nullptr || paramEnd == nullptr)
            {
                break;
            }
            continue;
        }

        paramEnd = readParameter(sql, paramNumber, paramStart, paramEnd);
    }

    if (paramEnd != nullptr)
    {
        sql += paramEnd;
    }
    return sql;
}

bool Query::open()
{
    if (database() == nullptr)
    {
        throw DatabaseException("Query is not connected to the database", source_location::current(), sql());
    }

    try
    {
        database()->queryOpen(this);
    }
    catch (const DatabaseException& e)
    {
        if (strstr(e.what(), "connection") == nullptr)
        {
            throw;
        }
        database()->close();
        database()->open();
        database()->queryOpen(this);
    }

    return true;
}

void Query::fetch()
{
    if (database() == nullptr || !active())
    {
        throw DatabaseException("Dataset isn't open", source_location::current(), sql());
    }

    if (eof())
    {
        throw DatabaseException("No more rows to read", source_location::current(), sql());
    }

    database()->queryFetch(this);
}

bool Query::readField(const char*, Variant&)
{
    return true;
}

bool Query::writeField(const char*, const Variant&)
{
    return true;
}

void Query::throwError(const String& method, const String& error)
{
    const String errorText("Exception in " + method + ": " + error);
    throw DatabaseException(errorText);
}

void QueryStatementManagement::setBulkMode(bool bulkMode)
{
    m_bulkMode = bulkMode;
}
