/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonObjectData.cpp - description                       ║
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
#include <sptk5/json/JsonDocument.h>

using namespace std;
using namespace sptk;
using namespace sptk::json;

ObjectData::ObjectData(Document* document, Element* parent)
: m_document(document), m_parent(parent)
{
}

ObjectData::~ObjectData()
{
    for (auto& itor: m_items)
        delete itor.second;
}

void ObjectData::setParent(Element* parent)
{
    if (m_parent != parent) {
        m_parent = parent;
        for (auto& itor: m_items)
            itor.second->m_parent = parent;
    }
}

void ObjectData::add(const string& name, Element* element)
{
    element->m_parent = m_parent;
    const string* sharedName = m_document->getString(name);
    auto itor = m_items.find(sharedName);
    if (itor != m_items.end())
        throw Exception("Element " + name + " conflicts with same name object");
    m_items[sharedName] = element;
}

Element* ObjectData::find(const string& name)
{
    const string* sharedName = m_document->getString(name);
    auto itor = m_items.find(sharedName);
    if (itor == m_items.end())
        return nullptr;
    return itor->second;
}

Element& ObjectData::operator[](const string& name)
{
    const string* sharedName = m_document->getString(name);
    auto itor = m_items.find(sharedName);
    Element* element;
    if (itor == m_items.end()) {
        element = new Element(m_document);
        element->m_parent = m_parent;
        m_items[sharedName] = element;
    } else
        element = itor->second;

    return *element;
}

const Element* ObjectData::find(const string& name) const
{
    const string* sharedName = m_document->getString(name);
    auto itor = m_items.find(sharedName);
    if (itor == m_items.end())
        throw Exception("Element name isn't found");
    return itor->second;
}

const Element& ObjectData::operator[](const string& name) const
{
    return *find(name);
}

void ObjectData::remove(const string& name)
{
    const string* sharedName = m_document->getString(name);
    auto itor = m_items.find(sharedName);
    if (itor == m_items.end())
        return;
    delete itor->second;
    m_items.erase(itor);
}

Element* ObjectData::move(const string& name)
{
    const string* sharedName = m_document->getString(name);
    auto itor = m_items.find(sharedName);
    if (itor == m_items.end())
        return nullptr;
    Element* data = itor->second;
    m_items.erase(itor);
    return data;
}
