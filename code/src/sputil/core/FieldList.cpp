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

#include <cstring>

#include <sptk5/Exception.h>
#include <sptk5/FieldList.h>

using namespace std;
using namespace sptk;

FieldList::FieldList(bool indexed)
{
    if (indexed)
    {
        m_index = make_shared<Map>();
    }
}

void FieldList::clear()
{
    m_list.clear();
    if (m_index)
    {
        m_index->clear();
    }
}

Field& FieldList::push_back(const String& fname, bool checkDuplicates)
{
    if (checkDuplicates)
    {
        if (const auto pfld = findField(fname))
        {
            throw Exception("Attempt to duplicate field name");
        }
    }

    const auto field = make_shared<Field>(fname);

    m_list.push_back(field);

    if (m_index)
    {
        (*m_index)[fname] = field;
    }

    return *field;
}

Field& FieldList::push_back(const SField& field)
{
    m_list.push_back(field);

    if (m_index)
    {
        (*m_index)[field->m_name] = field;
    }

    return *field;
}

SField FieldList::findField(const String& fname) const
{
    if (m_index)
    {
        if (const auto itor = m_index->find(fname);
            itor != m_index->end())
        {
            return itor->second;
        }
    }
    else
    {
        for (const auto& field: *this)
        {
            if (strcasecmp(field->m_name.c_str(), fname.c_str()) == 0)
            {
                return field;
            }
        }
    }
    return nullptr;
}

void FieldList::exportTo(const xdoc::SNode& node, bool compactMode, bool nullLargeData) const
{
    for (const auto& field: *this)
    {
        field->exportTo(node, compactMode, nullLargeData);
    }
}
