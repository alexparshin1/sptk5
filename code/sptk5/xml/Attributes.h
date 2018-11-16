/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Attributes.h - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_XML_ATTRIBUTES_H__
#define __SPTK_XML_ATTRIBUTES_H__

#include <sptk5/DateTime.h>
#include <sptk5/xml/Node.h>
#include <sptk5/xml/Element.h>
#include <sptk5/xml/NodeList.h>

#define XML_ATTRIBUTE_IS_NODE

namespace sptk {
namespace xml {

/**
 * @addtogroup XML
 * @{
 */

class Element;

/**
 * @brief XML attribute is just a named item
 */
class Attribute : public NamedItem
{
    friend class Attributes;

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
     * @param parent xml::Element*, parent element (can't be NULL)
     * @param name const std::string&, attribute name
     * @param value XMLValue, attribute value
     */
    Attribute(Element* parent, const std::string& name, Value value);

    /**
     * @brief Protected constructor (internal)
     *
     * Creates a new attribute. 
     * Doesn't verify if the attribute name already exists in parent element
     * @param parent xml::Element*, parent element (can't be NULL)
     * @param name const char*, attribute name
     * @param value XMLValue, attribute value
     */
    Attribute(Element* parent, const char* name, Value value);

public:
    /**
     * @brief Returns the value of the node
     */
    const String& value() const noexcept override;

    /**
     * @brief Sets new value to node.
     * @param new_value const std::string &, new value
     * @see value()
     */
    void value(const std::string& new_value) override;

    /**
     * @brief Sets new value to node
     * @param new_value const char *, value to set
     * @see value()
     */
    void value(const char* new_value) override;
};

class Node;

class Document;

/**
 * @brief XML node attributes
 *
 * The XMLAttributes class is map for node attributes.
 */
class SP_EXPORT Attributes : public NodeList
{
    friend class Node;
    friend class Element;

protected:

    /**
     * The parent XML element
     */
    Element* m_parent;

public:

    /**
     * @brief Constructor
     *
     * The XML attributes object uses the shared strings table (SST) for attribute names
     * @param parent xml::Element*, the parent XML element
     */
    explicit Attributes(Element* parent) noexcept
    : m_parent(parent)
    {
    }

    /**
     * @brief Assign operator
     *
     * Makes copy of an attribute set to another.
     * @param src as copy source
     */
    Attributes& operator=(const Attributes& src);

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
    Value getAttribute(std::string attr, const char* defaultValue = "") const;

    /**
     * @brief Sets attribute value for given attribute
     *
     * @param attr std::string, name of attribute
     * @param value XMLValue, an attribute value. See XMLValue class description for data convertions.
     * @param defaultValue const char *, a default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
     */
    void setAttribute(std::string attr, Value value, const char* defaultValue = "");

    /**
     * @brief Returns an attribute node
     *
     * If the attribute is not found, empty string is returned.
     * HTML tags can have empty attributes, for those you should use has_attribute() method.
     * @param attr std::string, name of attribute
     * @returns attribute node or NULL 
     */
    Attribute* getAttributeNode(std::string attr);

    /**
     * @brief Returns an attribute node (const version)
     *
     * If the attribute is not found, empty string is returned.
     * HTML tags can have empty attributes, for those you should use has_attribute() method.
     * @param attr std::string, name of attribute
     * @returns attribute node or NULL 
     */
    const Attribute* getAttributeNode(std::string attr) const;
};
/**
 * @}
 */
}
}
#endif
