/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-06                                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <algorithm>
#include <sptk5/cutils>
#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/db/Query.h>
#include <sstream> // Included directly, not for this file alone: libstdc++ happens to
                   // reach <sstream> through other standard headers and libc++ does not,
                   // so without it this translation unit fails to compile on FreeBSD and
                   // anywhere else libc++ is the standard library. Do not remove it as
                   // redundant - it is only redundant on one implementation.

using namespace std;
using namespace sptk;

void QueryStatementManagement::setDatabase(const WPoolDatabaseConnection& db)
{
    m_db = db;
}

bool QueryStatementManagement::bulkMode() const
{
    return m_bulkMode;
}

void QueryStatementManagement::closeStmt(const bool freeStatement)
{
    const auto db = database().lock();
    if (!db)
    {
        throw Exception("Database connection is not valid");
    }
    if (db != nullptr && statement() != nullptr)
    {
        if (freeStatement)
        {
            db->queryFreeStmt(dynamic_cast<Query*>(this));
            setPrepared(false);
        }
        else
        {
            db->queryCloseStmt(dynamic_cast<Query*>(this));
        }
        setActive(false);
    }
}

void QueryStatementManagement::closeQuery(const bool releaseStatement)
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

void QueryStatementManagement::connect(const WPoolDatabaseConnection& newDb)
{
    const auto db = newDb.lock();
    if (!db)
    {
        throw Exception("Database connection is not valid");
    }
    if (database().lock() == db || db == nullptr)
    {
        return;
    }
    disconnect();
    setDatabase(db);
    db->linkQuery(dynamic_cast<Query*>(this));
}

void QueryStatementManagement::disconnect()
{
    auto db = database().lock();
    if (!db)
    {
        return;
    }
    closeQuery(true);
    if (db != nullptr)
    {
        db->unlinkQuery(dynamic_cast<Query*>(this));
    }
    setDatabase({});
}

void Query::execute()
{
    const auto db = database().lock();
    if (!db)
    {
        throw Exception("Database connection is not valid");
    }
    if (db != nullptr && statement() != nullptr)
    {
        messages().clear();
        db->queryExecute(this);
    }
}

Variant Query::scalar()
{
    open();
    if (eof() || fieldCount() == 0)
    {
        close();
        return {};
    }
    const Variant& result = m_fields[0];
    close();
    return result;
}

//==============================================================================
Query::Query() noexcept
    : QueryStatementManagement(true)
    , m_fields(false)
{
}

Query::Query(const DatabaseConnection& db, const String& sql, const bool autoPrepare)
    : QueryStatementManagement(autoPrepare)
    , m_fields(false)
{
    if (db)
    {
        setDatabase(db->connection());

        const auto conn = database().lock();
        if (!conn)
        {
            throw Exception("Database connection is not valid");
        }

        conn->linkQuery(this);
    }
    Query::sql(sql);
}

Query::Query(const WPoolDatabaseConnection& db, const String& sql, const bool autoPrepare)
    : QueryStatementManagement(autoPrepare)
    , m_fields(false)
{
    if (!db.expired())
    {
        setDatabase(db);
        const auto conn = database().lock();
        if (!conn)
        {
            throw Exception("Database connection is not valid");
        }
        conn->linkQuery(this);
    }
    Query::sql(sql);
}

Query::~Query()
{
    try
    {
        closeQuery(true);

        if (const auto db = database().lock())
        {
            db->unlinkQuery(this);
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
}

bool Query::skipToNextParameter(String& sql, const char*& paramStart, const char*& paramEnd, const bool isPostgreSQL)
{
    // Looking up for SQL parameters
    const char* delimiters = "':-/$";

    // Find param start
    paramStart = strpbrk(paramEnd, delimiters);
    if (paramStart == nullptr)
    {
        return false;
    } // No more parameters

    auto rc = false;
    if (*paramStart == '\'')
    {
        // Started string constant
        const char* nextQuote = strchr(paramStart + 1, '\'');
        if (nextQuote == nullptr)
        {
            throw DatabaseException("unterminated string literal");
        }
        sql.append(paramEnd, static_cast<size_t>(nextQuote - paramEnd + 1));
        paramEnd = nextQuote + 1;
    }
    else if (*paramStart == '-' && paramStart[1] == '-')
    {
        // Started inline comment '--comment text', jump to the end of comment
        if (const char* endOfRow = strchr(paramStart + 1, '\n');
            endOfRow == nullptr)
        {
            // Comment at the end of the last row
            paramEnd = nullptr;
        }
        else
        {
            sql.append(paramEnd, static_cast<size_t>(endOfRow - paramEnd + 1));
            paramEnd = endOfRow + 1;
        }
    }
    else if (*paramStart == '/' && paramStart[1] == '*')
    {
        // Started C-style block comment, jump to the end of the comment
        const char* endOfRow = strstr(paramStart + 1, "*/");
        if (endOfRow == nullptr)
        {
            throw DatabaseException("unterminated block comment");
        }
        sql.append(paramEnd, static_cast<size_t>(endOfRow - paramEnd + 2));
        paramEnd = endOfRow + 2;
    }
    else if (*paramStart == '$' && isPostgreSQL)
    {
        // PostgreSQL dollar-quoted string: $$...$$ or $tag$...$tag$
        const char* tagEnd = paramStart + 1;
        while (std::isalnum(static_cast<unsigned char>(*tagEnd)) != 0 || *tagEnd == '_')
        {
            ++tagEnd;
        }

        if (*tagEnd == '$')
        {
            const String quoteTag(paramStart, tagEnd - paramStart + 1, 0);
            const char*  quoteEnd = strstr(tagEnd + 1, quoteTag.c_str());
            if (quoteEnd == nullptr)
            {
                throw DatabaseException("unterminated PostgreSQL dollar-quoted string");
            }
            sql.append(paramEnd, static_cast<size_t>(quoteEnd - paramEnd + quoteTag.length()));
            paramEnd = quoteEnd + quoteTag.length();
        }
        else
        {
            // Just a '$' character, not a dollar-quoted string.
            sql.append(paramEnd, static_cast<size_t>(paramStart - paramEnd + 1));
            paramEnd = paramStart + 1;
        }
    }
    else if (paramStart[1] == ':' || paramStart[1] == '=')
    {
        // Started PostgreSQL type qualifier '::' or assignment ':='
        sql.append(paramEnd, static_cast<size_t>(paramStart - paramEnd + 2));
        paramEnd = paramStart + 2;
    }
    else
    {
        sql.append(paramEnd, static_cast<size_t>(paramStart - paramEnd));
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

void Query::sqlParseParameter(String& sql, PoolDatabaseConnection& db, const char* paramStart, const char* paramEnd, int& paramNumber)
{
    const string_view paramName(paramStart + 1, paramEnd - paramStart - 1);
    auto              param = m_params.find(paramName);
    if (!param)
    {
        param = make_shared<QueryParameter>(paramName);
        m_params.add(param);
    }
    param->bindAdd(static_cast<uint32_t>(paramNumber));
    sql += db.paramMark(static_cast<uint32_t>(paramNumber));
    ++paramNumber;
}

void Query::sql(const String& _sql)
{
    const auto db = database().lock();
    if (!db)
    {
        throw Exception("Database connection is not valid");
    }
    if (db == nullptr)
    {
        throw DatabaseException("Query isn't connected to the database");
    }

    String sql = parseParameters(_sql);

    if (getSQL() != sql)
    {
        closeQuery(true);
        setSQL(std::move(sql));
        m_fields.clear();
    }
}

const char* Query::readParameter(String& sql, PoolDatabaseConnection& db, int& paramNumber, const char* paramStart, const char* paramEnd)
{
    for (;; ++paramEnd)
    {
        const auto current = static_cast<unsigned char>(*paramEnd);
        if (isalnum(current) != 0)
        {
            continue;
        }

        if (current == '_')
        {
            continue;
        }

        if (current == '.')
        {
            // Oracle ':new.' or ':old.'
            sql.append(paramStart, static_cast<size_t>(paramEnd - paramStart + 1));
            ++paramEnd;
            return paramEnd;
        }

        sqlParseParameter(sql, db, paramStart, paramEnd, paramNumber);
        break;
    }
    return paramEnd;
}

String Query::parseParameters(const String& _sql)
{
    m_params.clear();

    const auto sqlLength = _sql.length();
    if (_sql.find(':') == string::npos)
    {
        return _sql;
    }

    const char* paramStart {};
    const char* paramEnd = _sql.c_str();

    if (sqlLength > 64)
    {
        if (const auto paramEstimate = static_cast<size_t>(ranges::count(_sql, ':'));
            paramEstimate != 0)
        {
            m_params.reserve(paramEstimate);
        }
    }

    const auto db = database().lock();
    if (!db)
    {
        throw Exception("Database connection is not valid");
    }

    String sql;
    sql.reserve(_sql.length());
    const auto isPostgreSQL = db->connectionType() == DatabaseConnectionType::POSTGRES;

    auto paramNumber = 0;
    for (;;)
    {
        if (skipToNextParameter(sql, paramStart, paramEnd, isPostgreSQL))
        {
            paramEnd = readParameter(sql, *db, paramNumber, paramStart, paramEnd);
        }
        else if (paramStart == nullptr || paramEnd == nullptr)
        {
            break;
        }
    }

    if (paramEnd != nullptr)
    {
        sql += paramEnd;
    }
    return sql;
}

bool Query::open()
{
    const auto db = database().lock();
    if (!db)
    {
        throw Exception("Database connection is not valid");
    }

    db->queryOpen(this);

    return true;
}

void Query::fetch()
{
    const auto db = database().lock();
    if (!db)
    {
        throw Exception("Database connection is not valid");
    }

    if (!active())
    {
        throw DatabaseException("Query isn't open", source_location::current(), sql());
    }

    if (eof())
    {
        throw DatabaseException("No more rows to read", source_location::current(), sql());
    }

    db->queryFetch(this);
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

void QueryStatementManagement::setBulkMode(const bool bulkMode)
{
    m_bulkMode = bulkMode;
}

void sptk::THROW_QUERY_ERROR(const Query* query, const String& error, const std::source_location location)
{
    std::stringstream err;
    err << error;
    throw DatabaseException(err.str(), location, query->sql());
}
