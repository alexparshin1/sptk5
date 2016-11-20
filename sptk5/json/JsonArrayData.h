/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonArrayData.h - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
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

#ifndef __JSON_ARRAY_DATA_H__
#define __JSON_ARRAY_DATA_H__

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>

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
    typedef std::vector<Element*>   Vector;
    typedef Vector::iterator        iterator;
    typedef Vector::const_iterator  const_iterator;
protected:
    Element*                m_parent;
    Vector                  m_items;

    void setParent(Element *parent);

public:
    ArrayData(Element *parent = NULL);

    ~ArrayData();

    void add(Element *element);

    template <typename T> void add(T value)
    {
        add(new Element(value));
    }

    Element &operator[](size_t index) throw(Exception);

    const Element &operator[](size_t index) const throw(Exception);

    void remove(size_t index);

    iterator begin() { return m_items.begin(); }

    iterator end() { return m_items.end(); }

    const_iterator begin() const { return m_items.begin(); }

    const_iterator end() const { return m_items.end(); }

    size_t size() const { return m_items.size(); }
};

}}

#endif
