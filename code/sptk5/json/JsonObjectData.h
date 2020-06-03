/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonObjectData.h - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
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

#ifndef __JSON_OBJECT_DATA_H__
#define __JSON_OBJECT_DATA_H__

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <set>
#include <unordered_map>
#include <list>

namespace sptk::json {

/// @addtogroup JSON
/// @{

class Element;
class Document;

class SP_EXPORT ElementMap
{

public:
    class Item
    {
        friend class ElementMap;
    public:
        Item(const std::string* name, Element* element) : m_name(name), m_element(element) {}
        std::string name() const { return *m_name; }
        Element* element() { return m_element; }
        const Element* element() const { return m_element; }
    protected:
        const std::string*  m_name;
        Element*            m_element;
    };

    typedef std::list<Item>::iterator       iterator;
    typedef std::list<Item>::const_iterator const_iterator;

    Element* get(const std::string* name)
    {
        for (auto& itor: m_items) {
            if (itor.m_name == name)
                return itor.element();
        }
        return nullptr;
    }

    void insert(const std::string* name, Element* element)
    {
        for (auto& itor: m_items) {
            if (itor.m_name == name) {
                itor.m_element = element;
                return;
            }
        }
        m_items.emplace_back(name, element);
    }

    iterator begin() { return m_items.begin(); }
    const_iterator begin() const { return m_items.begin(); }
    iterator end() { return m_items.end(); }
    const_iterator end() const { return m_items.end(); }
    size_t size() const { return m_items.size(); }

    iterator find(const std::string* name)
    {
        for (auto itor = m_items.begin(); itor != m_items.end(); ++itor) {
            if (itor->m_name == name)
                return itor;
        }
        return m_items.end();
    }

    const_iterator find(const std::string* name) const
    {
        for (auto itor = m_items.begin(); itor != m_items.end(); ++itor) {
            if (itor->m_name == name)
                return itor;
        }
        return m_items.end();
    }

    iterator erase(iterator itor)
    {
        return m_items.erase(itor);
    }

private:
    std::list<Item> m_items;
};

/**
 * Map of names to JSON Element objects
 */
class SP_EXPORT ObjectData
{
    friend class Element;

public:

    /**
     * Type definition: map of element names to elements iterator
     */
    typedef ElementMap::iterator                       iterator;

    /**
     * Type definition: map of element names to elements const iterator
     */
    typedef ElementMap::const_iterator                 const_iterator;

private:

    Document*                                   m_document;

    /**
     * Parent JSON element
     */
    Element*                                    m_parent;

    /**
     * Child JSON elements
     */
    ElementMap                                  m_items;

protected:

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
    [[nodiscard]] const Element* find(const std::string& name) const;

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
    [[nodiscard]] const_iterator begin() const { return m_items.begin(); }

    /**
     * Get const end iterator of child elements
     * @return const end iterator of child elements
     */
    [[nodiscard]] const_iterator end() const { return m_items.end(); }

    /**
     * Get number of child elements
     * @return number of child elements
     */
    [[nodiscard]] size_t size() const { return m_items.size(); }
};

/// @}

}

#endif
