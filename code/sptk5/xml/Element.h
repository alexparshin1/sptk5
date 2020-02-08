/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Element.h - description                                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_XML_ELEMENT_H__
#define __SPTK_XML_ELEMENT_H__

#include <sptk5/Buffer.h>
#include <sptk5/Strings.h>
#include <sptk5/DateTime.h>
#include <sptk5/xml/Attributes.h>

#include <string>
#include <map>
#include <vector>

namespace sptk {
namespace xml {

/**
 * @addtogroup XML
 * @{
 */

class Document;

/**
 * @brief XML Element is a named item that may optionally have sub-nodes and attributes
 */
class SP_EXPORT Element : public NamedItem
{
    friend class Document;

    /**
     * The list of subnodes
     */
    NodeList       m_nodes;

    /**
     * Node attributes
     */
    Attributes     m_attributes;

protected:

    /**
     * @brief Protected constructor for creating Doc only
     *
     * @param doc               XML document.
     */
    explicit Element(Document& doc)
    : NamedItem(doc), m_attributes(this)
    {}

public:
    /**
     * @brief Constructor
     *
     * @param parent            Parent node.
     * @param tagname           Name of XML tag
     */
    Element(Node& parent, const char* tagname)
    : NamedItem(parent,tagname), m_attributes(this)
    {}

    /**
     * @brief Constructor
     *
     * @param parent            Parent node.
     * @param tagname           Name of XML tag
     */
    Element(Node* parent, const char* tagname)
    : NamedItem(*parent,tagname), m_attributes(this)
    {}

    /**
     * @brief Constructor
     *
     * @param parent            Parent node.
     * @param tagname           Name of XML tag
     */
    Element(Node& parent, const String& tagname)
    : NamedItem(parent,tagname), m_attributes(this)
    {}

    /**
     * Destructor
     */
    ~Element() override
    {
        Element::clear();
    }

    /**
     * @brief Returns node type
     */
    NodeType type() const override
    {
        return DOM_ELEMENT;
    }

    /**
     * @brief Returns the first subnode iterator
     */
    iterator begin() override
    {
        return m_nodes.begin();
    }

    /**
     * @brief Returns the first subnode const iterator
     */
    const_iterator begin() const override
    {
        return m_nodes.begin();
    }

    /**
     * @brief Returns the end subnode iterator
     */
    iterator end() override
    {
        return m_nodes.end();
    }

    /**
     * @brief Returns the end subnode const iterator
     */
    const_iterator end() const override
    {
        return m_nodes.end();
    }

    /**
     * @brief Returns a number of subnodes
     */
    uint32_t size() const override
    {
        return (uint32_t) m_nodes.size();
    }

    /**
     * @brief Returns true if node has no subnodes of subnodes
     */
    bool empty() const override
    {
        return m_nodes.empty();
    }

    /**
     * @brief Appends a subnode
     *
     * @param node              Node to append
     */
    void push_back(Node* node) override;

    /**
     * @brief Inserts a subnode
     *
     * @param pos               Insert position with the list of subnodes
     * @param node              Node to insert
     */
    void insert(iterator pos, Node* node) override;

    /**
     * @brief Removes a subnode
     *
     * Any memory allocated for subnode is released and subnode is
     * removed from its parent
     * @param node              Node to remove
     */
    void remove(Node* node) override;

    /**
     * @brief Removes a subnode
     *
     * Disconnects subnode from parent (this node)
     */
    void unlink(Node* node) override;

    /**
     * @brief Deletes all child nodes
     *
     * Any memory, associated with child nodes, is released.
     */
    void clearChildren() override;

    /**
     * @brief Deletes all children and clears all the attributes
     *
     * Any memory, associated with children or attributes,
     * is released.
     */
    void clear() override;

    /**
     * @brief Returns referrence to node attributes
     */
    Attributes& attributes() override
    {
        return m_attributes;
    }

    /**
     * @brief Returns referrence to node attributes (const version)
     */
    const Attributes& attributes() const override
    {
        return m_attributes;
    }

    /**
     * @brief Returns true, if node has any attributes
     */
    bool hasAttributes() const override
    {
        return !m_attributes.empty();
    }

    /**
     * @brief Returns true, if given attribute is found
     * @param attr              Attribute to search
     */
    bool hasAttribute(const char *attr) const override
    {
        return m_attributes.hasAttribute(attr);
    }

    /**
     * @brief Returns attribute value for given attribute.
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
     * @brief Sets new value to attribute 'attr'.
     *
     * If attribute is not found, it's added to map.
     * @param attr              Attribute name
     * @param value             Attribute value
     * @param defaultValue      Default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
     */
    void setAttribute(const char *attr, Variant value, const char *defaultValue="") override
    {
        m_attributes.setAttribute(attr,value,defaultValue);
    }

    /**
     * @brief Sets new value to attribute 'attr'.
     *
     * If attribute is not found, it's added to map.
     * @param attr              Attribute name
     * @param value             Attribute value
     * @param defaultValue      Default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
     */
    void setAttribute(const String& attr, Variant value, const char *defaultValue="") override
    {
        m_attributes.setAttribute(attr.c_str(),value,defaultValue);
    }
};

/**
 * XML processing instructions (PI)
 */
class SP_EXPORT PI : public Element
{
public:
    /**
     * Constructor
     *
     * @param parent            Parent node. Make sure it's a pointer to the existing node.
     * @param target            Target tag name
     */
    PI(Node& parent, const String& target)
    : Element(parent, target)
    {
    }

    /**
     * Constructor
     *
     * @param parent            Parent node. Make sure it's a pointer to the existing node.
     * @param target            Target tag name
     */
    PI(Node* parent, const String& target)
    : Element(*parent, target)
    {
    }

    /**
     * Constructor
     *
     * @param parent            Parent node
     * @param target            Target tag name
     */
    PI(Node& parent, String target, const String& data)
    : Element(parent, target)
    {
    }

    /**
     * Returns node type
     */
    virtual NodeType type() const
    {
        return DOM_PI;
    }
};

/**
 * @}
 */
}
}
#endif
