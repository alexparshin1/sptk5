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

#ifndef SPTK_QUERYBUILDER_H
#define SPTK_QUERYBUILDER_H

#include <sptk5/Strings.h>
#include <memory>

namespace sptk {

class QueryBuilder
{
public:

    class Join
    {
    public:
        Join(String tableAlias, Strings columns, String join);

        const String  tableAlias;
        const Strings columns;
        const String  joinDefinition;
    };

    QueryBuilder(String tableName, String pkColumn, Strings columns={}, const std::vector<Join>& joins={});

    String selectSQL(const Strings& filter= {}, const Strings& columns= {}, bool pretty= false);
    String insertSQL(const Strings& columns= {}, bool pretty= false);
    String updateSQL(const Strings& filter= {}, const Strings& columns= {}, bool pretty= false);
    String deleteSQL(const Strings& filter= {}, bool pretty= false);

private:
    String              m_tableName;
    String              m_pkColumn;
    Strings             m_columns;
    std::vector<Join>   m_joins;
};

}

#endif
