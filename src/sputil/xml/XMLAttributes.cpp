/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       XMLAttributes.cpp - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

XMLAttribute::XMLAttribute(XMLElement* parent, const char* tagname, XMLValue avalue) :
    XMLNamedItem(*parent->document())
{
    name(tagname);
    value(avalue);
    parent->attributes().push_back(this);
}

XMLAttribute::XMLAttribute(XMLElement* parent, const std::string& tagname, XMLValue avalue) :
    XMLNamedItem(*parent->document())
{
    name(tagname);
    value(avalue);
    parent->attributes().push_back(this);
}

/// @brief Returns the value of the node
const std::string& XMLAttribute::value() const
{
    return m_value;
}

/// @brief Sets new value to node.
/// @param new_value const std::string &, new value
/// @see value()
void XMLAttribute::value(const std::string &new_value)
{
    m_value = new_value;
}

/// @brief Sets new value to node
/// @param new_value const char *, value to set
/// @see value()
void XMLAttribute::value(const char *new_value)
{
    m_value = new_value;
}

XMLAttributes& XMLAttributes::operator =(const XMLAttributes& s)
{
    clear();
    for (auto node: s)
        new XMLAttribute(m_parent, node->name(), node->value());
    return *this;
}

XMLAttribute* XMLAttributes::getAttributeNode(std::string attr)
{
    auto itor = findFirst(attr.c_str());
    if (itor != end())
        return (XMLAttribute*) *itor;
    return nullptr;
}

const XMLAttribute* XMLAttributes::getAttributeNode(std::string attr) const
{
    auto itor = findFirst(attr.c_str());
    if (itor != end())
        return (const XMLAttribute*) *itor;
    return nullptr;
}

XMLValue XMLAttributes::getAttribute(std::string attr, const char *defaultValue) const
{
    auto itor = findFirst(attr.c_str());
    if (itor != end())
        return (*itor)->value();
    XMLValue rc;
    if (defaultValue != nullptr)
        rc = defaultValue;
    return rc;
}

void XMLAttributes::setAttribute(std::string attr, XMLValue value, const char *defaultValue)
{
    auto itor = findFirst(attr);
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
        new XMLAttribute(m_parent, attr, value);
}

bool XMLAttributes::hasAttribute(std::string attr) const
{
    auto itor = findFirst(attr.c_str());
    return itor != end();
}
