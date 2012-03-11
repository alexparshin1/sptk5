/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXmlElement.h  -  description
                             -------------------
    begin                : Sun May 22 2003
    based on the code    : Mikko Lahteenmaki <Laza@Flashmail.com>
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CXMLELEMENT_H__
#define __CXMLELEMENT_H__

#include <sptk5/CBuffer.h>
#include <sptk5/CStrings.h>
#include <sptk5/CDateTime.h>
#include <sptk5/xml/CXmlValue.h>
#include <sptk5/xml/CXmlAttributes.h>

#include <string>
#include <map>
#include <vector>

namespace sptk {

/// @addtogroup XML
/// @{

class CXmlDoc;

/// @brief XML Element is a named item that may optionally have sub-nodes and attributes
class SP_EXPORT CXmlElement : public CXmlNamedItem
{
    friend class CXmlDoc;
    CXmlNodeList       m_nodes;              ///< The list of subnodes

protected:
    CXmlAttributes     m_attributes;         ///< Node attributes

    /// @brief Protected constructor for creating CXmlDoc only
    ///
    /// @param doc CXmlDoc&, a document.
    CXmlElement(CXmlDoc& doc) : 
        CXmlNamedItem(doc), 
        m_attributes(this) 
    {}

public:
    /// @brief Constructor
    ///
    /// @param parent CXmlNode*, a parent node.
    /// @param tagname const char*, a name of XML tag
    CXmlElement(CXmlNode& parent, const char* tagname) :
        CXmlNamedItem(parent,tagname),
        m_attributes(this) 
    {}

    /// @brief Constructor
    ///
    /// @param parent CXmlNode*, a parent node.
    /// @param tagname const char*, a name of XML tag
    CXmlElement(CXmlNode* parent, const char* tagname) : 
        CXmlNamedItem(*parent,tagname),
        m_attributes(this)
    {}

    /// @brief Constructor
    ///
    /// @param parent CXmlNode &, a parent node.
    /// @param tagname const string&, a name of XML tag
    CXmlElement(CXmlNode& parent, const std::string& tagname) : 
        CXmlNamedItem(parent,tagname),
        m_attributes(this)
    {}

    /// @brief Returns node type
    virtual CXmlNodeType type() const
    {
        return DOM_ELEMENT;
    }

    /// @brief Returns the first subnode iterator
    virtual iterator begin()
    {
        return m_nodes.begin();
    }

    /// @brief Returns the first subnode const iterator
    virtual const_iterator begin() const
    {
        return m_nodes.begin();
    }

    /// @brief Returns the end subnode iterator
    virtual iterator end()
    {
        return m_nodes.end();
    }

    /// @brief Returns the end subnode const iterator
    virtual const_iterator end() const
    {
        return m_nodes.end();
    }

    /// @brief Returns a number of subnodes
    virtual uint32_t size() const
    {
        return (uint32_t) m_nodes.size();
    }

    /// @brief Returns true if node has no subnodes of subnodes
    virtual bool empty() const
    {
        return m_nodes.empty();
    }

    /// @brief Appends a subnode
    ///
    /// @param node CXmlNode*, node to append
    virtual void push_back(CXmlNode* node);

    /// @brief Inserts a subnode
    ///
    /// @param pos iterator, insert position with the list of subnodes
    /// @param node CXmlNode*, node to insert
    virtual void insert(iterator pos, CXmlNode* node);

    /// @brief Removes a subnode
    ///
    /// Any memory allocated for subnode is released and subnode is
    /// removed from its parent
    /// @param node CXmlNode*, node to remove
    virtual void remove(CXmlNode* node);

    /// @brief Removes a subnode
    ///
    /// Disconnects subnode from parent (this node)
    virtual void unlink(CXmlNode* node);

    /// @brief Deletes all child nodes
    ///
    /// Any memory, associated with child nodes, is released.
    virtual void clearChildren();

    /// @brief Deletes all children and clears all the attributes
    ///
    /// Any memory, associated with children or attributes,
    /// is released.
    virtual void clear();

    /// @brief Returns referrence to node attributes
    virtual CXmlAttributes& attributes()
    {
        return m_attributes;
    }

    /// @brief Returns referrence to node attributes (const version)
    virtual const CXmlAttributes& attributes() const
    {
        return m_attributes;
    }

    /// @brief Returns true, if node has any attributes
    virtual bool hasAttributes() const
    {
        return m_attributes.size() != 0;
    }

    /// @brief Returns true, if given attribute is found
    /// @param attr const char *, attribute to search
    virtual bool hasAttribute(const char *attr) const
    {
        return m_attributes.hasAttribute(attr);
    }

    /// @brief Returns attribute value for given attribute.
    ///
    /// HTML tags can have empty attributes, for those you should use has_attribute() method.
    /// @param attr std::string, name of attribute
    /// @param defaultValue const char *, a default value. If attribute doesn't exist then default value is returned.
    /// @returns attribute value
    virtual CXmlValue getAttribute(std::string attr, const char *defaultValue="") const
    {
        return m_attributes.getAttribute(attr,defaultValue);
    }

    /// @brief Sets new value to attribute 'attr'.
    ///
    /// If attribute is not found, it's added to map.
    /// @param attr const char*, attribute name
    /// @param value CXmlValue, attribute value
    /// @param defaultValue const char *, a default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
    virtual void setAttribute(const char *attr, CXmlValue value, const char *defaultValue="")
    {
        m_attributes.setAttribute(attr,value,defaultValue);
    }

    /// @brief Sets new value to attribute 'attr'.
    ///
    /// If attribute is not found, it's added to map.
    /// @param attr const string&, an attribute name
    /// @param value CXmlValue, attribute value
    /// @param defaultValue const char *, a default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
    virtual void setAttribute(const std::string& attr, CXmlValue value, const char *defaultValue="")
    {
        m_attributes.setAttribute(attr.c_str(),value,defaultValue);
    }
};

/// @}
}
#endif
