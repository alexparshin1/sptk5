/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/Field.h>
#include <sptk5/wsdl/WSType.h>
#include <sptk5/xdoc/Document.h>
#include <sptk5/xdoc/Node.h>

namespace sptk {
/**
 * @brief Wrapper for WSDL array type.
 */
template<typename T>
    requires std::is_base_of_v<WSType, T>
class WS_EXPORT WSArray
    : public WSType
{
public:
    using iterator = std::vector<T>::iterator;
    using const_iterator = std::vector<T>::const_iterator;

    /**
     * @brief Constructor.
     * @param name              Element name.
     */
    explicit WSArray(const char* name = "array")
        : WSType(name)
    {
    }

    /**
     * @brief Return class name.
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

    [[nodiscard]] bool empty() const
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

    [[maybe_unused]] T& front()
    {
        return m_array.front();
    }

    [[maybe_unused]] const T& front() const
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

    void push_back(T&& value)
    {
        m_array.push_back(std::move(value));
    }

    void emplace_back(const T& value)
    {
        m_array.emplace_back(value);
    }

    void emplace_back(T&& value)
    {
        m_array.emplace_back(std::move(value));
    }

    [[maybe_unused]] void reserve(size_t sz)
    {
        m_array.reserve(sz);
    }

    void resize(size_t sz)
    {
        m_array.resize(sz);
    }

    auto capacity() const
    {
        return m_array.capacity();
    }

    auto erase(const iterator& pos)
    {
        return m_array.erase(pos);
    }

    auto erase(const iterator& first, const iterator& last)
    {
        return m_array.erase(first, last);
    }

    void load(const xdoc::SNode& node, bool nullLargeData) override
    {
        for (const auto& arrayElement: node->nodes())
        {
            T item(name().c_str(), false);
            item.load(arrayElement, nullLargeData);
            m_array.push_back(std::move(item));
        }
    }

    /**
     * @brief Conversion to string.
     */
    [[nodiscard]] String asString() const override
    {
        xdoc::Document document;
        Buffer         buffer;
        exportTo(document.root(), nullptr);
        const auto& arrayNode = document.root()->findFirst(name());
        arrayNode->exportTo(xdoc::DataFormat::JSON, buffer, false);
        return static_cast<String>(buffer);
    }

    /**
     * @brief Adds an element to response XML with this object data.
     * @param output            Destination node.
     * @param name              Optional name for the child element.
     */
    void exportTo(const xdoc::SNode& output, const char* name) const override
    {
        const char* itemName = name == nullptr ? "item" : name;
        auto        arrayNode = output->pushNode(this->name(), xdoc::Node::Type::Array);
        for (const auto& element: m_array)
        {
            element.exportTo(arrayNode, itemName);
        }
    }

private:
    std::vector<T> m_array;
};

} // namespace sptk
