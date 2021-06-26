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

#pragma once

#include <sptk5/Strings.h>
#include <memory>

namespace sptk {

class QueryBuilder
{
public:

    class Join
    {
    public:
        Join(const String& tableAlias, const Strings& columns, const String& join);

        String tableAlias;
        Strings columns;
        String joinDefinition;
    };

    QueryBuilder(const String& tableName, const String& pkColumn, const Strings& columns = {},
                 const std::vector<Join>& joins = {});

    virtual ~QueryBuilder() = default;

    virtual String selectSQL(const Strings& filter, const Strings& columns, bool pretty) const;

    virtual String insertSQL(const Strings& columns, bool pretty) const;

    virtual String updateSQL(const Strings& filter, const Strings& columns, bool pretty) const;

    virtual String deleteSQL(const Strings& filter, bool pretty) const;

    String tableName() const;

    String pkColumnName() const;

private:
    String m_tableName;
    String m_pkColumn;
    Strings m_columns;
    std::vector<Join> m_joins;

    Strings makeSelectColumns(const Strings& columns) const;

    void removeUnNeededColumns(const Join& join, const String& tableAlias);
};

}
