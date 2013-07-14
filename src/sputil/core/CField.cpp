/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          cfields.cpp  -  description
                             -------------------
    begin                : Wed Dec 15 1999
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

#include <sptk5/CField.h>
#include <sptk5/CException.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
using namespace sptk;

CField::CField(const char *name) {
   m_name = name;
   displayName = name;
   width = -1;
   flags = 4; // FL_ALIGN_LEFT
   dataSize(0);
   visible = true;
   precision = 3; // default precision, only affects floating point fields
}

string CField::asString() const throw(CException) {
   char print_buffer[32];
   if (m_dataType & VAR_NULL) return "";
   switch (dataType()) {
      case VAR_BOOL:
         if (m_data.intData)
	    return "true";
	 else
	    return "false";
      case VAR_INT:
         sprintf(print_buffer,"%i",m_data.intData);
         return string(print_buffer);
      case VAR_INT64:
#if SIZEOF_LONG == 8
         sprintf(print_buffer,"%li",m_data.int64Data);
#else
         sprintf(print_buffer,"%lli",m_data.int64Data);
#endif
         return string(print_buffer);
      case VAR_FLOAT:
         {
            char formatString[10];
            sprintf(formatString,"%%0.%if",precision);
            sprintf(print_buffer,formatString,m_data.floatData);
            return string(print_buffer);
         }
      case VAR_STRING:
      case VAR_TEXT:
      case VAR_BUFFER:     
         if (!m_data.buffer.data)
            return "";
         return m_data.buffer.data;
      case VAR_DATE:       return CDateTime(m_data.floatData).dateString();
      case VAR_DATE_TIME:   
         {
            CDateTime dt(m_data.floatData);
            return dt.dateString() + " " + dt.timeString();
         }
      case VAR_IMAGE_PTR:
         sprintf(print_buffer,"%p",(char *)m_data.imagePtr);
         return string(print_buffer);
      case VAR_IMAGE_NDX:
         sprintf(print_buffer,"%i",m_data.imageNdx);
         return string(print_buffer);
      default:             
         throw CException("Can't convert field for that type");
   }
}

void CField::toXML(CXmlNode& node,bool compactXmlMode) const {
   string value = asString();
   if (!value.empty()) {
      CXmlElement* element = 0;
      if (dataType() == VAR_TEXT) {
         element = new CXmlElement(node,fieldName());
         new CXmlCDataSection(*element,value);
      } else {
    	 if (compactXmlMode)
            node.setAttribute(fieldName(),value);
         else {
	        element = new CXmlElement(node,"field");
	        element->value(value);
         }
      }
      if (!compactXmlMode) {
         element->setAttribute("name",fieldName());
         element->setAttribute("type",CVariant::typeName(dataType()));
         element->setAttribute("size",int2string((uint32_t)dataSize()));
      }
   }
}
