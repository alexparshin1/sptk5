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

#include <sptk5/wsdl/WSRestriction.h>
#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;

WSRestriction::WSRestriction(const String& typeName, xdoc::SNode& simpleTypeElement)
    : m_wsdlTypeName(typeName)
{
    xdoc::Node::Vector enumerationNodes;
    simpleTypeElement->select(enumerationNodes, "xsd:restriction/xsd:enumeration");
    for (const auto& enumerationNode: enumerationNodes)
    {
        if (enumerationNode != nullptr)
        {
            m_enumeration.push_back(enumerationNode->getAttribute("value"));
        }
    }

    if (!m_enumeration.empty())
    {
        m_type = Type::Enumeration;
    }
    else
    {
        xdoc::Node::Vector patternNodes;
        simpleTypeElement->select(patternNodes, "xsd:restriction/xsd:pattern");
        for (auto& patternNode: patternNodes)
        {
            String pattern = patternNode->getAttribute("value").replace(R"(\\)", R"(\)");
            if (!pattern.empty())
            {
                m_type = Type::Pattern;
            }
            m_patterns.emplace_back(pattern);
        }
    }
}

WSRestriction::WSRestriction(Type type, const String& wsdlTypeName, const Strings& enumerationsOrPatterns)
    : m_type(type), m_wsdlTypeName(wsdlTypeName)
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
        for (auto& pattern: enumerationsOrPatterns)
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
            for (auto& regex: m_patterns)
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

#if USE_GTEST

static const String coloursXML {
    "<xsd:element name=\"Colours\">"
    "<xsd:simpleType>"
    "<xsd:restriction base=\"xsd:string\">"
    "<xsd:enumeration value=\"Red\"/>"
    "<xsd:enumeration value=\"Green\"/>"
    "<xsd:enumeration value=\"Blue\"/>"
    "</xsd:restriction>"
    "</xsd:simpleType>"
    "</xsd:element>"
};

static const String initialsXML {
    "<xsd:element name=\"Initials\">"
    "<xsd:simpleType>"
    "<xsd:restriction base=\"xsd:string\">"
    "<xsd:pattern value=\"[A-Z][A-Z]\"/>"
    "</xsd:restriction>"
    "</xsd:simpleType>"
    "</xsd:element>"
};

TEST(SPTK_WSRestriction, parseEnumeration)
{
    xdoc::Document document;
    document.load(xdoc::DataFormat::XML, coloursXML);

    auto simpleTypeElement = document.root()->findFirst("xsd:simpleType");

    WSRestriction restrictions("Colours", simpleTypeElement);

    try
    {
        restrictions.check("Colour", "Green");
    }
    catch (const Exception&)
    {
        FAIL() << "Green is allowed colour!";
    }

    try
    {
        restrictions.check("Colour", "Yellow");
        FAIL() << "Yellow is not allowed colour!";
    }
    catch (const Exception&)
    {
        SUCCEED() << "Correctly detected not allowed colour";
    }
}

TEST(SPTK_WSRestriction, parseInitials)
{
    xdoc::Document document;
    document.load(xdoc::DataFormat::XML, initialsXML);

    auto simpleTypeElement = document.root()->findFirst("xsd:simpleType");

    WSRestriction restrictions("Initials", simpleTypeElement);

    try
    {
        restrictions.check("Initials", "AL");
    }
    catch (const Exception&)
    {
        FAIL() << "AL is correct initials!";
    }

    try
    {
        restrictions.check("Initials", "xY");
        FAIL() << "xY is incorrect initials!";
    }
    catch (const Exception&)
    {
        SUCCEED() << "Correctly detected incorrect initials";
    }
}

#endif
