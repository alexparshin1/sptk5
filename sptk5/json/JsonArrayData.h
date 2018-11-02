/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonArrayData.h - description                          ║
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

#ifndef __JSON_ARRAY_DATA_H__
#define __JSON_ARRAY_DATA_H__

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <sptk5/json/JsonElement.h>

namespace sptk { namespace json {

/// @addtogroup JSON
/// @{

class Element;

/**
 * Array of JSON Element objects
 */
class ArrayData
{
    friend class Element;

public:
    /**
     * Type definition: array of JSON elements
     */
    typedef std::vector<Element*>   Vector;

    /**
     * Type definition: iterator on array of JSON elements
     */
    typedef Vector::iterator        iterator;

    /**
     * Type definition: const iterator on array of JSON elements
     */
    typedef Vector::const_iterator  const_iterator;

protected:

    /**
     * Document this object belongs to
     */
    const Document*         m_document;

    /**
     * Parent JSON element
     */
    Element*                m_parent;

    /**
     * Array of elements
     */
    Vector                  m_items;

    /**
     * Set parent JSON element for this array
     * @param parent            Parent JSON element
     */
    void setParent(Element* parent);

public:

    /**
     * Constructor
     * @param document          Document this object belongs to
     * @param parent            Parent JSON element
     */
    explicit ArrayData(const Document* document, Element* parent = NULL);

    /**
     * Destructor
     */
    ~ArrayData();

    /**
     * Add JSON element to this array
     * @param element           JSON element
     */
    void add(Element* element);

    /**
     * Add JSON element to this array.
     *
     * JSON element is constructed from value.
     * @param value             JSON element value
     */
    template <typename T> void add(T value)
    {
        add(new Element(value));
    }

    /**
     * Get JSON element from this array by index
     * @param index             Element index
     */
    Element& operator[](size_t index);

    /**
     * Get JSON element from this array by index (const version)
     * @param index             Element index
     */
    const Element& operator[](size_t index) const;

    /**
     * Remove JSON element from this array by index (const version)
     * @param index             Element index
     */
    void remove(size_t index);

    /**
     * Get array begin iterator
     */
    iterator begin() { return m_items.begin(); }

    /**
     * Get array end iterator
     */
    iterator end() { return m_items.end(); }

    /**
     * Get array const begin iterator
     */
    const_iterator begin() const { return m_items.begin(); }

    /**
     * Get array const end iterator
     */
    const_iterator end() const { return m_items.end(); }

    /**
     * Get array size
     */
    size_t size() const { return m_items.size(); }

    /**
     * Is array empty?
     */
    bool empty() const { return m_items.empty(); }
};

}}

#endif
