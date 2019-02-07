/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Attributes.h - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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
#include <sptk5/Variant.h>
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
 * XML attribute is just a named item
 */
class Attribute : public NamedItem
{
    friend class Attributes;

    /**
     * Attribute value
     */
    String m_value;

protected:


    /**
     * Protected constructor (internal)
     *
     * Creates a new attribute. 
     * Doesn't verify if the attribute name already exists in parent element
     * @param parent            Parent element (can't be NULL)
     * @param name              Attribute name
     * @param value             Attribute value
     */
    Attribute(Element* parent, const String& name, Variant value);

    /**
     * Protected constructor (internal)
     *
     * Creates a new attribute. 
     * Doesn't verify if the attribute name already exists in parent element
     * @param parent            Parent element (can't be NULL)
     * @param name              Attribute name
     * @param value             Attribute value
     */
    Attribute(Element* parent, const char* name, Variant value);

public:
    /**
     * Returns the value of the node
     */
    const String& value() const noexcept override;

    /**
     * Sets new value to node.
     * @param new_value         New value
     * @see value()
     */
    void value(const String& new_value) override;

    /**
     * Sets new value to node
     * @param new_value         Value to set
     * @see value()
     */
    void value(const char* new_value) override;
};

class Node;

class Document;

/**
 * XML node attributes
 *
 * The XMLAttributes class is map for node attributes.
 */
class SP_EXPORT Attributes : public NodeList
{
    friend class Node;
    friend class Element;

    /**
     * The parent XML element
     */
    Element* m_parent;

public:

    /**
     * Constructor
     *
     * The XML attributes object uses the shared strings table (SST) for attribute names
     * @param parent            Parent XML element
     */
    explicit Attributes(Element* parent) noexcept
    : m_parent(parent)
    {
    }

    /**
     * Assign operator
     *
     * Makes copy of an attribute set to another.
     * @param src as copy source
     */
    Attributes& operator=(const Attributes& src);

    /**
     * Searches for named attribute
     *
     * Returns true, if given attribute is found.
     * @param attr              Name of attribute to search
     */
    bool hasAttribute(const String& attr) const;

    /**
     * Returns an attribute value
     *
     * If the attribute is not found, empty string is returned.
     * HTML tags can have empty attributes, for those you should use has_attribute() method.
     * @param attr              Name of attribute
     * @param defaultValue      Default value. If attribute doesn't exist then default value is returned.
     * @returns attribute value 
     */
    Variant getAttribute(const String& attr, const char* defaultValue = "") const;

    /**
     * Sets attribute value for given attribute
     *
     * @param attr std::string, name of attribute
     * @param value             Attribute value
     * @param defaultValue      Default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
     */
    void setAttribute(const String& attr, Variant value, const char* defaultValue = "");

    /**
     * Returns an attribute node
     *
     * If the attribute is not found, empty string is returned.
     * HTML tags can have empty attributes, for those you should use has_attribute() method.
     * @param attr              Name of attribute
     * @returns attribute node or NULL 
     */
    Attribute* getAttributeNode(const String& attr);

    /**
     * Returns an attribute node (const version)
     *
     * If the attribute is not found, empty string is returned.
     * HTML tags can have empty attributes, for those you should use has_attribute() method.
     * @param attr              Name of attribute
     * @returns attribute node or NULL 
     */
    const Attribute* getAttributeNode(const String& attr) const;
};
/**
 * @}
 */
}
}
#endif
