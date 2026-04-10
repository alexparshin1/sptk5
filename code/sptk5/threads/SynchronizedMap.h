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

#include <functional>
#include <map>
#include <mutex>

namespace sptk {
/**
 * @addtogroup threads Thread Classes.
 * @{
 */

/**
 * @brief Synchronized map.
 *
 * Simple thread-safe map.
 */
template<class K, class T>
class SynchronizedMap
{
public:
    virtual ~SynchronizedMap() = default;

    /**
     * @brief Map callback function used in each() method.
     *
     * Iterates through the map until false is returned.
     * @param key const K&, Map item key.
     * @param item T&, Map item.
     * @param data void*, Optional function-specific data.
     */
    using CallbackFunction = std::function<bool(const K& key, T& item)>;

    /**
     * @brief Inserts data item to the map.
     * @param key const K&, A data key.
     * @param data const T&, A data item.
     */
    virtual void insert(const K& key, const T& data)
    {
        std::scoped_lock lock(m_mutex);
        m_map.emplace(key, data);
    }

    /**
     * @brief Finds a data item from the map.
     *
     * Returns true if the key exists.
     * @param key               A data key.
     * @param item              A data item (output).
     * @param remove            If true, then the item is removed from the map.
     */
    virtual bool get(const K& key, T& item, const bool remove)
    {
        std::scoped_lock       lock(m_mutex);
        typename Map::iterator itor = m_map.find(key);
        if (itor == m_map.end())
        {
            return false;
        }
        item = itor->second;
        if (remove)
        {
            m_map.erase(itor);
        }
        return true;
    }

    /**
     * @brief Finds a data item from the map.
     *
     * Returns true if the key exists.
     * @param key               A data key.
     * @param item              A data item (output).
     */
    virtual bool get(const K& key, T& item)
    {
        return get(key, item, false);
    }

    /**
     * @brief Removes data with the matching key.
     *
     * Returns true if the key existed.
     * @param key const K&, A data key.
     */
    virtual bool erase(const K& key)
    {
        std::scoped_lock       lock(m_mutex);
        typename Map::iterator itor = m_map.find(key);
        if (itor == m_map.end())
        {
            return false;
        }
        m_map.erase(itor);
        return true;
    }

    /**
     * @brief Returns true if the map is empty.
     */
    bool empty() const
    {
        std::scoped_lock lock(m_mutex);
        return m_map.empty();
    }

    /**
     * @brief Returns the number of items in the map.
     */
    size_t size() const
    {
        std::scoped_lock lock(m_mutex);
        return m_map.size();
    }

    /**
     * @brief Removes all items from the map.
     */
    void clear()
    {
        std::scoped_lock lock(m_mutex);
        m_map.clear();
    }

    /**
     * @brief Calls callbackFunction() for every list until false is returned.
     * @param callbackFunction  Callback function that is executed for list items.
     * @returns true  if every list item was processed.
     */
    bool for_each(CallbackFunction callbackFunction)
    {
        std::scoped_lock lock(m_mutex);
        for (auto itor: m_map)
        {
            if (!callbackFunction(itor.first, itor.second))
            {
                return false;
            }
        }
        return true;
    }

private:
    using Map = std::map<K, T>;
    mutable std::mutex m_mutex; ///< Mutex for synchronizing map access.
    Map                m_map;   ///< Underlying map.
};
/**
 * @}
 */

} // namespace sptk
