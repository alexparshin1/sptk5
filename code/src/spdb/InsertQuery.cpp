/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/db/InsertQuery.h>

using namespace std;
using namespace sptk;

String InsertQuery::reviewQuery(DatabaseConnectionType connectionType, const String& sql,
                                const String& idFieldName)
{
    switch (connectionType) {
        case DatabaseConnectionType::POSTGRES:
            return sql + " RETURNING " + idFieldName;
        case DatabaseConnectionType::ORACLE:
            return sql + " RETURNING " + idFieldName + " INTO :last_id";
        default:
            break;
    }
    return sql;
}

InsertQuery::InsertQuery(DatabaseConnection db, const String& sql, const String& idFieldName)
: Query(db, reviewQuery(db->connectionType(), sql, idFieldName), true), m_idFieldName(idFieldName)
{
}

void InsertQuery::sql(const String& _sql)
{
    if (!database())
        throwException("Database connection is not defined yet")
    Query::sql(reviewQuery(database()->connectionType(), _sql, m_idFieldName));
}

void InsertQuery::exec()
{
    m_id = 0;
    switch ( database()->connectionType() ) {

    case DatabaseConnectionType::ORACLE:
        param("last_id").setOutput();
        param("last_id").setNull(VAR_INT);
        open();
        m_id = (uint64_t) (*this)[0].asInteger();
        close();
        break;

    case DatabaseConnectionType::POSTGRES:
        open();
        m_id = (uint64_t) (*this)[0].asInteger();
        close();
        break;

    case DatabaseConnectionType::MYSQL:
        Query::exec();
        if (!m_lastInsertedId)
            m_lastInsertedId = make_shared<Query>(database(),"SELECT LAST_INSERT_ID()");
        m_lastInsertedId->open();
        m_id = (uint64_t) (*m_lastInsertedId)[0].asInteger();
        m_lastInsertedId->close();
        break;
    case DatabaseConnectionType::MSSQL_ODBC:
        Query::exec();
        if (!m_lastInsertedId)
            m_lastInsertedId = make_shared<Query>(database(),"SELECT @@IDENTITY");
        m_lastInsertedId->open();
        m_id = (uint64_t) (*m_lastInsertedId)[0].asInteger();
        m_lastInsertedId->close();
        break;
    default:
        throwException ("Unsupported database connection type")
    }
}

String InsertQuery::sql() const
{
    return Query::sql();
}
