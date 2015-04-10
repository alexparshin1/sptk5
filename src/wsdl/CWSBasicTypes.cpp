/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSBasicTypes.cpp  -  description
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

#include <sptk5/wsdl/CWSBasicTypes.h>

using namespace std;
using namespace sptk;

CXmlElement* WSBasicType::addElement(CXmlElement* parent) const
{
    string text(asString());
    if (m_optional && (isNull() || text.empty()))
        return NULL;
    CXmlElement* element = new CXmlElement(*parent, m_name);
    element->text(text);
    return element;
}

void WSString::load(const CXmlNode* attr)
{
    setString(attr->text());
}

void WSString::load(std::string attr)
{
    setString(attr);
}

void WSBool::load(const CXmlNode* attr)
{
    setBool(attr->text() == "true");
}

void WSBool::load(string attr)
{
    setBool(attr == "true");
}

void WSDate::load(const CXmlNode* attr)
{
    setDate(CDateTime(attr->text().c_str()));
}

void WSDate::load(string attr)
{
    setDate(CDateTime(attr.c_str()));
}

void WSDateTime::load(const CXmlNode* attr)
{
    setDateTime(CDateTime(attr->text().c_str()));
}

void WSDateTime::load(string attr)
{
    setDateTime(CDateTime(attr.c_str()));
}

string WSDateTime::asString() const
{
    CDateTime dt = asDateTime();
    return dt.dateString(true) + "T" + dt.timeString(true,true);
}

void WSDouble::load(const CXmlNode* attr)
{
    setFloat(atof(attr->text().c_str()));
}

void WSDouble::load(string attr)
{
    setFloat(atof(attr.c_str()));
}

void WSInteger::load(const CXmlNode* attr)
{
    setInteger(atoi(attr->text().c_str()));
}

void WSInteger::load(string attr)
{
    setInteger(atoi(attr.c_str()));
}
