/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonArrayData.cpp - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/json/JsonElement.h>
#include <sptk5/json/JsonArrayData.h>

using namespace std;
using namespace sptk;
using namespace sptk::json;

ArrayData::ArrayData(const Document* document, Element* parent)
: m_document(document), m_parent(parent)
{
}

ArrayData::~ArrayData()
{
    for (Element* element: m_items)
        delete element;
}

void ArrayData::setParent(Element* parent)
{
    if (m_parent != parent) {
        m_parent = parent;
        for (Element *element: m_items)
            element->m_parent = parent;
    }
}

void ArrayData::add(Element* element)
{
    element->m_parent = m_parent;
    m_items.push_back(element);
}

Element& ArrayData::operator[](size_t index)
{
    if (index >= m_items.size())
        throw Exception("Element index out of bound");
    return *m_items[index];
}

const Element& ArrayData::operator[](size_t index) const
{
    if (index >= m_items.size())
        throw Exception("Element index out of bound");
    return *m_items[index];
}

void ArrayData::remove(size_t index)
{
    if (index >= m_items.size())
        return;
    delete m_items[index];
    m_items.erase(m_items.begin() + index);
}
