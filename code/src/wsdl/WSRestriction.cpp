/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/wsdl/WSRestriction.h>

#include <utility>
#include <sstream> // Included directly, not for this file alone: libstdc++ happens to
                   // reach <sstream> through other standard headers and libc++ does not,
                   // so without it this translation unit fails to compile on FreeBSD and
                   // anywhere else libc++ is the standard library. Do not remove it as
                   // redundant - it is only redundant on one implementation.

using namespace std;
using namespace sptk;

WSRestriction::WSRestriction(String typeName, const xdoc::SNode& simpleTypeElement)
    : m_wsdlTypeName(std::move(typeName))
{
    for (const auto  enumerationNodes = simpleTypeElement->select("xsd:restriction/xsd:enumeration");
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
        for (const auto  patternNodes = simpleTypeElement->select("xsd:restriction/xsd:pattern");
             const auto& patternNode: patternNodes)
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

WSRestriction::WSRestriction(const Type type, String wsdlTypeName, const Strings& enumerationsOrPatterns)
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

String WSRestriction::generateConstructor(const String& variableName) const
{
    stringstream str;
    Strings      patterns;

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
