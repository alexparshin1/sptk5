/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSRestriction.cpp  -  description
                             -------------------
    begin                : 03 Aug 2012
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/wsdl/CWSRestriction.h>
#include <sstream>

using namespace std;
using namespace sptk;

WSRestriction::WSRestriction(string typeName, CXmlNode* simpleTypeElement)
: m_typeName(typeName)
{
    CXmlNodeVector enumerationNodes;
    simpleTypeElement->select(enumerationNodes, "xsd:restriction/xsd:enumeration");
    for (CXmlNode::iterator itor = enumerationNodes.begin(); itor != enumerationNodes.end(); itor++) {
        CXmlElement* enumerationNode = dynamic_cast<CXmlElement*>(*itor);
        if (enumerationNode)
            m_enumerations.push_back(enumerationNode->getAttribute("value").c_str());
    }
}

WSRestriction::WSRestriction(string typeName, string enumerations, const char* delimiter)
: m_typeName(typeName), m_enumerations(enumerations, delimiter)
{
}

void WSRestriction::check(std::string value)
{
    if (!m_enumerations.empty() && m_enumerations.indexOf(value) == -1)
        throw CException("value '" + value + "' is invalid for restriction on " + m_typeName);
}

string sptk::WSRestriction::generateConstructor(std::string variableName) const
{
    stringstream str;
    str << "WSRestriction " << variableName << "(\"" << m_typeName << "\", "
        << "\"" << m_enumerations.asString("|") << "\", \"|\")";
    return str.str();
}

