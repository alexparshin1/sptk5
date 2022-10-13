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

#include <sptk5/RegularExpression.h>
#include <sstream>

#include "sptk5/db/QueryBuilder.h"

using namespace std;
using namespace sptk;

QueryBuilder::Join::Join(const String& tableAlias, const Strings& columns, const String& join)
    : tableAlias(tableAlias)
    , columns(columns)
    , joinDefinition(join)
{
}

QueryBuilder::QueryBuilder(const String& tableName, const String& pkColumn, const Strings& columns,
                           const vector<Join>& joins)
    : m_tableName(tableName)
    , m_pkColumn(pkColumn)
    , m_columns(columns)
    , m_joins(joins)
{
    m_columns.remove(m_pkColumn);
    for (const auto& join: m_joins)
    {
        auto tableAlias = join.tableAlias;
        removeUnNeededColumns(join, tableAlias);
    }
}

void QueryBuilder::removeUnNeededColumns(const Join& join, const String& tableAlias)
{
    static const RegularExpression matchExpressionAndAlias(R"(^.*\s(\S+))");
    for (const auto& column: join.columns)
    {
        auto matches = matchExpressionAndAlias.m(column);
        if (matches)
        {
            auto alias = matches[0].value;
            m_columns.remove(alias);
        }
        else
        {
            if (!tableAlias.empty() && column.startsWith(tableAlias + "."))
            {
                m_columns.remove(column.substr(tableAlias.length() + 1));
            }
            else
            {
                m_columns.remove(column);
            }
        }
    }
}

String QueryBuilder::selectSQL(const Strings& filter, const Strings& columns, bool pretty) const
{
    stringstream query;

    query << "SELECT ";

    Strings outputColumns = makeSelectColumns(columns);
    query << outputColumns.join(", ") << endl;

    query << "  FROM " << m_tableName << " t" << endl;

    for (const auto& join: m_joins)
    {
        query << join.joinDefinition << endl;
    }

    if (!filter.empty())
    {
        bool first = true;
        for (const auto& condition: filter)
        {
            if (condition.trim().empty())
            {
                continue;
            }
            if (first)
            {
                first = false;
                query << " WHERE (" << condition << ")";
            }
            else
            {
                query << " AND (" << condition << ")";
            }
        }
    }

    String queryStr = query.str();
    if (!pretty)
    {
        queryStr = queryStr.replace("[\\n\\r\\s]+", " ").trim();
    }

    return queryStr;
}

Strings QueryBuilder::makeSelectColumns(const Strings& columns) const
{
    static const RegularExpression matchExpression(R"([\+\-*/~\(\)])");

    Strings outputColumns(columns);
    if (!outputColumns.empty())
    {
        return outputColumns;
    }

    outputColumns.push_back("t." + m_pkColumn);
    for (const auto& column: m_columns)
    {
        if (column.find(' ') == string::npos)
        {
            outputColumns.push_back("t." + column);
        }
        else
        {
            outputColumns.push_back(column);
        }
    }
    for (const auto& join: m_joins)
    {
        for (const auto& column: join.columns)
        {
            if (matchExpression.matches(column))
            {
                // if column contains expression and alias, don't add table alias prefix
                outputColumns.push_back(column);
            }
            else
            {
                outputColumns.push_back(join.tableAlias + "." + column);
            }
        }
    }

    return outputColumns;
}

String QueryBuilder::insertSQL(const Strings& columns, bool pretty) const
{
    stringstream query;

    const Strings* insertColumns = &columns;
    if (columns.empty())
    {
        insertColumns = &m_columns;
    }

    Strings filteredColumns;
    for (const auto& columnName: *insertColumns)
    {
        if (columnName.find(' ') != string::npos)
        {
            continue;
        }
        filteredColumns.push_back(columnName);
    }

    query << "INSERT INTO " << m_tableName << "(" << filteredColumns.join(", ") << ")" << endl
          << "VALUES ("
          << ":" << filteredColumns.join(", :") << ")";

    String queryStr = query.str();
    if (!pretty)
    {
        queryStr = queryStr.replace("[\\n\\r\\s]+", " ").trim();
    }

    return queryStr;
}

String QueryBuilder::updateSQL(const Strings& filter, const Strings& columns, bool pretty) const
{
    stringstream query;

    query << "UPDATE " << m_tableName << endl
          << "   SET ";

    const Strings* updateColumns = &columns;
    if (columns.empty())
    {
        updateColumns = &m_columns;
    }

    bool first {true};
    for (const auto& columnName: *updateColumns)
    {
        if (columnName.find(' ') != string::npos)
        {
            continue;
        }
        if (first)
        {
            first = false;
        }
        else
        {
            query << ", ";
        }
        query << columnName << " = :" << columnName;
    }
    query << "\n WHERE ";
    if (filter.empty())
    {
        query << m_pkColumn << " = :" << m_pkColumn;
    }
    else
    {
        query << filter.join("\n   AND ");
    }

    String queryStr = query.str();
    if (!pretty)
    {
        queryStr = queryStr.replace("[\\n\\r\\s]+", " ").trim();
    }

    return queryStr;
}

String QueryBuilder::deleteSQL(const Strings& filter, bool pretty) const
{
    stringstream query;

    query << "DELETE FROM " << m_tableName << endl;

    query << " WHERE ";
    if (filter.empty())
    {
        query << m_pkColumn << " = :" << m_pkColumn;
    }
    else
    {
        query << filter.join("\n   AND ");
    }

    String queryStr = query.str();
    if (!pretty)
    {
        queryStr = queryStr.replace("[\\n\\r\\s]+", " ").trim();
    }

    return queryStr;
}

String QueryBuilder::tableName() const
{
    return m_tableName;
}

String QueryBuilder::pkColumnName() const
{
    return m_pkColumn;
}

