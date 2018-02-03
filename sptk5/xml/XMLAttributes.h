/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       XMLAttributes.h - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_XMLATTRIBUTES_H__
#define __SPTK_XMLATTRIBUTES_H__

#include <sptk5/DateTime.h>
#include <sptk5/xml/XMLNode.h>
#include <sptk5/xml/XMLElement.h>
#include <sptk5/xml/XMLNodeList.h>

#define XML_ATTRIBUTE_IS_NODE

namespace sptk {

/**
 * @addtogroup XML
 * @{
 */

class XMLElement;

/**
 * @brief XML attribute is just a named item
 */
class XMLAttribute: public XMLNamedItem
{
    friend class XMLAttributes;
protected:
    /**
     * Attribute value
     */
    String m_value;


    /**
     * @brief Protected constructor (internal)
     *
     * Creates a new attribute. 
     * Doesn't verify if the attribute name already exists in parent element
     * @param parent XMLElement*, parent element (can't be NULL)
     * @param name const std::string&, attribute name
     * @param value XMLValue, attribute value
     */
    XMLAttribute(XMLElement* parent, const std::string& name, XMLValue value);

    /**
     * @brief Protected constructor (internal)
     *
     * Creates a new attribute. 
     * Doesn't verify if the attribute name already exists in parent element
     * @param parent XMLElement*, parent element (can't be NULL)
     * @param name const char*, attribute name
     * @param value XMLValue, attribute value
     */
    XMLAttribute(XMLElement* parent, const char* name, XMLValue value);

public:
    /**
     * @brief Returns the value of the node
     */
    virtual const String& value() const;

    /**
     * @brief Sets new value to node.
     * @param new_value const std::string &, new value
     * @see value()
     */
    virtual void value(const std::string &new_value);

    /**
     * @brief Sets new value to node
     * @param new_value const char *, value to set
     * @see value()
     */
    virtual void value(const char *new_value);
};

class XMLNode;
class XMLDocument;

/**
 * @brief XML node attributes
 *
 * The XMLAttributes class is map for node attributes.
 */
class SP_EXPORT XMLAttributes: public XMLNodeList
{
    friend class XMLNode;
    friend class XMLElement;

protected:

    /**
     * The parent XML element
     */
    XMLElement* m_parent;

public:

    /**
     * @brief Constructor
     *
     * The XML attributes object uses the shared strings table (SST) for attribute names
     * @param parent XMLElement*, the parent XML element
     */
    XMLAttributes(XMLElement* parent) :
            m_parent(parent)
    {
    }

    /**
     * @brief Assign operator
     *
     * Makes copy of an attribute set to another.
     * @param src as copy source
     */
    XMLAttributes& operator =(const XMLAttributes& src);

    /**
     * @brief Searches for named attribute
     *
     * Returns true, if given attribute is found.
     * @param attr std::string, name of attribute to search
     */
    bool hasAttribute(std::string attr) const;

    /**
     * @brief Returns an attribute value
     *
     * If the attribute is not found, empty string is returned.
     * HTML tags can have empty attributes, for those you should use has_attribute() method.
     * @param attr std::string, name of attribute
     * @param defaultValue const char *, a default value. If attribute doesn't exist then default value is returned.
     * @returns attribute value 
     */
    XMLValue getAttribute(std::string attr, const char *defaultValue = "") const;

    /**
     * @brief Sets attribute value for given attribute
     *
     * @param attr std::string, name of attribute
     * @param value XMLValue, an attribute value. See XMLValue class description for data convertions.
     * @param defaultValue const char *, a default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
     */
    void setAttribute(std::string attr, XMLValue value, const char *defaultValue = "");

    /**
     * @brief Returns an attribute node
     *
     * If the attribute is not found, empty string is returned.
     * HTML tags can have empty attributes, for those you should use has_attribute() method.
     * @param attr std::string, name of attribute
     * @returns attribute node or NULL 
     */
    XMLAttribute* getAttributeNode(std::string attr);

    /**
     * @brief Returns an attribute node (const version)
     *
     * If the attribute is not found, empty string is returned.
     * HTML tags can have empty attributes, for those you should use has_attribute() method.
     * @param attr std::string, name of attribute
     * @returns attribute node or NULL 
     */
    const XMLAttribute* getAttributeNode(std::string attr) const;
};
/**
 * @}
 */
}
#endif
