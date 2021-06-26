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

#include <sptk5/Field.h>
#include <sptk5/cxml>
#include <sptk5/json/JsonElement.h>
#include <sptk5/wsdl/WSFieldIndex.h>
#include <sptk5/wsdl/WSType.h>
#include <sptk5/xml/Element.h>

namespace sptk {

/**
 * Wrapper for WSDL array type
 */
template<typename T>
class SP_EXPORT WSArray
    : public WSType
{
public:
    using value_type = T;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    /**
     * Constructor
     * @param name              Element name
     */
    explicit WSArray(const char* name = "array")
        : m_name(name)
    {
    }

    /**
     * Return class name
     */
    [[nodiscard]] String className() const override
    {
        return "WSArray";
    }

    [[nodiscard]] bool isNull() const override
    {
        return m_array.empty();
    }

    [[nodiscard]] size_t size() const
    {
        return m_array.size();
    }

    void clear() override
    {
        return m_array.clear();
    }

    bool empty() const
    {
        return m_array.empty();
    }

    T& operator[](size_t index)
    {
        return m_array[index];
    }

    const T& operator[](size_t index) const
    {
        return m_array[index];
    }

    iterator begin()
    {
        return m_array.begin();
    }

    iterator end()
    {
        return m_array.end();
    }

    const_iterator begin() const
    {
        return m_array.begin();
    }

    const_iterator end() const
    {
        return m_array.end();
    }

    T& front()
    {
        return m_array.front();
    }

    const T& front() const
    {
        return m_array.front();
    }

    T& back()
    {
        return m_array.back();
    }

    const T& back() const
    {
        return m_array.back();
    }

    void push_back(const T& value)
    {
        m_array.push_back(value);
    }

    void emplace_back(const T& value)
    {
        m_array.emplace_back(value);
    }

    auto erase(const iterator& pos)
    {
        return m_array.erase(pos);
    }

    auto erase(const iterator& first, const iterator& last)
    {
        return m_array.erase(first, last);
    }

    void load(const xml::Node* node) override
    {
        for (const auto* arrayElement: *node)
        {
            T item(m_name.c_str(), false);
            item.load(arrayElement);
            m_array.push_back(std::move(item));
        }
    }

    /**
     * Loads type data from request JSON element
     * @param attr              JSON element
     */
    void load(const json::Element* attr) override
    {
        for (const auto* arrayElement: attr->getArray())
        {
            T item(m_name.c_str(), false);
            item.load(arrayElement);
            m_array.push_back(std::move(item));
        }
    }

    /**
     * Conversion to string
     */
    String asString() const override
    {
        throwException("Invalid conversion attempt")
    }

    /**
     * Adds an element to response XML with this object data
     * @param parent            Parent XML element
     * @param name              Optional name for child element
     */
    void addElement(xml::Node* output, const char* name = nullptr) const override
    {
        const char* itemName = name == nullptr ? "item" : name;
        auto* arrayNode = new xml::Element(output, m_name.c_str());
        for (const auto& element: m_array)
        {
            element.addElement(arrayNode, itemName);
        }
    }

    /**
     * Adds an element to response JSON with this object data
     * @param parent            Parent JSON element
     */
    void addElement(json::Element* parent) const override
    {
        auto* records_array = parent->add_array(m_name);
        for (const auto& element: m_array)
        {
            element.addElement(records_array);
        }
    }

private:
    String m_name;
    std::vector<T> m_array;
};

}
