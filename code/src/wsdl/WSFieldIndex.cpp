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

#include <sptk5/wsdl/WSFieldIndex.h>

using namespace std;
using namespace sptk;

void WSFieldIndex::set(const Strings& fieldNames, std::initializer_list<WSType*> fieldList)
{
    size_t index = 0;
    for (auto* field: fieldList) {
        m_fields[ fieldNames[index] ] = field;
        ++index;
    }
}

WSType* WSFieldIndex::find(const String& name) const
{
    auto itor = m_fields.find(name);
    if (itor == m_fields.end())
        return nullptr;
    return itor->second;
}

void WSFieldIndex::forEach(const function<bool(const String&, WSType*)>& method)
{
    for (auto& field: m_fields) {
        if (!method(field.first, field.second))
            break;
    }
}

void WSFieldIndex::forEach(const function<bool(const String&, WSType*)>& method) const
{
    for (auto& field: m_fields) {
        if (!method(field.first, field.second))
            break;
    }
}
