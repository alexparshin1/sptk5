/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/wsdl/WSRestriction.h>

#include <utility>

using namespace std;
using namespace sptk;

WSRestriction::WSRestriction(String typeName, const xdoc::SNode& simpleTypeElement)
    : m_wsdlTypeName(std::move(typeName))
{
    for (auto enumerationNodes = simpleTypeElement->select("xsd:restriction/xsd:enumeration");
         const auto& enumerationNode: enumerationNodes)
    {
        if (enumerationNode != nullptr)
        {
            m_enumeration.push_back(enumerationNode->attributes().get("value"));
        }
    }

    if (!m_enumeration.empty())
    {
        m_type = Type::Enumeration;
    }
    else
    {
        auto patternNodes = simpleTypeElement->select("xsd:restriction/xsd:pattern");
        for (const auto& patternNode: patternNodes)
        {
            const String pattern = patternNode->attributes().get("value").replace(R"(\\)", R"(\)");
            if (!pattern.empty())
            {
                m_type = Type::Pattern;
            }
            m_patterns.emplace_back(pattern);
        }
    }
}

WSRestriction::WSRestriction(Type type, String wsdlTypeName, const Strings& enumerationsOrPatterns)
    : m_type(type)
    , m_wsdlTypeName(std::move(wsdlTypeName))
{
    if (enumerationsOrPatterns.empty())
    {
        m_type = Type::Unknown;
        return;
    }

    if (type == Type::Enumeration)
    {
        m_enumeration = enumerationsOrPatterns;
    }
    else if (type == Type::Pattern)
    {
        for (const auto& pattern: enumerationsOrPatterns)
        {
            m_patterns.emplace_back(pattern);
        }
    }
}

void WSRestriction::check(const String& typeName, const String& value) const
{
    if (m_type == Type::Enumeration)
    {
        if (m_enumeration.indexOf(value) >= 0)
        {
            return;
        }
    }
    else if (m_type == Type::Pattern)
    {
        for (const auto& regex: m_patterns)
        {
            if (regex.matches(value))
            {
                return;
            }
        }
    }
    else
    {
        return;
    }

    throw Exception("The value '" + value + "' is invalid for restriction on element " + typeName);
}

String sptk::WSRestriction::generateConstructor(const String& variableName) const
{
    stringstream str;
    Strings patterns;

    switch (m_type)
    {
        case Type::Enumeration:
            str << "WSRestriction " << variableName << "(WSRestriction::Type::Enumeration, \"" << m_wsdlTypeName
                << "\", "
                << "{ \"" << m_enumeration.join("\", \"") << "\" })";
            break;
        case Type::Pattern:
            for (const auto& regex: m_patterns)
            {
                patterns.push_back(regex.pattern());
            }
            str << "WSRestriction " << variableName << "(WSRestriction::Type::Pattern, \"" << m_wsdlTypeName << "\", "
                << "{ R\"(" << patterns.join(")\", R\"(") << ")\" })";
            break;
        default:
            break;
    }

    return str.str();
}

WSRestriction::Type WSRestriction::type() const
{
    return m_type;
}
