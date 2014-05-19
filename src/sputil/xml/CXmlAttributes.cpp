/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXmlNode.cpp  -  description
                             -------------------
    begin                : Wed June 21 2006
    based on the code    : Mikko Lahteenmaki <Laza@Flashmail.com>
    copyright            : (C) 1999-2014 by Alexey S.Parshin
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

#include <sptk5/cxml>

using namespace sptk;

CXmlAttribute::CXmlAttribute(CXmlElement* parent, const char* tagname, CXmlValue avalue) :
    CXmlNamedItem(*parent->document())
{
    name(tagname);
    value(avalue);
    parent->attributes().push_back(this);
}

CXmlAttribute::CXmlAttribute(CXmlElement* parent, const std::string& tagname, CXmlValue avalue) :
    CXmlNamedItem(*parent->document())
{
    name(tagname);
    value(avalue);
    parent->attributes().push_back(this);
}

/// @brief Returns the value of the node
const std::string& CXmlAttribute::value() const
{
    return m_value;
}

/// @brief Sets new value to node.
/// @param new_value const std::string &, new value
/// @see value()
void CXmlAttribute::value(const std::string &new_value)
{
    m_value = new_value;
}

/// @brief Sets new value to node
/// @param new_value const char *, value to set
/// @see value()
void CXmlAttribute::value(const char *new_value)
{
    m_value = new_value;
}

CXmlAttributes& CXmlAttributes::operator =(const CXmlAttributes& s)
{
    clear();
    for (CXmlAttributes::const_iterator it = s.begin(); it != s.end(); it++) {
        CXmlNode* node = *it;
        new CXmlAttribute(m_parent, node->name(), node->value());
    }
    return *this;
}

CXmlAttribute* CXmlAttributes::getAttributeNode(std::string attr)
{
    iterator itor = findFirst(attr.c_str());
    if (itor != end())
        return (CXmlAttribute*) *itor;
    return NULL;
}

const CXmlAttribute* CXmlAttributes::getAttributeNode(std::string attr) const
{
    const_iterator itor = findFirst(attr.c_str());
    if (itor != end())
        return (const CXmlAttribute*) *itor;
    return NULL;
}

CXmlValue CXmlAttributes::getAttribute(std::string attr, const char *defaultValue) const
{
    const_iterator itor = findFirst(attr.c_str());
    if (itor != end())
        return (*itor)->value();
    CXmlValue rc;
    if (defaultValue)
        rc = defaultValue;
    return rc;
}

void CXmlAttributes::setAttribute(std::string attr, CXmlValue value, const char *defaultValue)
{
    iterator itor = findFirst(attr.c_str());
    if (defaultValue && value.str() == defaultValue) {
        if (itor != end()) {
            delete *itor;
            erase(itor);
        }
        return;
    }
    if (itor != end())
        (*itor)->value(value);
    else
        new CXmlAttribute(m_parent, attr, value);
}

bool CXmlAttributes::hasAttribute(std::string attr) const
{
    const_iterator itor = findFirst(attr.c_str());
    return itor != end();
}
