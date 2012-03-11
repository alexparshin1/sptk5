/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXmlValue.cpp  -  description
                             -------------------
    begin                : Wed June 21 2006
    copyright            : (C) 2003-2012 by Alexey S.Parshin
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
#include <stdio.h>

using namespace sptk;

CXmlValue::operator bool() const
{
    if (empty())
        return false;
    char ch = c_str()[0];
    const char *p = strchr("TtYy1", ch);
    return p != 0;
}

CXmlValue& CXmlValue::operator =(bool v)
{
    if (v)
        assign("Y");
    else
        assign("N");
    return *this;
}

CXmlValue& CXmlValue::operator =(int32_t v)
{
    char buff[64];
    sprintf(buff, "%i", v);
    assign(buff);
    return *this;
}

CXmlValue& CXmlValue::operator =(uint32_t v)
{
    char buff[64];
    sprintf(buff, "%u", v);
    assign(buff);
    return *this;
}

CXmlValue& CXmlValue::operator =(int64_t v)
{
    char buff[64];
#if SIZEOF_LONG == 8
    sprintf(buff,"%li",v);
#else
    sprintf(buff, "%lli", v);
#endif
    assign(buff);
    return *this;
}

CXmlValue& CXmlValue::operator =(uint64_t v)
{
    char buff[64];
#if SIZEOF_LONG == 8
    sprintf(buff,"%lu",v);
#else
    sprintf(buff, "%llu", v);
#endif
    assign(buff);
    return *this;
}

CXmlValue& CXmlValue::operator =(double v)
{
    char buff[64];
    sprintf(buff, "%f", v);
    assign(buff);
    return *this;
}
