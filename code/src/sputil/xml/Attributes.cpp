/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Attributes.cpp - description                           ║
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

#include <sptk5/cxml>

using namespace sptk;
using namespace sptk::xml;

Attribute::Attribute(Element* parent, const char* tagname, Variant avalue)
: NamedItem(*parent->document())
{
    NamedItem::name(tagname);
    Attribute::value(avalue.asString());
    parent->attributes().push_back(this);
}

Attribute::Attribute(Element* parent, const String& tagname, Variant avalue)
: NamedItem(*parent->document())
{
    NamedItem::name(tagname);
    Attribute::value(avalue.asString());
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
void Attribute::value(const String& new_value)
{
    m_value = new_value;
}

/// @brief Sets new value to node
/// @param new_value const char *, value to set
/// @see value()
void Attribute::value(const char* new_value)
{
    m_value = new_value;
}

Attributes& Attributes::operator=(const Attributes& s)
{
    if (&s == this)
        return *this;
    clear();
    for (auto* node: s)
        new Attribute(m_parent, node->name(), node->value());
    return *this;
}

Attribute* Attributes::getAttributeNode(const String& attr)
{
    const auto itor = findFirst(attr.c_str());
    if (itor != end())
        return (Attribute*) *itor;
    return nullptr;
}

const Attribute* Attributes::getAttributeNode(const String& attr) const
{
    const auto itor = findFirst(attr.c_str());
    if (itor != end())
        return (const Attribute*) *itor;
    return nullptr;
}

Variant Attributes::getAttribute(const String& attr, const char* defaultValue) const
{
    const auto itor = findFirst(attr.c_str());
    if (itor != end())
        return (*itor)->value();
    Variant rc;
    if (defaultValue != nullptr)
        rc = defaultValue;
    return rc;
}

void Attributes::setAttribute(const String& attr, Variant value, const char* defaultValue)
{
    const auto itor = findFirst(attr);
    if (defaultValue != nullptr && value.asString() == defaultValue) {
        if (itor != end()) {
            delete *itor;
            erase(itor);
        }
        return;
    }
    if (itor != end())
        (*itor)->value(String(value));
    else
        new Attribute(m_parent, attr, value);
}

bool Attributes::hasAttribute(const String& attr) const
{
    const auto itor = findFirst(attr.c_str());
    return itor != end();
}
