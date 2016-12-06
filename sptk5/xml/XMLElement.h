/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       XMLElement.h - description                             ║
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

#ifndef __SPTK_XMLELEMENT_H__
#define __SPTK_XMLELEMENT_H__

#include <sptk5/Buffer.h>
#include <sptk5/Strings.h>
#include <sptk5/DateTime.h>
#include <sptk5/xml/XMLValue.h>
#include <sptk5/xml/XMLAttributes.h>

#include <string>
#include <map>
#include <vector>

namespace sptk {

/**
 * @addtogroup XML
 * @{
 */

class XMLDocument;

/**
 * @brief XML Element is a named item that may optionally have sub-nodes and attributes
 */
class SP_EXPORT XMLElement : public XMLNamedItem
{
    friend class XMLDocument;
    /**
     * The list of subnodes
     */
    XMLNodeList       m_nodes;


protected:
    /**
     * Node attributes
     */
    XMLAttributes     m_attributes;


    /**
     * @brief Protected constructor for creating XMLDoc only
     *
     * @param doc XMLDoc&, a document.
     */
    XMLElement(XMLDocument& doc) : 
        XMLNamedItem(doc), 
        m_attributes(this) 
    {}

public:
    /**
     * @brief Constructor
     *
     * @param parent XMLNode*, a parent node.
     * @param tagname const char*, a name of XML tag
     */
    XMLElement(XMLNode& parent, const char* tagname) :
        XMLNamedItem(parent,tagname),
        m_attributes(this) 
    {}

    /**
     * @brief Constructor
     *
     * @param parent XMLNode*, a parent node.
     * @param tagname const char*, a name of XML tag
     */
    XMLElement(XMLNode* parent, const char* tagname) : 
        XMLNamedItem(*parent,tagname),
        m_attributes(this)
    {}

    /**
     * @brief Constructor
     *
     * @param parent XMLNode &, a parent node.
     * @param tagname const string&, a name of XML tag
     */
    XMLElement(XMLNode& parent, const std::string& tagname) : 
        XMLNamedItem(parent,tagname),
        m_attributes(this)
    {}

    /**
     * @brief Returns node type
     */
    virtual XMLNodeType type() const
    {
        return DOM_ELEMENT;
    }

    /**
     * @brief Returns the first subnode iterator
     */
    virtual iterator begin()
    {
        return m_nodes.begin();
    }

    /**
     * @brief Returns the first subnode const iterator
     */
    virtual const_iterator begin() const
    {
        return m_nodes.begin();
    }

    /**
     * @brief Returns the end subnode iterator
     */
    virtual iterator end()
    {
        return m_nodes.end();
    }

    /**
     * @brief Returns the end subnode const iterator
     */
    virtual const_iterator end() const
    {
        return m_nodes.end();
    }

    /**
     * @brief Returns a number of subnodes
     */
    virtual uint32_t size() const
    {
        return (uint32_t) m_nodes.size();
    }

    /**
     * @brief Returns true if node has no subnodes of subnodes
     */
    virtual bool empty() const
    {
        return m_nodes.empty();
    }

    /**
     * @brief Appends a subnode
     *
     * @param node XMLNode*, node to append
     */
    virtual void push_back(XMLNode* node);

    /**
     * @brief Inserts a subnode
     *
     * @param pos iterator, insert position with the list of subnodes
     * @param node XMLNode*, node to insert
     */
    virtual void insert(iterator pos, XMLNode* node);

    /**
     * @brief Removes a subnode
     *
     * Any memory allocated for subnode is released and subnode is
     * removed from its parent
     * @param node XMLNode*, node to remove
     */
    virtual void remove(XMLNode* node);

    /**
     * @brief Removes a subnode
     *
     * Disconnects subnode from parent (this node)
     */
    virtual void unlink(XMLNode* node);

    /**
     * @brief Deletes all child nodes
     *
     * Any memory, associated with child nodes, is released.
     */
    virtual void clearChildren();

    /**
     * @brief Deletes all children and clears all the attributes
     *
     * Any memory, associated with children or attributes,
     * is released.
     */
    virtual void clear();

    /**
     * @brief Returns referrence to node attributes
     */
    virtual XMLAttributes& attributes()
    {
        return m_attributes;
    }

    /**
     * @brief Returns referrence to node attributes (const version)
     */
    virtual const XMLAttributes& attributes() const
    {
        return m_attributes;
    }

    /**
     * @brief Returns true, if node has any attributes
     */
    virtual bool hasAttributes() const
    {
        return m_attributes.size() != 0;
    }

    /**
     * @brief Returns true, if given attribute is found
     * @param attr const char *, attribute to search
     */
    virtual bool hasAttribute(const char *attr) const
    {
        return m_attributes.hasAttribute(attr);
    }

    /**
     * @brief Returns attribute value for given attribute.
     *
     * HTML tags can have empty attributes, for those you should use has_attribute() method.
     * @param attr std::string, name of attribute
     * @param defaultValue const char *, a default value. If attribute doesn't exist then default value is returned.
     * @returns attribute value
     */
    virtual XMLValue getAttribute(std::string attr, const char *defaultValue="") const
    {
        return m_attributes.getAttribute(attr,defaultValue);
    }

    /**
     * @brief Sets new value to attribute 'attr'.
     *
     * If attribute is not found, it's added to map.
     * @param attr const char*, attribute name
     * @param value XMLValue, attribute value
     * @param defaultValue const char *, a default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
     */
    virtual void setAttribute(const char *attr, XMLValue value, const char *defaultValue="")
    {
        m_attributes.setAttribute(attr,value,defaultValue);
    }

    /**
     * @brief Sets new value to attribute 'attr'.
     *
     * If attribute is not found, it's added to map.
     * @param attr const string&, an attribute name
     * @param value XMLValue, attribute value
     * @param defaultValue const char *, a default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
     */
    virtual void setAttribute(const std::string& attr, XMLValue value, const char *defaultValue="")
    {
        m_attributes.setAttribute(attr.c_str(),value,defaultValue);
    }
};

/**
 * @}
 */
}
#endif
