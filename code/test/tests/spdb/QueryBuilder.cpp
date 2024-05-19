/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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
#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

TEST(SPTK_QueryBuilder, selectSQL)
{
    QueryBuilder queryBuilder(
        "employee", "id",
        {"first_name", "last_name", "position", "department_id"});

    auto selectSQL = queryBuilder.selectSQL({}, {}, false);
    EXPECT_STREQ(
        "SELECT t.id, t.first_name, t.last_name, t.position, t.department_id FROM employee t",
        selectSQL.c_str());

    selectSQL = queryBuilder.selectSQL({"id=1", "name <> ''"}, {}, false);
    EXPECT_STREQ(
        "SELECT t.id, t.first_name, t.last_name, t.position, t.department_id FROM employee t WHERE (id=1) AND (name <> '')",
        selectSQL.c_str());

    selectSQL = queryBuilder.selectSQL({"id=1", "name <> ''"}, {"first_name", "id"}, false);
    EXPECT_STREQ(
        "SELECT first_name, id FROM employee t WHERE (id=1) AND (name <> '')",
        selectSQL.c_str());
}

TEST(SPTK_QueryBuilder, selectJoinsSQL)
{
    vector<QueryBuilder::Join> joins;
    joins.emplace_back("d", Strings({"name department_name"}), "JOIN department d ON d.id = t.department_id");
    joins.emplace_back("c", Strings({"name country_name"}), "JOIN country c ON c.id = d.country_id");

    QueryBuilder queryBuilder(
        "employee", "id",
        {"id", "first_name", "last_name", "position", "department_id"},
        joins);

    auto selectSQL = queryBuilder.selectSQL({"c.name = 'Australia'"}, {}, false);
    EXPECT_STREQ(
        "SELECT t.id, t.first_name, t.last_name, t.position, t.department_id, d.name department_name, c.name country_name "
        "FROM employee t "
        "JOIN department d ON d.id = t.department_id "
        "JOIN country c ON c.id = d.country_id "
        "WHERE (c.name = 'Australia')",
        selectSQL.c_str());
}

TEST(SPTK_QueryBuilder, insertSQL)
{
    QueryBuilder queryBuilder(
        "employee", "id",
        {"id", "first_name", "last_name", "position", "department_id"});

    auto insertSQL = queryBuilder.insertSQL({}, false);
    EXPECT_STREQ(
        "INSERT INTO employee(first_name, last_name, position, department_id) VALUES (:first_name, :last_name, :position, :department_id)",
        insertSQL.c_str());

    insertSQL = queryBuilder.insertSQL({"first_name", "last_name"}, false);
    EXPECT_STREQ(
        "INSERT INTO employee(first_name, last_name) VALUES (:first_name, :last_name)",
        insertSQL.c_str());
}

TEST(SPTK_QueryBuilder, updateSQL)
{
    QueryBuilder queryBuilder(
        "employee", "id",
        {"id", "first_name", "last_name", "position", "department_id"});

    auto updateSQL = queryBuilder.updateSQL({}, {}, false);
    EXPECT_STREQ(
        "UPDATE employee SET first_name = :first_name, last_name = :last_name, position = :position, department_id = :department_id WHERE id = :id",
        updateSQL.c_str());

    updateSQL = queryBuilder.updateSQL({"id = 1", "last_name = 'Doe'"}, {}, false);
    EXPECT_STREQ(
        "UPDATE employee SET first_name = :first_name, last_name = :last_name, position = :position, department_id = :department_id WHERE id = 1 AND last_name = 'Doe'",
        updateSQL.c_str());

    updateSQL = queryBuilder.updateSQL({}, {"first_name", "last_name"}, false);
    EXPECT_STREQ(
        "UPDATE employee SET first_name = :first_name, last_name = :last_name WHERE id = :id",
        updateSQL.c_str());
}

TEST(SPTK_QueryBuilder, deleteSQL)
{
    QueryBuilder queryBuilder(
        "employee", "id",
        {"id", "first_name", "last_name", "position", "department_id"});

    auto deleteSQL = queryBuilder.deleteSQL({}, false);
    EXPECT_STREQ(
        "DELETE FROM employee WHERE id = :id",
        deleteSQL.c_str());

    deleteSQL = queryBuilder.deleteSQL({"id = 1", "last_name = 'Doe'"}, false);
    EXPECT_STREQ(
        "DELETE FROM employee WHERE id = 1 AND last_name = 'Doe'",
        deleteSQL.c_str());
}
