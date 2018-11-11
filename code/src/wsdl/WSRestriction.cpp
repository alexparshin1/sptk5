/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWSRestriction.cpp - description                       ║
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

#include <sptk5/wsdl/WSRestriction.h>

using namespace std;
using namespace sptk;

WSRestriction::WSRestriction(const string& typeName, xml::Node* simpleTypeElement)
: m_typeName(typeName)
{
    xml::NodeVector enumerationNodes;
    simpleTypeElement->select(enumerationNodes, "xsd:restriction/xsd:enumeration");
    for (auto node: enumerationNodes) {
        auto enumerationNode = dynamic_cast<xml::Element*>(node);
        if (enumerationNode != nullptr)
            m_enumerations.push_back(string(enumerationNode->getAttribute("value").c_str()));
    }
}

WSRestriction::WSRestriction(const string& typeName, const string& enumerations, const char* delimiter)
: m_typeName(typeName), m_enumerations(enumerations, delimiter)
{
}

void WSRestriction::check(const std::string& typeName, const std::string& value) const
{
    if (!m_enumerations.empty() && m_enumerations.indexOf(value) == -1)
        throw Exception("value '" + value + "' is invalid for restriction on " + m_typeName + " for type " + typeName);
}

string sptk::WSRestriction::generateConstructor(const string& variableName) const
{
    stringstream str;
    str << "WSRestriction " << variableName << "(\"" << m_typeName << "\", "
        << "\"" << m_enumerations.asString("|") << "\", \"|\")";
    return str.str();
}

