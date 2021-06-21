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

#include <sptk5/json/JsonElement.h>
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
    {
        delete itor.element();
    }
}

void ObjectData::add(const String& name, Element* element)
{
    element->m_parent = m_parent;
    const string* sharedName = m_document->getString(name);

    if (auto itor = m_items.find(sharedName);
        itor != m_items.end())
    {
        throw Exception("Element " + name + " conflicts with same name object");
    }
    m_items.set(sharedName, element);
}

Element* ObjectData::find(const String& name)
{
    const string* sharedName = m_document->getString(name);
    auto itor = m_items.find(sharedName);
    if (itor == m_items.end())
    {
        return nullptr;
    }
    return itor->element();
}

Element& ObjectData::operator[](const String& name)
{
    const string* sharedName = m_document->getString(name);
    auto itor = m_items.find(sharedName);
    Element* element = nullptr;
    if (itor == m_items.end())
    {
        element = new Element(m_document);
        element->m_parent = m_parent;
        m_items.set(sharedName, element);
    }
    else
    {
        element = itor->element();
    }

    return *element;
}

const Element* ObjectData::find(const String& name) const
{
    const string* sharedName = m_document->getString(name);
    auto itor = m_items.find(sharedName);
    if (itor == m_items.end())
    {
        throw Exception("Element name isn't found");
    }
    return itor->element();
}

const Element& ObjectData::operator[](const String& name) const
{
    return *find(name);
}

void ObjectData::remove(const String& name)
{
    const string* sharedName = m_document->getString(name);
    auto itor = m_items.find(sharedName);
    if (itor == m_items.end())
    {
        return;
    }
    delete itor->element();
    m_items.erase(itor);
}

Element* ObjectData::move(const String& name)
{
    const string* sharedName = m_document->getString(name);
    auto itor = m_items.find(sharedName);
    if (itor == m_items.end())
    {
        return nullptr;
    }
    auto* data = itor->element();
    m_items.erase(itor);
    return data;
}
