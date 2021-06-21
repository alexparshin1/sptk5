/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/Buffer.h>
#include <sptk5/Strings.h>
#include <sptk5/DateTime.h>
#include <sptk5/xml/Attributes.h>

#include <string>
#include <map>
#include <vector>

namespace sptk::xml {

/**
 * @addtogroup XML
 * @{
 */

class Document;

/**
 * XML Element is a named item that may optionally have sub-nodes and attributes
 */
class SP_EXPORT Element : public NamedItem
{
    friend class Document;

public:
    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param tagname           Name of XML tag
     */
    Element(Node& parent, const char* tagname)
    : NamedItem(parent,tagname), m_attributes(this)
    {}

    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param tagname           Name of XML tag
     */
    Element(Node* parent, const char* tagname)
    : NamedItem(*parent,tagname), m_attributes(this)
    {}

    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param tagname           Name of XML tag
     */
    Element(Node& parent, const String& tagname)
    : NamedItem(parent,tagname), m_attributes(this)
    {}

    /**
     * Copy constructor
     *
     * @param other             Other object
     */
    Element(const Element&) = delete;

    /**
     * Move constructor
     *
     * @param other             Other object
     */
    Element(Element&&) noexcept = default;

    /**
     * Destructor
     */
    ~Element() override
    {
        Element::clear();
    }

    /**
     * Copy assignment
     *
     * @param other             Other object
     */
    Element& operator = (const Element&) = delete;

    /**
     * Move assignment
     *
     * @param other             Other object
     */
    Element& operator = (Element&&) noexcept = default;

    /**
     * Returns node type
     */
    Type type() const override
    {
        return Type::DOM_ELEMENT;
    }

    /**
     * Returns the first subnode iterator
     */
    iterator begin() override
    {
        return m_nodes.begin();
    }

    /**
     * Returns the first subnode const iterator
     */
    const_iterator begin() const override
    {
        return m_nodes.begin();
    }

    /**
     * Returns the end subnode iterator
     */
    iterator end() override
    {
        return m_nodes.end();
    }

    /**
     * Returns the end subnode const iterator
     */
    const_iterator end() const override
    {
        return m_nodes.end();
    }

    /**
     * Returns a number of subnodes
     */
    uint32_t size() const override
    {
        return (uint32_t) m_nodes.size();
    }

    /**
     * Returns true if node has no subnodes of subnodes
     */
    bool empty() const override
    {
        return m_nodes.empty();
    }

    /**
     * Appends a subnode
     *
     * @param node              Node to append
     */
    void push_back(Node* node) override;

    /**
     * Inserts a subnode
     *
     * @param pos               Insert position with the list of subnodes
     * @param node              Node to insert
     */
    void insert(iterator pos, Node* node) override;

    /**
     * Removes a subnode
     *
     * Any memory allocated for subnode is released and subnode is
     * removed from its parent
     * @param node              Node to remove
     */
    void remove(Node* node) override;

    /**
     * Removes a subnode
     *
     * Disconnects subnode from parent (this node)
     */
    void unlink(Node* node) override;

    /**
     * Deletes all child nodes
     *
     * Any memory, associated with child nodes, is released.
     */
    void clearChildren() override;

    /**
     * Deletes all children and clears all the attributes
     *
     * Any memory, associated with children or attributes,
     * is released.
     */
    void clear() override;

    /**
     * Returns referrence to node attributes
     */
    Attributes& attributes() override
    {
        return m_attributes;
    }

    /**
     * Returns referrence to node attributes (const version)
     */
    const Attributes& attributes() const override
    {
        return m_attributes;
    }

    /**
     * Returns true, if node has any attributes
     */
    bool hasAttributes() const override
    {
        return !m_attributes.empty();
    }

    /**
     * Returns true, if given attribute is found
     * @param attr              Attribute to search
     */
    bool hasAttribute(const char *attr) const override
    {
        return m_attributes.hasAttribute(attr);
    }

    /**
     * Returns attribute value for given attribute.
     *
     * HTML tags can have empty attributes, for those you should use has_attribute() method.
     * @param attr              Name of attribute
     * @param defaultValue      Default value. If attribute doesn't exist then default value is returned.
     * @returns attribute value
     */
    Variant getAttribute(const String& attr, const char *defaultValue="") const override
    {
        return m_attributes.getAttribute(attr,defaultValue);
    }

    /**
     * Sets new value to attribute 'attr'.
     *
     * If attribute is not found, it's added to map.
     * @param attr              Attribute name
     * @param value             Attribute value
     * @param defaultValue      Default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
     */
    void setAttribute(const String& attr, const Variant& value, const char *defaultValue="") override
    {
        m_attributes.setAttribute(attr, value, defaultValue);
    }

protected:

    /**
     * Protected constructor for creating Doc only
     *
     * @param doc               XML document.
     */
    explicit Element(Document& doc)
    : NamedItem(doc), m_attributes(this)
    {}

private:

    NodeList       m_nodes;         ///< The list of subnodes
    Attributes     m_attributes;    ///< Node attributes
};

/**
 * XML processing instructions (PI)
 */
class SP_EXPORT PI : public Element
{
public:

    using Element::Element;

    /**
     * Returns node type
     */
    Type type() const override
    {
        return Type::DOM_PI;
    }
};

/**
 * @}
 */
}
