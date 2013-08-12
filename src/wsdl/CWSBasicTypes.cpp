/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSBasicTypes.cpp  -  description
                             -------------------
    begin                : 03 Aug 2012
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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
#include <sptk5/CRegExp.h>

using namespace std;
using namespace sptk;

void WSBool::load(const CXmlNode* attr)
{
    m_data.setBool(attr->text() == "true");
}

std::string WSBool::asString() const
{
    return m_data.getBool() ? "true" : "false";
}

void WSDate::load(const CXmlNode* attr)
{
    m_data.setDate(CDateTime(attr->text().c_str()));
}

std::string WSDate::asString() const
{
    return m_data.asString();
}

void WSDateTime::load(const CXmlNode* attr)
{
    static CRegExp getDate("^([\\d\\-]+)T(.*)Z.*$");
    string dateString = getDate.s(attr->text(),"\\1 \\2");
    m_data.setDateTime(CDateTime(dateString.c_str()));
}

std::string WSDateTime::asString() const
{
    static CRegExp getDate("^([\\d\\-]+) (.*)$");
    return getDate.s(m_data.asString(),"\\1T\\2Z");
}

void WSDouble::load(const CXmlNode* attr)
{
    m_data.setFloat(atof(attr->text().c_str()));
}

void WSInteger::load(const CXmlNode* attr)
{
    m_data.setInteger(atoi(attr->text().c_str()));
}
