/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Attributes.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/cxml>

using namespace sptk;
using namespace sptk::xml;

Attribute::Attribute(Element* parent, const char* tagname, Value avalue) :
    NamedItem(*parent->document())
{
    name(tagname);
    value(avalue);
    parent->attributes().push_back(this);
}

Attribute::Attribute(Element* parent, const std::string& tagname, Value avalue) :
    NamedItem(*parent->document())
{
    name(tagname);
    value(avalue);
    parent->attributes().push_back(this);
}

/// @brief Returns the value of the node
const String& Attribute::value() const noexcept
{
    return m_value;
}

/// @brief Sets new value to node.
/// @param new_value const std::string &, new value
/// @see value()
void Attribute::value(const std::string &new_value) 
{
    m_value = new_value;
}

/// @brief Sets new value to node
/// @param new_value const char *, value to set
/// @see value()
void Attribute::value(const char *new_value) 
{
    m_value = new_value;
}

Attributes& Attributes::operator =(const Attributes& s)
{
    if (&s == this)
        return *this;
    clear();
    for (auto node: s)
        new Attribute(m_parent, node->name(), node->value());
    return *this;
}

Attribute* Attributes::getAttributeNode(std::string attr)
{
    const auto itor = findFirst(attr.c_str());
    if (itor != end())
        return (Attribute*) *itor;
    return nullptr;
}

const Attribute* Attributes::getAttributeNode(std::string attr) const
{
    const auto itor = findFirst(attr.c_str());
    if (itor != end())
        return (const Attribute*) *itor;
    return nullptr;
}

Value Attributes::getAttribute(std::string attr, const char *defaultValue) const
{
    const auto itor = findFirst(attr.c_str());
    if (itor != end())
        return (*itor)->value();
    Value rc;
    if (defaultValue != nullptr)
        rc = defaultValue;
    return rc;
}

void Attributes::setAttribute(std::string attr, Value value, const char *defaultValue)
{
    const auto itor = findFirst(attr);
    if (defaultValue != nullptr && value.str() == defaultValue) {
        if (itor != end()) {
            delete *itor;
            erase(itor);
        }
        return;
    }
    if (itor != end())
        (*itor)->value(value);
    else
        new Attribute(m_parent, attr, value);
}

bool Attributes::hasAttribute(std::string attr) const
{
    const auto itor = findFirst(attr.c_str());
    return itor != end();
}
