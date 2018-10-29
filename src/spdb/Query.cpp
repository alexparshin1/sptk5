/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CQuery.cpp - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

using namespace std;
using namespace sptk;

int Query::nextObjectIndex = 0;

static const char cantAllocateStmt[] = "Can't allocate statement";

void Query::allocStmt()
{
    if (m_db == nullptr) {
        throw DatabaseException(cantAllocateStmt, __FILE__, __LINE__, m_sql);
    }
    m_db->queryAllocStmt(this);
}

void Query::freeStmt()
{
    if (m_db != nullptr && m_statement != nullptr) {
        m_db->queryFreeStmt(this);
        m_prepared = false;
        m_active = false;
    }
}

void Query::closeStmt()
{
    if (m_db != nullptr && m_statement != nullptr) {
        m_db->queryCloseStmt(this);
        m_active = false;
    }
}

void Query::prepare()
{
    if (!m_autoPrepare)
        throw DatabaseException("Can't prepare this statement");
    if (m_prepared)
        return;
    if (m_db != nullptr && m_statement != nullptr) {
        m_db->queryPrepare(this);
        m_prepared = true;
    }
}

void Query::unprepare()
{
    if (!m_prepared)
        return;
    if (m_db != nullptr && m_statement != nullptr) {
        m_db->queryUnprepare(this);
        m_prepared = false;
        m_active = false;
    }
}

void Query::execute()
{
    if (m_db != nullptr && m_statement != nullptr) {
        m_messages.clear();
        m_db->queryExecute(this);
    }
}

int Query::countCols()
{
    if (m_db != nullptr && m_statement != nullptr)
        return m_db->queryColCount(this);
    return 0;
}

void Query::colAttributes(int16_t column, int16_t descType, int32_t& value)
{
    if (m_db != nullptr && m_statement != nullptr)
        m_db->queryColAttributes(this, column, descType, value);
}

void Query::colAttributes(int16_t column, int16_t descType, char* buff, int32_t len)
{
    if (m_db != nullptr && m_statement != nullptr)
        m_db->queryColAttributes(this, column, descType, buff, len);
}

String Query::getError() const
{
    if (m_db != nullptr && m_statement != nullptr)
        return m_db->queryError(this);
    return "";
}

//==============================================================================
Query::Query() noexcept
: m_autoPrepare(true), m_statement(nullptr), m_connection(nullptr), m_prepared(false),
  m_active(false), m_eof(true), m_fields(true),
  m_db(nullptr), m_createdFile(nullptr), m_createdLine(0), m_bulkMode(false)
{
    m_objectIndex = nextObjectIndex;
    nextObjectIndex++; 
}

Query::Query(DatabaseConnection _db, const String& _sql, bool autoPrepare, const char* createdFile, unsigned createdLine)
    : m_fields(true), m_bulkMode(false)
{
    m_objectIndex = nextObjectIndex;
    nextObjectIndex++;
    m_statement = nullptr;
    m_autoPrepare = autoPrepare;
    m_prepared = false;
    m_active = false;
    m_eof = true;
    m_createdFile = createdFile;
    m_createdLine = createdLine;
    if (_db) {
        m_db = _db->connection();
        m_db->linkQuery(this);
    } else {
        m_db = nullptr;
    }
    sql(_sql);
}

Query::Query(PoolDatabaseConnection* _db, const String& _sql, bool autoPrepare, const char* createdFile, unsigned createdLine)
        : m_fields(true), m_bulkMode(false)
{
    m_objectIndex = nextObjectIndex;
    nextObjectIndex++;
    m_statement = nullptr;
    m_autoPrepare = autoPrepare;
    m_prepared = false;
    m_active = false;
    m_eof = true;
    m_createdFile = createdFile;
    m_createdLine = createdLine;
    if (_db != nullptr) {
        m_db = _db;
        m_db->linkQuery(this);
    } else {
        m_db = nullptr;
    }
    sql(_sql);
}

Query::Query(const Query& srcQuery)
:   m_fields(true)
{
    m_objectIndex = nextObjectIndex;
    nextObjectIndex++;
    m_statement = nullptr;
    m_autoPrepare = srcQuery.m_autoPrepare;
    m_prepared = false;
    m_active = false;
    m_eof = true;
    m_createdFile = srcQuery.m_createdFile;
    m_createdLine = srcQuery.m_createdLine;

    if (srcQuery.m_db != nullptr) {
        m_db = srcQuery.m_db;
        m_db->linkQuery(this);
    } else {
        m_db = nullptr;
    }

    sql(srcQuery.m_sql);
}

Query::~Query()
{
    try {
        closeQuery(true);
    }
    catch (...) { }
    if (m_db != nullptr)
        m_db->unlinkQuery(this);
}

void Query::sql(const String& _sql)
{
    // Looking up for SQL parameters
    char delimitters[] = "':-/";
    const char* paramStart;
    const char* paramEnd = _sql.c_str();
    int paramNumber = 0;

    m_params.clear();

    string odbcSQL;
    for (; ;) {
        // Find param start
        paramStart = strpbrk(paramEnd, delimitters);
        if (paramStart == nullptr)
            break;      // No more parameters

        if (*paramStart == '\'') {
            // Started string constant
            const char* nextQuote = strchr(paramStart + 1, '\'');
            if (nextQuote == nullptr)
                break;  // Quote opened but never closed?
            odbcSQL += string(paramEnd, nextQuote - paramEnd + 1);
            paramEnd = (char*) nextQuote + 1;
            continue;
        }

        if (*paramStart == '-' && paramStart[1] == '-') {
            // Started inline comment '--comment text', jump to the end of comment
            const char* endOfRow = strchr(paramStart + 1, '\n');
            if (endOfRow == nullptr)
                break;  // Comment at the end of last row
            odbcSQL += string(paramEnd, endOfRow - paramEnd + 1);
            paramEnd = (char*) endOfRow + 1;
            continue;
        }

        if (*paramStart == '/' && paramStart[1] == '*') {
            // Started block comment '/* comment text */', jump to the end of comment
            const char* endOfRow = strstr(paramStart + 1, "*/");
            if (endOfRow == nullptr)
                break;  // Comment at the end of last row
            odbcSQL += string(paramEnd, endOfRow - paramEnd + 2);
            paramEnd = (char*) endOfRow + 2;
            continue;
        }

        if (*paramStart == '/' || paramStart[1] == ':' || paramStart[1] == '=') {
            // Started PostgreSQL type qualifier '::' or assignment ':='
            odbcSQL += string(paramEnd, paramStart - paramEnd + 2);
            paramEnd = paramStart + 2;
            continue;
        }

        odbcSQL += string(paramEnd, paramStart - paramEnd);

        paramEnd = paramStart + 1;
        if (*paramStart != ':') {
            odbcSQL += *paramStart;
            continue;
        }

        for (; ; paramEnd++) {

            if (isalnum(*paramEnd) != 0)
                continue;

            if (*paramEnd == '_')
                continue;

            if (*paramEnd == '.') {
                // Oracle ':new.' or ':old.'
                odbcSQL += string(paramStart, paramEnd - paramStart + 1);
                paramEnd++;
                break;
            }

            string paramName(paramStart + 1, paramEnd - paramStart - 1);
            QueryParameter* param = m_params.find(paramName.c_str());
            if (param == nullptr) {
                param = new QueryParameter(paramName.c_str());
                m_params.add(param);
            }
            param->bindAdd(uint32_t(paramNumber));
            if (m_db == nullptr)
                throw DatabaseException("Query isn't connected to the database");
            odbcSQL += m_db->paramMark(uint32_t(paramNumber));
            paramNumber++;

            break;
        }
    }

    odbcSQL += paramEnd;

    for (int i = (int) m_params.size() - 1; i >= 0; i--)
        if (m_params[i].bindCount() == 0)
            m_params.remove(uint32_t(i));

    if (m_sql != odbcSQL) {
        m_sql = odbcSQL;
        if (active())
            close();
        m_prepared = false;
        m_fields.clear();
    }
}

bool Query::open()
{
    if (m_db == nullptr)
        throw DatabaseException("Query is not connected to the database", __FILE__, __LINE__, m_sql);

    try {
        m_db->queryOpen(this);
    }
    catch (const DatabaseException& e) {
        if (strstr(e.what(), "connection") == nullptr)
            throw;
        m_db->close();
        m_db->open();
        m_db->queryOpen(this);
    }

    return true;
}

void Query::fetch()
{
    if (m_db == nullptr || !m_active) {
        throw DatabaseException("Dataset isn't open", __FILE__, __LINE__, m_sql);
    }

    m_db->queryFetch(this);
}

void Query::closeQuery(bool releaseStatement)
{
    m_active = false;
    m_eof = true;
    if (m_statement != nullptr) {
        if (releaseStatement) {
            freeStmt();
        } else {
            closeStmt();
        }
    }
    //m_fields.clear();
}

void Query::connect(PoolDatabaseConnection* _db)
{
    if (m_db == _db)
        return;
    disconnect();
    m_db = _db;
    m_db->linkQuery(this);
}

void Query::disconnect()
{
    closeQuery(true);
    if (m_db != nullptr)
        m_db->unlinkQuery(this);
    m_db = nullptr;
}

bool Query::readField(const char*, Variant&)
{
    //fvalue = m_fields[fname];
    return true;
}

bool Query::writeField(const char*, const Variant&)
{
    //m_fields[fname] = fvalue;
    return true;
}

void Query::notImplemented(const String& functionName) const
{
    throw DatabaseException(functionName + " isn't implemented", __FILE__, __LINE__, m_sql);
}

void Query::throwError(const String& method, const String& error)
{
    String errorText("Exception in " + method + ": " + error);
    throw DatabaseException(errorText);
}
