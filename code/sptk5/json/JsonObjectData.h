/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonObjectData.h - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
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

#ifndef __JSON_OBJECT_DATA_H__
#define __JSON_OBJECT_DATA_H__

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <set>
#include <unordered_map>

namespace sptk { namespace json {

/// @addtogroup JSON
/// @{

class Element;
class Document;

/**
 * Map of names to JSON Element objects
 */
class ObjectData
{
    friend class Element;

    Document*                                   m_document;

public:
    /**
     * Type definition: map of element names to elements
     */
    typedef std::unordered_map<const std::string*, Element*> Map;

    /**
     * Type definition: map of element names to elements iterator
     */
    typedef Map::iterator                       iterator;

    /**
     * Type definition: map of element names to elements const iterator
     */
    typedef Map::const_iterator                 const_iterator;

protected:

    /**
     * Parent JSON element
     */
    Element*                                    m_parent;

    /**
     * Child JSON elements
     */
    Map                                         m_items;

    /**
     * Set parent JSON element
     */
    void setParent(Element *parent);

public:

    /**
     * Constructor
     * @param document          Document this object belongs to
     * @param parent            Parent JSON element
     */
    explicit ObjectData(Document* document, Element* parent = nullptr);

    /**
     * Destructor
     */
    ~ObjectData();

    /**
     * Add child JSON element
     * @param name              Child element name
     * @param element           Child element
     */
    void add(const std::string& name, Element *element);

    /**
     * Reference child element by name. If child element is not found,
     * a new Element is created.
     * @param name              Child element name
     * @returns Child element reference
     */
    Element& operator[](const std::string& name);

    /**
     * Find child element by name
     * @param name              Child element name
     * @returns Child element pointer, or NULL if not found
     */
    Element *find(const std::string& name);

    /**
     * Const reference child element by name
     * @param name              Child element name
     * @returns Const child element reference, or throws exception if not found
     */
    const Element& operator[](const std::string& name) const;

    /**
     * Find child element by name
     * @param name              Child element name
     * @returns Child element const pointer, or NULL if not found
     */
    const Element* find(const std::string& name) const;

    /**
     * Remove child element by name (and release its memory)
     * @param name              Child element name
     */
    void remove(const std::string& name);

    /**
     * Remove child element by name from object, without destroying it
     * @param name              Child element name
     * @return Element pointer
     */
    Element* move(const std::string& name);

    /**
     * Get begin iterator of child elements
     * @return begin iterator of child elements
     */
    iterator begin() { return m_items.begin(); }

    /**
     * Get end iterator of child elements
     * @return end iterator of child elements
     */
    iterator end() { return m_items.end(); }

    /**
     * Get const begin iterator of child elements
     * @return const begin iterator of child elements
     */
    const_iterator begin() const { return m_items.begin(); }

    /**
     * Get const end iterator of child elements
     * @return const end iterator of child elements
     */
    const_iterator end() const { return m_items.end(); }

    /**
     * Get number of child elements
     * @return number of child elements
     */
    size_t size() const { return m_items.size(); }
};

/// @}

}}

#endif
