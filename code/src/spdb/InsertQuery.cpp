/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
                                const String& idFieldName, const bool supportsReturning)
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
            return sql + " RETURNING " + idFieldName;
        case SQLITE3:
            // SQLite gained RETURNING in 3.35, and Enterprise Linux 9 ships 3.34.1 with nothing
            // newer in any repository. Where it is missing the statement stays as it is and exec()
            // asks the connection for the id instead.
            if (!supportsReturning)
            {
                return sql;
            }
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
    : Query(db, reviewQuery(db->connectionType(), sql, idFieldName, db->supportsReturning()), autoPrepare)
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
    Query::sql(reviewQuery(db->connectionType(), _sql, m_idFieldName, db->supportsReturning()));
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
            m_id = scalar().asInt64();
            break;

        case SQLITE3:
            if (db->supportsReturning())
            {
                m_id = scalar().asInt64();
            }
            else
            {
                // No RETURNING to read a row from, so the statement returns nothing and the id
                // comes from the connection. Read straight after the insert and before anything
                // else can be inserted on this connection, which holding it for the query arranges.
                Query::exec();
                m_id = static_cast<uint64_t>(db->lastInsertId());
            }
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
