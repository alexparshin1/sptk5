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

#ifndef __CSYNCHRONIZEDMAP_H__
#define __CSYNCHRONIZEDMAP_H__

#include <sptk5/sptk.h>
#include <map>
#include <functional>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * Synchronized map
 *
 * Simple thread-safe map
 */
template <class K, class T>
class SynchronizedMap
{
    /**
     * Lock to synchronize map operations
     */
    mutable std::mutex      m_sync;

    typedef std::map<K,T>   Map;
    /**
     * Map
     */
    Map                     m_map;

public:

    /**
     * Map callback function used in each() method.
     *
     * Iterates through list until false is returned.
     * @param key const K&, Map item key
     * @param item T&, Map item
     * @param data void*, Optional function-specific data
     */
    typedef std::function<bool(const K& key, T& item)>  CallbackFunction;

    /**
     * Default constructor
     */
    SynchronizedMap()
    {}

    /**
     * Destructor
     */
    virtual ~SynchronizedMap()
    {}

    /**
     * Inserts data item to the map
     * @param key const K&, A data key
     * @param data const T&, A data item
     */
    virtual void insert(const K& key, const T& data)
    {
        std::lock_guard<std::mutex> lock(m_sync);
        m_map.emplace(key, data);
    }

    /**
     * Finds a data item from the list front
     *
     * Returns true if key exists and data populated.
     * @param key               A data key
     * @param item              A list item (output)
     * @param remove            If true, then item is removed from map
     */
    virtual bool get(const K& key, T& item, bool remove=false)
    {
        std::lock_guard<std::mutex> lock(m_sync);
        typename Map::iterator itor = m_map.find(key);
        if (itor == m_map.end())
            return false;
        item = itor->second;
        if (remove)
            m_map.erase(itor);
        return true;
    }

    /**
     * Removes data with matching key
     *
     * Returns true if key existed.
     * @param key const K&, A data key
     */
    virtual bool remove(const K& key)
    {
        std::lock_guard<std::mutex> lock(m_sync);
        typename Map::iterator itor = m_map.find(key);
        if (itor == m_map.end())
            return false;
        m_map.erase(itor);
        return true;
    }

    /**
     * Returns true if the list is empty
     */
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m_sync);
        return m_map.empty();
    }

    /**
     * Returns number of items in the list
     */
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_sync);
        return m_map.size();
    }

    /**
     * Removes all items from the list
     */
    void clear()
    {
        std::lock_guard<std::mutex> lock(m_sync);
        m_map.clear();
    }

    /**
     * Calls callbackFunction() for every list until false is returned
     * @param callbackFunction  Callback function that is executed for list items
     * @returns true  if every list item was processed
     */
    bool for_each(CallbackFunction callbackFunction)
    {
        std::lock_guard<std::mutex> lock(m_sync);
        for (auto itor: m_map) {
            if (!callbackFunction(itor.first, itor.second))
                return false;
        }
        return true;
    }
};

/**
 * @}
 */

}
#endif
