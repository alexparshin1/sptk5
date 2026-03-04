/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <list>
#include <stdexcept>
#include <tuple>
#include <unordered_map>

namespace sptk {

/**
 * @addtogroup utility Utility Classes.
 * @{
 */

/**
 * @brief Ordered map with access by key and insertion-order index.
 *
 * Stores unique keys and preserves insertion order.
 * Updating an existing key changes only the value and preserves its index.
 */
template<class K, class T>
class OrderedMap
{
public:
    using value_type = std::pair<const K, T>;
    using container_type = std::list<value_type>;
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;

    /**
     * @brief Default constructor.
     */
    OrderedMap() = default;
    /**
     * @brief Copy constructor.
     */
    OrderedMap(const OrderedMap& other)
        : m_items(other.m_items)
    {
        reindex();
    }

    /**
     * @brief Move constructor.
     */
    OrderedMap(OrderedMap&& other) noexcept
        : m_items(std::move(other.m_items))
        , m_index(std::move(other.m_index))
    {
    }

    /**
     * @brief Destructor.
     */
    ~OrderedMap() = default;

    /**
     * @brief Copy assignment operator.
     * @param other Another object.
     * @return this object.
     */
    OrderedMap& operator=(const OrderedMap& other)
    {
        if (this != &other)
        {
            m_index.clear();
            m_items.clear();
            for (const auto& item: other.m_items)
            {
                m_items.emplace_back(item);
            }
            reindex();
        }
        return *this;
    }

    /**
     * @brief Move assignment operator.
     * @param other Another object.
     * @return this object.
     */
    OrderedMap& operator=(OrderedMap&& other) noexcept
    {
        if (this != &other)
        {
            m_items = std::move(other.m_items);
            m_index = std::move(other.m_index);
        }
        return *this;
    }

    /**
     * @brief Returns true if no elements are stored.
     */
    [[nodiscard]] bool empty() const
    {
        return m_items.empty();
    }

    /**
     * @brief Returns the number of elements.
     */
    [[nodiscard]] size_t size() const
    {
        return m_items.size();
    }

    /**
     * @brief Returns reference to the first value.
     */
    T& front()
    {
        if (m_items.empty())
        {
            throw std::out_of_range("OrderedMap is empty");
        }
        return m_items.front().second;
    }

    /**
     * @brief Returns const reference to the first value.
     */
    const T& front() const
    {
        if (m_items.empty())
        {
            throw std::out_of_range("OrderedMap is empty");
        }
        return m_items.front().second;
    }

    /**
     * @brief Returns reference to the last value.
     */
    T& back()
    {
        if (m_items.empty())
        {
            throw std::out_of_range("OrderedMap is empty");
        }
        return m_items.back().second;
    }

    /**
     * @brief Returns const reference to the last value.
     */
    const T& back() const
    {
        if (m_items.empty())
        {
            throw std::out_of_range("OrderedMap is empty");
        }
        return m_items.back().second;
    }

    /**
     * @brief Removes all elements.
     */
    void clear()
    {
        m_items.clear();
        m_index.clear();
    }

    /**
     * @brief Returns true if the key exists.
     */
    [[nodiscard]] bool contains(const K& key) const
    {
        return m_index.contains(key);
    }

    /**
     * @brief Inserts a key/value or updates the existing key.
     * @return tuple<iterator, inserted> where inserted is true when inserted, false when updated.
     */
    std::tuple<iterator, bool> insert(const K& key, const T& value)
    {
        if (auto itor = m_index.find(key);
            itor != m_index.end())
        {
            itor->second->second = value;
            return {itor->second, false};
        }
        auto it = m_items.emplace(m_items.end(), key, value);
        try
        {
            m_index.emplace(key, it);
        }
        catch (const std::exception&)
        {
            m_items.pop_back();
            throw;
        }
        return {it, true};
    }

    /**
     * @brief Returns reference to value by key, or creates a new entry.
     */
    T& operator[](const K& key)
    {
        if (const auto itor = m_index.find(key); itor != m_index.end())
        {
            auto& itemsIterator = itor->second;
            return itemsIterator->second;
        }
        const auto& [it, inserted] = insert(key, T {});
        return it->second;
    }

    /**
     * @brief Returns reference to value by key, or throws an exception.
     */
    T& at(const K& key)
    {
        if (const auto itor = m_index.find(key); itor != m_index.end())
        {
            auto& itemsIterator = itor->second;
            return itemsIterator->second;
        }
        throw std::out_of_range("OrderedMap::at: key not found");
    }

    /**
     * @brief Returns reference to value by key, or throws an exception.
     */
    const T& at(const K& key) const
    {
        if (const auto itor = m_index.find(key); itor != m_index.end())
        {
            auto& itemsIterator = itor->second;
            return itemsIterator->second;
        }
        throw std::out_of_range("OrderedMap::at: key not found");
    }

    /**
     * @brief Returns iterator to the value pair by the key, or end().
     */
    [[nodiscard]] iterator find(const K& key)
    {
        if (auto it = m_index.find(key);
            it != m_index.end())
        {
            return it->second;
        }
        return m_items.end();
    }

    /**
     * @brief Returns iterator to the value pair by the key, or end().
     */
    [[nodiscard]] const_iterator find(const K& key) const
    {
        const auto it = m_index.find(key);
        if (it == m_index.end())
            return m_items.end();
        return it->second;
    }

    /**
     * @brief Returns iterator to the first item.
     */
    iterator begin()
    {
        return m_items.begin();
    }

    /**
     * @brief Returns const iterator to the first item.
     */
    const_iterator begin() const
    {
        return m_items.begin();
    }

    /**
     * @brief Returns iterator to the first item.
     */
    iterator end()
    {
        return m_items.end();
    }

    /**
     * @brief Returns const iterator to the first item.
     */
    const_iterator end() const
    {
        return m_items.end();
    }

    /**
     * @brief Removes an element by key.
     * @return true when removed.
     */
    bool erase(const K& key)
    {
        const auto itor = m_index.find(key);
        if (itor == m_index.end())
        {
            return false;
        }
        auto& itemsIterator = itor->second;
        m_items.erase(itemsIterator);
        m_index.erase(itor);
        return true;
    }

    /**
     * @brief Removes an element by insertion-order index.
     * @return true when removed.
     */
    bool erase(iterator it)
    {
        if (it == m_items.end())
        {
            return false;
        }
        return erase(it->first);
    }

private:
    container_type                  m_items;
    std::unordered_map<K, iterator> m_index;

    void reindex()
    {
        try
        {
            m_index.clear();
            for (auto it = m_items.begin(); it != m_items.end(); ++it)
            {
                m_index.emplace(it->first, it);
            }
        }
        catch (const std::exception&)
        {
            m_items.clear();
            m_index.clear();
            throw;
        }
    }
};

/**
 * @}
 */

} // namespace sptk
