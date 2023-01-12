/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

void WSFieldIndex::setElements(const Strings& elementNames, std::initializer_list<WSType*> fieldList)
{
    m_elements.clear();
    m_elementIndex.clear();

    size_t index = 0;
    for (auto* field: fieldList)
    {
        m_elementIndex[elementNames[index]] = field;
        ++index;
    }

    std::copy(fieldList.begin(), fieldList.end(), back_inserter(m_elements));
}

void WSFieldIndex::setAttributes(const Strings& attributeNames, std::initializer_list<WSType*> fieldList)
{
    m_attributes.clear();
    m_attributeIndex.clear();

    size_t index = 0;
    for (auto* field: fieldList)
    {
        m_attributeIndex[attributeNames[index]] = field;
        ++index;
    }

    std::copy(fieldList.begin(), fieldList.end(), back_inserter(m_attributes));
}

WSType* WSFieldIndex::find(const String& name) const
{
    auto itor = m_elementIndex.find(name);
    if (itor != m_elementIndex.end())
    {
        return itor->second;
    }

    itor = m_attributeIndex.find(name);
    if (itor != m_attributeIndex.end())
    {
        return itor->second;
    }

    return nullptr;
}

inline bool fieldTypeHas(WSFieldIndex::Group fieldType, WSFieldIndex::Group checkFor)
{
    return ((int) fieldType & (int) checkFor) != 0;
}

void WSFieldIndex::forEach(const function<bool(WSType*)>& method, Group fieldType)
{
    if (fieldTypeHas(fieldType, Group::ELEMENTS))
    {
        for (auto* field: m_elements)
        {
            if (!method(field))
            {
                return;
            }
        }
    }

    if (fieldTypeHas(fieldType, Group::ATTRIBUTES))
    {
        for (auto* field: m_attributes)
        {
            if (!method(field))
            {
                return;
            }
        }
    }
}

void WSFieldIndex::forEach(const function<bool(const WSType*)>& method, Group fieldType) const
{
    if (fieldTypeHas(fieldType, Group::ELEMENTS))
    {
        for (const auto* field: m_elements)
        {
            if (!method(field))
            {
                return;
            }
        }
    }

    if (fieldTypeHas(fieldType, Group::ATTRIBUTES))
    {
        for (const auto* field: m_attributes)
        {
            if (!method(field))
            {
                return;
            }
        }
    }
}

bool WSFieldIndex::hasElements() const
{
    return !m_elementIndex.empty();
}

bool WSFieldIndex::hasAttributes() const
{
    return !m_attributeIndex.empty();
}
