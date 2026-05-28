/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-08                                             ║
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

#include "sptk5/RegularExpression.h"


#include <sptk5/db/InsertQuery.h>

using namespace std;
using namespace sptk;

String InsertQuery::reviewMsSqlQuery(const String& sql, const String& idFieldName)
{
    static const RegularExpression parseInsertQuery(R"(VALUES)", "i");

    if (sql.toUpperCase().contains("OUTPUT"))
    {
        throw Exception("OUTPUT clause is not supported in MS SQL InsertQuery.");
    }
    auto replaced = false;
    return parseInsertQuery.replaceAll(sql, "OUTPUT Inserted." + idFieldName + " VALUES", replaced);
}

String InsertQuery::reviewQuery(const DatabaseConnectionType connectionType, const String& sql,
                                const String& idFieldName)
{
    if (sql.toUpperCase().contains("RETURNING"))
    {
        throw Exception("RETURNING clause is not supported in InsertQuery.");
    }

    switch (connectionType)
    {
        using enum DatabaseConnectionType;
        case MSSQL_ODBC:
            return reviewMsSqlQuery(sql, idFieldName);
        case POSTGRES:
        case SQLITE3:
            return sql + " RETURNING " + idFieldName;
        case ORACLE:
        case ORACLE_OCI:
            return sql + " RETURNING " + idFieldName + " INTO :last_id";
        default:
            break;
    }
    return sql;
}

InsertQuery::InsertQuery(const DatabaseConnection& db, const String& sql, const String& idFieldName, const bool autoPrepare)
    : Query(db, reviewQuery(db->connectionType(), sql, idFieldName), autoPrepare)
    , m_idFieldName(idFieldName)
{
}

void InsertQuery::sql(const String& _sql)
{
    const auto db = database().lock();
    if (!db)
    {
        throw Exception("Database connection is not valid");
    }
    Query::sql(reviewQuery(db->connectionType(), _sql, m_idFieldName));
}

void InsertQuery::exec()
{
    const auto db = database().lock();
    if (!db)
    {
        throw Exception("Database connection is not valid");
    }
    m_id = 0;
    switch (db->connectionType())
    {
        using enum DatabaseConnectionType;
        case ORACLE:
        case ORACLE_OCI:
            param("last_id").setOutput();
            param("last_id").setNull(VariantDataType::VAR_INT64);
            m_id = static_cast<uint64_t>(scalar().asInt64());
            break;

        case POSTGRES:
        case SQLITE3:
            m_id = scalar().asInt64();
            break;

        case MYSQL:
            Query::exec();
            if (!m_lastInsertedId)
            {
                m_lastInsertedId = make_shared<Query>(database(), "SELECT LAST_INSERT_ID()");
            }
            m_id = static_cast<uint64_t>(m_lastInsertedId->scalar().asInt64());
            break;

        case MSSQL_ODBC:
            m_id = static_cast<uint64_t>(scalar().asInt64());
            break;
        default:
            throw Exception("Unsupported database connection type");
    }
}

String InsertQuery::sql() const
{
    return Query::sql();
}
