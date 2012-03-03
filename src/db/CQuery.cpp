/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CQuery.cpp  -  description
                             -------------------
    begin                : Tue Jan 11 2000
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sptk5/db/CDatabaseDriver.h>
#include <sptk5/db/CQuery.h>
#include <sptk5/CException.h>

using namespace std;
using namespace sptk;

int CQuery::nextObjectIndex = 0;

static const char cantAllocateStmt[] = "Can't allocate statement";

static inline bool successful(int ret)
{
    return (ret & 1) == ret;
}

void CQuery::allocStmt()
{
    if (!m_db) {
        logText("Error in CQuery::allocStmt(): " + string(cantAllocateStmt));
        throw CException(cantAllocateStmt, __FILE__, __LINE__, m_sql);
    }
    m_db->queryAllocStmt(this);
}

void CQuery::freeStmt()
{
    if (m_db && m_statement) {
        m_db->queryFreeStmt(this);
        m_prepared = false;
        m_active = false;
    }
}

void CQuery::closeStmt()
{
    if (m_db && m_statement) {
        m_db->queryCloseStmt(this);
        m_active = false;
    }
}

void CQuery::prepare()
{
    if (m_prepared)
        return;
    if (m_db && m_statement) {
        m_db->queryPrepare(this);
        m_prepared = true;
    }
}

void CQuery::unprepare()
{
    if (!m_prepared)
        return;
    if (m_db && m_statement) {
        m_db->queryUnprepare(this);
        m_prepared = false;
        m_active = false;
    }
}

void CQuery::execute()
{
    if (m_db && m_statement) {
        m_messages.clear();
        m_db->queryExecute(this);
    }
}

int CQuery::countCols()
{
    if (m_db && m_statement)
        return m_db->queryColCount(this);
    return 0;
}

void CQuery::colAttributes(int16_t column, int16_t descType, int32_t& value)
{
    if (m_db && m_statement)
        m_db->queryColAttributes(this, column, descType, value);
}

void CQuery::colAttributes(int16_t column, int16_t descType, char *buff, int32_t len)
{
    if (m_db && m_statement)
        m_db->queryColAttributes(this, column, descType, buff, len);
}

string CQuery::getError() const
{
    if (m_db && m_statement)
        return m_db->queryError(this);
    return "";
}
//==============================================================================
CQuery::CQuery(CDatabaseDriver *_db, string _sql, const char* createdFile, unsigned createdLine) :
        CDataSource(), m_fields(true)
{
    m_objectIndex = nextObjectIndex;
    nextObjectIndex++;
    m_statement = 0L;
    m_autoPrepare = true;
    m_prepared = false;
    m_active = false;
    m_eof = true;
    m_duration = 0;
    m_totalDuration = 0;
    m_totalCalls = 0;
    m_createdFile = createdFile;
    m_createdLine = createdLine;
    if (_db) {
        m_db = _db;
        m_db->linkQuery(this);
    } else {
        m_db = NULL;
    }
    sql(_sql);
}

CQuery::CQuery(const CQuery& srcQuery) :
        CDataSource(), m_fields(true)
{
    m_objectIndex = nextObjectIndex;
    nextObjectIndex++;
    m_statement = 0L;
    m_prepared = false;
    m_active = false;
    m_eof = true;
    m_duration = 0;
    m_totalDuration = 0;
    m_totalCalls = 0;
    m_createdFile = srcQuery.m_createdFile;
    m_createdLine = srcQuery.m_createdLine;

    if (srcQuery.m_db) {
        m_db = srcQuery.m_db;
        m_db->linkQuery(this);
    } else {
        m_db = NULL;
    }

    sql(srcQuery.m_sql);
}

CQuery::~CQuery()
{
    closeQuery(true);
    storeStatistics();
    if (m_db)
        m_db->unlinkQuery(this);
}

void CQuery::sql(string _sql)
{
    // Looking up for SQL parameters
    string paramName;
    char delimitter[] = " ";
    char delimitters[] = "'\":'";
    char *s = strdup(_sql.c_str());
    char *paramStart = s;
    char *paramEnd;
    int paramNumber = 0;

    m_params.clear();

    string odbcSQL = "";
    bool endOfString;
    for (;;) {
        paramEnd = strpbrk(paramStart, delimitters);
        if (!paramEnd) {
            odbcSQL += paramStart;
            break;
        }
        *delimitter = *paramEnd;

        if (*paramEnd == ':') {
            if (paramEnd != s && isalnum(*(paramEnd - 1))) {
                *paramEnd = char(0);
                odbcSQL += paramStart;
                odbcSQL += ":";
                paramStart = paramEnd + 1;
                continue;
            }
            if (paramEnd[1] == ':') {
                paramEnd++;
                *paramEnd = char(0);
                odbcSQL += paramStart;
                paramStart = paramEnd + 1;
                continue;
            }
        }

        if (*paramEnd == '\'' || *paramEnd == '"') {
            paramEnd = strpbrk(paramEnd + 1, delimitter);
            if (!paramEnd) {
                break; // Unmatched quotes
            }
            *paramEnd = char(0);
            odbcSQL += paramStart;
            odbcSQL += delimitter;
            paramStart = paramEnd + 1;
            continue;
        }

        *paramEnd = char(0);
        odbcSQL += paramStart;
        paramStart = paramEnd + 1;

        delimitter[0] = 0;
        char *ptr = paramStart;
        for (; *ptr; ptr++) {
            char c = *ptr;
            if (c == '_')
                continue;
            if (!isalnum(c)) {
                delimitter[0] = c;
                break;
            }
        }

        paramEnd = ptr;
        endOfString = (*paramEnd == 0);
        *paramEnd = char(0);
        if (ptr != paramStart) {
            CParam *param = m_params.find(paramStart);
            if (!param) {
                param = new CParam(paramStart);
                m_params.add(param);
            }
            param->bindAdd(paramNumber);
            if (!m_db)
                throw CException("Query isn't connected to the database");
            odbcSQL += m_db->paramMark(paramNumber) + delimitter;
            paramNumber++;
        } else {
            odbcSQL += ":";
        }
        paramStart = paramEnd + 1;
        if (endOfString)
            break;
    }

    free(s);

    for (int i = (int) m_params.size() - 1; i >= 0; i--)
        if (!m_params[i].bindCount())
            m_params.remove(i);

    if (m_sql != odbcSQL) {
        m_sql = odbcSQL;
        if (active())
            close();
        m_prepared = false;
        m_fields.clear();
    }
}

bool CQuery::open()
{
    if (!m_db)
        throw CException("Query is not connected to the database", __FILE__, __LINE__, m_sql);

    if (m_db->logFile())
        logText("Opening query: " + replaceAll(m_sql, "\n", " "));

    m_params.rewind();

    uint32_t started = CDateTime::TimeOfDayMs();
    m_db->queryOpen(this);
    uint32_t finished = CDateTime::TimeOfDayMs();

    // Execute duration, in seconds
    m_duration = (finished - started) / 1000.0;
    if (m_duration < 0)
        m_duration += 86400.0;
    if (m_db->logFile()) {
        char buffer[64];
        sprintf(buffer, "[Q%i] Duration %0.3f sec", m_objectIndex, m_duration);
        *m_db->logFile() << CLP_DEBUG << buffer << endl;
    }

    m_totalDuration += m_duration;
    m_totalCalls++;

    m_fields.rewind();

    return true;
}

void CQuery::fetch()
{
    m_fields.rewind();
    if (!m_db || !m_active) {
        logText("Error in CQuery::fetch(): Dataset isn't open");
        throw CException("Dataset isn't open", __FILE__, __LINE__, m_sql);
    }

    m_db->queryFetch(this);
}

void CQuery::closeQuery(bool releaseStatement)
{
    m_active = false;
    m_eof = true;
    if (m_statement) {
        if (releaseStatement) {
            freeStmt();
            logText("Released");
        } else {
            closeStmt();
            logText("Closed");
        }
    }
    //m_fields.clear();
}

void CQuery::connect(CDatabaseDriver *_db)
{
    if (m_db == _db)
        return;
    disconnect();
    m_db = _db;
    m_db->linkQuery(this);
}

void CQuery::disconnect()
{
    closeQuery(true);
    if (m_db)
        m_db->unlinkQuery(this);
    m_db = NULL;
}

bool CQuery::readField(const char *fname, CVariant& fvalue)
{
    //fvalue = m_fields[fname];
    return true;
}

bool CQuery::writeField(const char *fname, const CVariant& fvalue)
{
    //m_fields[fname] = fvalue;
    return true;
}

void CQuery::notImplemented(string functionName) const
{
    throw CException(functionName + " isn't implemented", __FILE__, __LINE__, m_sql);
}

void CQuery::logText(std::string text, const CLogPriority& priority)
{
    if (!m_db)
        return;
    CBaseLog* alog = m_db->logFile();
    if (alog) {
        CBaseLog& blog = *alog;
        blog << priority;
        blog << "[Q" << m_objectIndex << "] ";
        blog << text.c_str() << endl;
    }
}

void CQuery::logAndThrow(string method, string error) throw (CException)
{
    string errorText("Exception in " + method + ": " + error);
    logText(errorText, CLP_ERROR);
    throw CException(errorText);
}

void CQuery::storeStatistics()
{
    if (!m_db)
        return;
    if (m_createdFile) {
        char* buffer = new char[strlen(m_createdFile) + 16];
        sprintf(buffer, "%s:%i", m_createdFile, m_createdLine);
        m_db->addStatistics(buffer, m_totalDuration, m_totalCalls, m_sql);
        delete buffer;
        m_totalDuration = 0;
        m_totalCalls = 0;
    }
}
