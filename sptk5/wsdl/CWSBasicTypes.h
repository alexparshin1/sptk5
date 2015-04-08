/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSBasicTypes.h  -  description
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

#ifndef __CWSBASICTYPES_H__
#define __CWSBASICTYPES_H__

#include <sptk5/cxml>
#include <sptk5/CField.h>
#include <sptk5/xml/CXmlElement.h>

namespace sptk {

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief Base type for all standard WSDL types
class WSString : public CField
{
protected:
    bool m_optional;    ///< Element optionality flag

public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    WSString(const char* name) : CField(name), m_optional(false) {}

    /// @brief Sets optionality flag
    /// @param opt bool, Element optionality flag
    void optional(bool opt) { m_optional = opt; }

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);

    /// @brief Adds an element to response XML with this object data
    /// @param parent CXmlElement*, Parent XML element
    /// @param name std::string, New element tag name
    CXmlElement* addElement(CXmlElement* parent, std::string name) const;
};

/// @brief Wrapper for WSDL bool type
class WSBool : public WSString
{
public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    WSBool(const char* name) : WSString(name) {}

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);
};

/// @brief Wrapper for WSDL date type
class WSDate : public WSString
{
public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    WSDate(const char* name) : WSString(name) {}

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);
};

/// @brief Wrapper for WSDL dateTime type
class WSDateTime : public WSString
{
public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    WSDateTime(const char* name) : WSString(name) {}

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);

    /// @brief Better (than in base class) conversion method
    virtual std::string asString() const THROWS_EXCEPTIONS;
};

/// @brief Wrapper for WSDL double type
class WSDouble : public WSString
{
public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    WSDouble(const char* name) : WSString(name) {}

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);
};

/// @brief Wrapper for WSDL int type
class WSInteger : public WSString
{
public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    WSInteger(const char* name) : WSString(name) {}

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);
};

/// @}

}
#endif
