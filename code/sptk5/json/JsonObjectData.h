/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
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

/**
 * Internal map of JSON object property names to JSON elements.
 *
 * Implementation keeps the creation/load order of properties,
 * and uses shared strings to increase property index performance.
 */
class SP_EXPORT PropertyMap
{

public:
    /**
     * Map item
     */
    class Property
    {
        friend class PropertyMap;
    public:
        /**
         * Constructor
         * @param name          Property name
         * @param element       Property element
         */
        Property(const std::string* name, Element* element) : m_name(name), m_element(element) {}
        String name() const { return *m_name; }
        Element* element() { return m_element; }
        const Element* element() const { return m_element; }
    private:
        const std::string*  m_name;
        Element*            m_element;
    };

    typedef std::list<Property>::iterator       iterator;
    typedef std::list<Property>::const_iterator const_iterator;

    Element* get(const std::string* name)
    {
        for (auto& itor: m_properties) {
            if (itor.m_name == name)
                return itor.element();
        }
        return nullptr;
    }

    void set(const std::string* name, Element* element)
    {
        for (auto& itor: m_properties) {
            if (itor.m_name == name) {
                itor.m_element = element;
                return;
            }
        }
        m_properties.emplace_back(name, element);
    }

    iterator begin() { return m_properties.begin(); }
    const_iterator begin() const { return m_properties.begin(); }
    iterator end() { return m_properties.end(); }
    const_iterator end() const { return m_properties.end(); }
    size_t size() const { return m_properties.size(); }

    iterator find(const std::string* name)
    {
        for (auto itor = m_properties.begin(); itor != m_properties.end(); ++itor) {
            if (itor->m_name == name)
                return itor;
        }
        return m_properties.end();
    }

    const_iterator find(const std::string* name) const
    {
        for (auto itor = m_properties.begin(); itor != m_properties.end(); ++itor) {
            if (itor->m_name == name)
                return itor;
        }
        return m_properties.end();
    }

    iterator erase(iterator itor)
    {
        return m_properties.erase(itor);
    }

private:
    std::list<Property> m_properties;
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
    typedef PropertyMap::iterator               iterator;

    /**
     * Type definition: map of element names to elements const iterator
     */
    typedef PropertyMap::const_iterator         const_iterator;

    /**
     * Constructor
     * @param document          Document this object belongs to
     * @param parent            Parent JSON element
     */
    explicit ObjectData(Document* document, Element* parent = nullptr);

    ObjectData(const ObjectData&) = delete;
    ObjectData(ObjectData&&) = default;
    ObjectData& operator = (const ObjectData&) = delete;
    ObjectData& operator = (ObjectData&&) = default;

    /**
     * Destructor
     */
    ~ObjectData();

    /**
     * Add child JSON element
     * @param name              Child element name
     * @param element           Child element
     */
    void add(const String& name, Element *element);

    /**
     * Reference child element by name. If child element is not found,
     * a new Element is created.
     * @param name              Child element name
     * @returns Child element reference
     */
    Element& operator[](const String& name);

    /**
     * Find child element by name
     * @param name              Child element name
     * @returns Child element pointer, or NULL if not found
     */
    Element *find(const String& name);

    /**
     * Const reference child element by name
     * @param name              Child element name
     * @returns Const child element reference, or throws exception if not found
     */
    const Element& operator[](const String& name) const;

    /**
     * Find child element by name
     * @param name              Child element name
     * @returns Child element const pointer, or NULL if not found
     */
    [[nodiscard]] const Element* find(const String& name) const;

    /**
     * Remove child element by name (and release its memory)
     * @param name              Child element name
     */
    void remove(const String& name);

    /**
     * Remove child element by name from object, without destroying it
     * @param name              Child element name
     * @return Element pointer
     */
    Element* move(const String& name);

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

private:

    Document*                                   m_document;

    /**
     * Parent JSON element
     */
    Element*                                    m_parent;

    /**
     * Child JSON elements
     */
    PropertyMap                                  m_items;
};

/// @}

}

#endif
