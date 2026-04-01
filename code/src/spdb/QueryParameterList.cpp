/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-06                                             ║
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

#include <sptk5/cutils>
#include <sptk5/db/QueryParameter.h>
#include <sptk5/db/QueryParameterList.h>

using namespace std;
using namespace sptk;

void QueryParameterList::clear()
{
    m_items.clear();
    m_index.clear();
}

void QueryParameterList::add(const SQueryParameter& item)
{
    m_items.push_back(item);
    m_index[item->name()] = item;
}

SQueryParameter QueryParameterList::find(const String& paramName)
{
    const auto itor = m_index.find(paramName);

    if (itor == m_index.end())
    {
        return nullptr;
    }

    return itor->second;
}

QueryParameter& QueryParameterList::operator[](const String& paramName) const
{
    const auto itor = m_index.find(paramName);

    if (itor == m_index.end())
    {
        throw Exception("Invalid parameter name: " + paramName);
    }

    return *itor->second;
}

QueryParameter& QueryParameterList::operator[](const size_t index) const
{
    if (index >= m_items.size())
    {
        throw Exception("Invalid parameter index");
    }
    return *m_items[index];
}

size_t QueryParameterList::size() const
{
    return m_items.size();
}

void QueryParameterList::remove(const size_t i)
{
    if (i >= m_items.size())
    {
        throw Exception("Invalid parameter index");
    }
    const auto  itor = m_items.begin() + static_cast<int>(i);
    const auto& item = *itor;
    m_index.erase(item->name());
    m_items.erase(itor);
}

void QueryParameterList::enumerate(ParamVector& params) const
{
    params.clear();

    if (m_items.empty())
    {
        return;
    }

    size_t maxIndex = 0;

    for (const auto& param: m_items)
    {
        for (const auto& bindIndex = param->m_bindParamIndexes;
             const auto  index: bindIndex)
        {
            if (index > maxIndex)
            {
                maxIndex = index;
            }
        }
    }

    params.assign(maxIndex + 1, nullptr);

    for (const auto& param: m_items)
    {
        for (const auto& bindIndex = param->m_bindParamIndexes;
             const auto  index: bindIndex)
        {
            params[index] = param;
        }
    }
}
