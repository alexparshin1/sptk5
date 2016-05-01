/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSTypeTranslator.cpp  -  description
                             -------------------
    begin                : 03 Aug 2012
    copyright            : (C) 1999-2016 by Alexey Parshin. All rights reserved.
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

#include <sptk5/wsdl/CWSTypeTranslator.h>

using namespace std;
using namespace sptk;

namespace sptk {
    CWSTypeTranslator wsTypeTranslator;
}

CWSTypeTranslator::CWSTypeTranslator()
{
    wsTypeToCxxTypeMap["xsd:boolean"]   = "sptk::WSBool";
    wsTypeToCxxTypeMap["xsd:date"]      = "sptk::WSDate";
    wsTypeToCxxTypeMap["xsd:dateTime"]  = "sptk::WSDateTime";
    wsTypeToCxxTypeMap["xsd:double"]    = "sptk::WSDouble";
    wsTypeToCxxTypeMap["xsd:float"]     = "sptk::WSDouble";
    wsTypeToCxxTypeMap["xsd:int"]       = "sptk::WSInteger";
    wsTypeToCxxTypeMap["xsd:string"]    = "sptk::WSString";
    wsTypeToCxxTypeMap["xsd:time"]      = "sptk::WSTime";
}

std::string CWSTypeTranslator::toCxxType(std::string wsType,std::string defaultType) const
{
    std::map<std::string,std::string>::const_iterator itor = wsTypeToCxxTypeMap.find(wsType);
    if (itor == wsTypeToCxxTypeMap.end())
        return defaultType;
    return itor->second;
}
