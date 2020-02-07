/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CParamList.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/db/QueryParameterList.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;


QueryParameterList::QueryParameterList() :
    m_bindingTypeChanged(true)
{
}

QueryParameterList::~QueryParameterList()
{
    try {
        clear();
    } catch (const Exception& e) {
        CERR(e.what() << endl)
    }
}

void QueryParameterList::clear()
{
    for (auto* item: m_items)
        delete item;

    m_items.clear();
    m_index.clear();
}

void QueryParameterList::add(QueryParameter* item)
{
    m_items.push_back(item);
    m_index[item->name()] = item;
    item->m_paramList = this;
}

QueryParameter* QueryParameterList::find(const String& paramName)
{
    auto itor = m_index.find(paramName);

    if (itor == m_index.end())
        return nullptr;

    return itor->second;
}

QueryParameter& QueryParameterList::operator[](const String& paramName) const
{
    auto itor = m_index.find(paramName);

    if (itor == m_index.end())
        throwException("Invalid parameter name: " << paramName)

    return *itor->second;
}

QueryParameter& QueryParameterList::operator[](int32_t index) const
{
    return *m_items[size_t(index)];
}

uint32_t QueryParameterList::size() const
{
    return (uint32_t) m_items.size();
}

void QueryParameterList::remove(uint32_t i)
{
    auto itor = m_items.begin() + i;
    QueryParameter* item = *itor;
    m_index.erase(item->name());
    m_items.erase(itor);
    delete item;
}

void QueryParameterList::enumerate(CParamVector& params)
{
    CParamVector::iterator ptor;
    IntList::iterator btor;
    params.resize(m_items.size() * 2);

    if (m_items.empty())
        return;

    size_t maxIndex = 0;

    for (auto* param: m_items) {
        IntList& bindIndex = param->m_bindParamIndexes;

        for (btor = bindIndex.begin(); btor != bindIndex.end(); ++btor) {
            size_t index = *btor;

            if (index >= params.size())
                params.resize(index + 1);

            params[index] = param;

            if (index > maxIndex)
                maxIndex = index;
        }
    }

    params.resize(maxIndex + 1);
}
