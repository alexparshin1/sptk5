/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#include "sptk5/db/QueryBuilder.h"
#include "sptk5/cutils"

using namespace std;
using namespace sptk;

QueryBuilder::Join::Join(String tableAlias, Strings columns, String join)
: tableAlias(move(tableAlias)), columns(move(columns)), joinDefinition(move(join))
{

}

QueryBuilder::QueryBuilder(String tableName, String pkColumn, Strings columns,
                           const std::vector<Join>& joins)
: m_tableName(move(tableName)),
  m_pkColumn(move(pkColumn)),
  m_columns(move(columns)),
  m_joins(joins)
{
    m_columns.remove(m_pkColumn);
    for (auto& join: m_joins) {
        auto tableAlias = join.tableAlias;
        for (auto& column: join.columns) {
            Strings nameAndAlias(column, " ");
            if (nameAndAlias.size() == 2)
                m_columns.remove(nameAndAlias[1]);
            else {
                if (!tableAlias.empty() && column.startsWith(tableAlias + "."))
                    m_columns.remove(column.substr(tableAlias.length() + 1));
                else
                    m_columns.remove(column);
            }
        }
    }
}

String QueryBuilder::selectSQL(const Strings& filter, const Strings& columns, bool pretty) const
{
    stringstream query;

    query << "SELECT ";

    Strings outputColumns(columns);
    if (outputColumns.empty()) {
        outputColumns.push_back("t." + m_pkColumn);
        for (auto& column: m_columns)
            outputColumns.push_back("t." + column);
        for (auto& join: m_joins)
            for (auto& column: join.columns)
                outputColumns.push_back(join.tableAlias + "." + column);
    }
    query << outputColumns.join(", ") << endl;

    query << "  FROM " << m_tableName << " AS t" << endl;

    for (auto& join: m_joins)
        query << join.joinDefinition << endl;

    if (!filter.empty())
        query << " WHERE " << filter.join("\n   AND ");

    String queryStr = query.str();
    if (!pretty)
        queryStr = queryStr.replace("[\\n\\r\\s]+", " ").trim();

    return queryStr;
}

String QueryBuilder::insertSQL(const Strings& columns, bool pretty) const
{
    stringstream query;

    const Strings* insertColumns = &columns;
    if (columns.empty())
        insertColumns = &m_columns;

    query << "INSERT INTO " << m_tableName << "(" << insertColumns->join(", ") << ")" << endl
          << "VALUES (" << ":" << insertColumns->join(", :") << ")";

    String queryStr = query.str();
    if (!pretty)
        queryStr = queryStr.replace("[\\n\\r\\s]+", " ").trim();

    return queryStr;
}

String QueryBuilder::updateSQL(const Strings& filter, const Strings& columns, bool pretty) const
{
    stringstream query;

    query << "UPDATE " << m_tableName << endl
          << "   SET ";

    const Strings* updateColumns = &columns;
    if (columns.empty())
        updateColumns = &m_columns;

    bool first {true};
    for (auto& columnName: *updateColumns) {
        if (first)
            first = false;
        else
            query << ", ";
        query << columnName << " = :" << columnName;
    }
    query << "\n WHERE ";
    if (filter.empty())
        query << m_pkColumn << " = :" << m_pkColumn;
    else
        query << filter.join("\n   AND ");

    String queryStr = query.str();
    if (!pretty)
        queryStr = queryStr.replace("[\\n\\r\\s]+", " ").trim();

    return queryStr;
}

String QueryBuilder::deleteSQL(const Strings& filter, bool pretty) const
{
    stringstream query;

    query << "DELETE " << m_tableName << endl;

    query << " WHERE ";
    if (filter.empty())
        query << m_pkColumn << " = :" << m_pkColumn;
    else
        query << filter.join("\n   AND ");

    String queryStr = query.str();
    if (!pretty)
        queryStr = queryStr.replace("[\\n\\r\\s]+", " ").trim();

    return queryStr;
}

#if USE_GTEST

TEST(SPTK_QueryBuilder, selectSQL)
{
    QueryBuilder queryBuilder(
            "employee", "id",
            {"first_name", "last_name", "position", "department_id"});

    auto selectSQL = queryBuilder.selectSQL();
    EXPECT_STREQ(
            "SELECT t.id, t.first_name, t.last_name, t.position, t.department_id FROM employee AS t",
            selectSQL.c_str());

    selectSQL = queryBuilder.selectSQL({"id=1", "name <> ''"});
    EXPECT_STREQ(
            "SELECT t.id, t.first_name, t.last_name, t.position, t.department_id FROM employee AS t WHERE id=1 AND name <> ''",
            selectSQL.c_str());

    selectSQL = queryBuilder.selectSQL({"id=1", "name <> ''"}, {"first_name", "id"});
    EXPECT_STREQ(
            "SELECT first_name, id FROM employee AS t WHERE id=1 AND name <> ''",
            selectSQL.c_str());
}

TEST(SPTK_QueryBuilder, selectJoinsSQL)
{
    vector<QueryBuilder::Join>  joins;
    joins.emplace_back("d", Strings({"name department_name"}), "JOIN department d ON d.id = t.department_id");
    joins.emplace_back("c", Strings({"name country_name"}), "JOIN country c ON c.id = d.country_id");

    QueryBuilder queryBuilder(
            "employee", "id",
            {"id", "first_name", "last_name", "position", "department_id"},
            joins);

    auto selectSQL = queryBuilder.selectSQL({"c.name = 'Australia'"});
    EXPECT_STREQ(
            "SELECT t.id, t.first_name, t.last_name, t.position, t.department_id, d.name department_name, c.name country_name "
            "FROM employee AS t "
            "JOIN department d ON d.id = t.department_id "
            "JOIN country c ON c.id = d.country_id "
            "WHERE c.name = 'Australia'",
            selectSQL.c_str());
}

TEST(SPTK_QueryBuilder, insertSQL)
{
    QueryBuilder queryBuilder(
            "employee", "id",
            {"id", "first_name", "last_name", "position", "department_id"});

    auto insertSQL = queryBuilder.insertSQL();
    EXPECT_STREQ(
            "INSERT INTO employee(first_name, last_name, position, department_id) VALUES (:first_name, :last_name, :position, :department_id)",
            insertSQL.c_str());

    insertSQL = queryBuilder.insertSQL({"first_name", "last_name"});
    EXPECT_STREQ(
            "INSERT INTO employee(first_name, last_name) VALUES (:first_name, :last_name)",
            insertSQL.c_str());
}

TEST(SPTK_QueryBuilder, updateSQL)
{
    QueryBuilder queryBuilder(
            "employee", "id",
            {"id", "first_name", "last_name", "position", "department_id"});

    auto updateSQL = queryBuilder.updateSQL();
    EXPECT_STREQ(
            "UPDATE employee SET first_name = :first_name, last_name = :last_name, position = :position, department_id = :department_id WHERE id = :id",
            updateSQL.c_str());

    updateSQL = queryBuilder.updateSQL({"id = 1", "last_name = 'Doe'"});
    EXPECT_STREQ(
            "UPDATE employee SET first_name = :first_name, last_name = :last_name, position = :position, department_id = :department_id WHERE id = 1 AND last_name = 'Doe'",
            updateSQL.c_str());

    updateSQL = queryBuilder.updateSQL({}, {"first_name", "last_name"});
    EXPECT_STREQ(
            "UPDATE employee SET first_name = :first_name, last_name = :last_name WHERE id = :id",
            updateSQL.c_str());
}

TEST(SPTK_QueryBuilder, deleteSQL)
{
    QueryBuilder queryBuilder(
            "employee", "id",
            {"id", "first_name", "last_name", "position", "department_id"});

    auto deleteSQL = queryBuilder.deleteSQL();
    EXPECT_STREQ(
            "DELETE employee WHERE id = :id",
            deleteSQL.c_str());

    deleteSQL = queryBuilder.deleteSQL({"id = 1", "last_name = 'Doe'"});
    EXPECT_STREQ(
            "DELETE employee WHERE id = 1 AND last_name = 'Doe'",
            deleteSQL.c_str());
}

#endif
