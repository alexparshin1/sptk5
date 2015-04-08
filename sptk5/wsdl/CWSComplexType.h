/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSComplexType.h  -  description
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

#ifndef __CWSCOMPLEXTYPE_H__
#define __CWSCOMPLEXTYPE_H__

#include <sptk5/cxml>
#include <sptk5/CVariant.h>
#include <sptk5/xml/CXmlElement.h>

namespace sptk {

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief Base type for all user WSDL types
class WSComplexType
{
protected:
   std::string m_name; ///< WSDL element name
public:
   /// @brief Default constructor
   WSComplexType(const char* name) : m_name(name) {}

   /// @brief Destructor
   virtual ~WSComplexType() {}

   /// @brief Copies data from other object
   /// @brief other const WSComplexType&, Object to copy from
   void copyFrom(const WSComplexType& other);

   /// @brief Loads CAddHandler from XML node
   /// @param input const sptk::CXmlElement*, XML node containing CAddHandler data
   virtual void load(const sptk::CXmlElement* input) THROWS_EXCEPTIONS = 0;

   /// @brief Unloads CAddHandler to existing XML node
   /// @param output sptk::CXmlElement*, existing XML node
   virtual void unload(sptk::CXmlElement* output) const THROWS_EXCEPTIONS = 0;

   /// @brief Unloads CAddHandler to new XML node
   /// @param parent sptk::CXmlElement*, parent XML node where new node is created
   /// @param name std::string, new XML node name
   virtual void addElement(sptk::CXmlElement* parent, std::string name) const THROWS_EXCEPTIONS;
};

/// @}

}
#endif
