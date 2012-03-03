/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXmlNode.h  -  description
                             -------------------
    begin                : Wed June 21 2006
    based on the code    : Mikko Lahteenmaki <Laza@Flashmail.com>
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CXMLATTRIBUTES_H__
#define __CXMLATTRIBUTES_H__

//#include <sptk5/cxml>
#include <sptk5/CDateTime.h>
#include <sptk5/xml/CXmlNode.h>
#include <sptk5/xml/CXmlElement.h>
#include <sptk5/xml/CXmlNodeList.h>

#define XML_ATTRIBUTE_IS_NODE

namespace sptk {

/// @addtogroup XML
/// @{

class CXmlElement;

/// @brief XML attribute is just a named item
class CXmlAttribute: public CXmlNamedItem
{
    friend class CXmlAttributes;
protected:
    std::string m_value;  ///< Attribute value

    /// @brief Protected constructor (internal)
    ///
    /// Creates a new attribute. 
    /// Doesn't verify if the attribute name already exists in parent element
    /// @param parent CXmlElement*, parent element (can't be NULL)
    /// @param name const std::string&, attribute name
    /// @param value CXmlValue, attribute value
    CXmlAttribute(CXmlElement* parent, const std::string& name, CXmlValue value);

    /// @brief Protected constructor (internal)
    ///
    /// Creates a new attribute. 
    /// Doesn't verify if the attribute name already exists in parent element
    /// @param parent CXmlElement*, parent element (can't be NULL)
    /// @param name const char*, attribute name
    /// @param value CXmlValue, attribute value
    CXmlAttribute(CXmlElement* parent, const char* name, CXmlValue value);

public:
    /// @brief Returns the value of the node
    virtual const std::string& value() const
    {
        return m_value;
    }

    /// @brief Sets new value to node.
    /// @param new_value const std::string &, new value
    /// @see value()
    virtual void value(const std::string &new_value)
    {
        m_value = new_value;
    }

    /// @brief Sets new value to node
    /// @param new_value const char *, value to set
    /// @see value()
    virtual void value(const char *new_value)
    {
        m_value = new_value;
    }
};

class CXmlNode;
class CXmlDoc;
/*
/// @brief Custom compare attributes method
struct less_attr : public std::binary_function<const char*,const char*,bool> {
    /// @brief Compares two strings
    /// @returns true if s1 < s2
    bool operator() (const char* s1,const char* s2) const {
        return strcmp(s1,s2) < 0;
    }
};

/// @brief The map of character strings to attributes
///
/// Designed to be used together with SST (Shared Strings Table)
typedef std::map<const char*,CXmlAttribute,less_attr> CXmlAttributeMap;
*/

/// @brief XML node attributes
///
/// The CXmlAttributes class is map for node attributes.
class SP_EXPORT CXmlAttributes: public CXmlNodeList
{
    friend class CXmlNode;
    friend class CXmlElement;
protected:
    CXmlElement* m_parent;    ///< The parent XML element

    /// @brief Returns an attribute node
    ///
    /// If the attribute is not found, empty string is returned.
    /// HTML tags can have empty attributes, for those you should use has_attribute() method.
    /// @param attr std::string, name of attribute
    /// @returns attribute node or NULL 
    CXmlAttribute* getAttributeNode(std::string attr);

    /// @brief Returns an attribute node (const version)
    ///
    /// If the attribute is not found, empty string is returned.
    /// HTML tags can have empty attributes, for those you should use has_attribute() method.
    /// @param attr std::string, name of attribute
    /// @returns attribute node or NULL 
    const CXmlAttribute* getAttributeNode(std::string attr) const;

public:

    /// @brief Constructor
    ///
    /// The XML attributes object uses the shared strings table (SST) for attribute names
    /// @param parent CXmlElement*, the parent XML element
    CXmlAttributes(CXmlElement* parent) :
            m_parent(parent)
    {
    }

    /// @brief Assign operator
    ///
    /// Makes copy of an attribute set to another.
    /// @param src as copy source
    CXmlAttributes& operator =(const CXmlAttributes& src);

    /// @brief Searches for named attribute
    ///
    /// Returns true, if given attribute is found.
    /// @param attr std::string, name of attribute to search
    bool hasAttribute(std::string attr) const;

    /// @brief Returns an attribute value
    ///
    /// If the attribute is not found, empty string is returned.
    /// HTML tags can have empty attributes, for those you should use has_attribute() method.
    /// @param attr std::string, name of attribute
    /// @param defaultValue const char *, a default value. If attribute doesn't exist then default value is returned.
    /// @returns attribute value 
    CXmlValue getAttribute(std::string attr, const char *defaultValue = "") const;

    /// @brief Sets attribute value for given attribute
    ///
    /// @param attr std::string, name of attribute
    /// @param value CXmlValue, an attribute value. See CXmlValue class description for data convertions.
    /// @param defaultValue const char *, a default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
    void setAttribute(std::string attr, CXmlValue value, const char *defaultValue = "");
};
/// @}
}
#endif
