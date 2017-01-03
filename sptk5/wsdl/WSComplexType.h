/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSComplexType.h - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_WSCOMPLEXTYPE_H__
#define __SPTK_WSCOMPLEXTYPE_H__

#include <sptk5/cxml>
#include <sptk5/Variant.h>
#include <sptk5/FieldList.h>
#include <sptk5/db/QueryParameterList.h>
#include <sptk5/xml/XMLElement.h>

namespace sptk {

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * @brief Base type for all user WSDL types
 */
class WSComplexType
{
protected:
   /**
    * WSDL element name
    */
   std::string  m_name;

   /**
    * Element optionality flag
    */
   bool         m_optional;

   /**
    * Is data loaded flage
    */
   bool         m_loaded;

public:
   /**
    * @brief Default constructor
    * @param name const char*, Element name
    * @param optional bool, Element optionality flag
    */
    WSComplexType(const char* name, bool optional=false) : m_name(name), m_optional(optional), m_loaded(false) {}

   /**
    * @brief Destructor
    */
   virtual ~WSComplexType() {}

   /**
    * @brief Copy data from other object
    * @brief other const WSComplexType&, Object to copy from
    */
   void copyFrom(const WSComplexType& other);

   /**
    * @brief Load data from XML node
    * @param input const XMLElement*, XML node containing CAddHandler data
    */
   virtual void load(const XMLElement* input) THROWS_EXCEPTIONS = 0;

   /**
    * Load data from FieldList
    *
    * Only simple WSDL type members are loaded.
    * @param input const sptk::FieldList&, query field list containing CMqType data
    */
   virtual void load(const sptk::FieldList& input) THROWS_EXCEPTIONS = 0;

   /**
    * @brief Unload data to existing XML node
    * @param output XMLElement*, existing XML node
    */
   virtual void unload(XMLElement* output) const THROWS_EXCEPTIONS = 0;

   /**
    * Unload data to Query's parameters
    * @param output QueryParameterList&, query parameters
    */
   virtual void unload(QueryParameterList& output) const THROWS_EXCEPTIONS = 0;

   /**
    * @brief Unload data to new XML node
    * @param parent XMLElement*, parent XML node where new node is created
    */
   virtual void addElement(XMLElement* parent) const THROWS_EXCEPTIONS;

   /**
    * @brief True is data was loaded
    */
   virtual bool isNull() const
   {
       return !m_loaded;
   }

   /**
    * @brief Returns element name
    */
   std::string name() const
   {
       return m_name;
   }

   /**
    * @brief True is element is optional
    */
   virtual bool isOptional() const
   {
       return m_optional;
   }
};

/**
 * @}
 */

}
#endif
