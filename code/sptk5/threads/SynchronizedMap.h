/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SynchronizedMap.h - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifndef __CSYNCHRONIZEDMAP_H__
#define __CSYNCHRONIZEDMAP_H__

#include <sptk5/sptk.h>
#include <map>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * @brief Synchronized map
 *
 * Simple thread-safe map
 */
template <class K, class T>
class SynchronizedMap
{
    typedef std::map<K,T>   Map;
    /**
     * Map
     */
    Map                     m_map;


protected:

    /**
     * Lock to synchronize map operations
     */
    mutable std::mutex      m_sync;


public:

    /**
     * @brief Map callback function used in each() method.
     *
     * Iterates through list until false is returned.
     * @param key const K&, Map item key
     * @param item T&, Map item
     * @param data void*, Optional function-specific data
     */
    typedef bool (CallbackFunction)(const K& key, T& item, void* data);

public:

    /**
     * @brief Default constructor
     */
    SynchronizedMap()
    {}

    /**
     * @brief Destructor
     */
    virtual ~SynchronizedMap()
    {}

    /**
     * @brief Inserts data item to the map
     * @param key const K&, A data key
     * @param data const T&, A data item
     */
    virtual void insert(const K& key, const T& data)
    {
        std::lock_guard<std::mutex> lock(m_sync);
        m_map[key] = data;
    }

    /**
     * @brief Finds a data item from the list front
     *
     * Returns true if key exists and data populated.
     * @param key const K&, A data key
     * @param item T&, A list item (output)
     */
    virtual bool get(const K& key, T& item)
    {
        std::lock_guard<std::mutex> lock(m_sync);
        typename Map::iterator itor = m_map.find(key);
        if (itor == m_map.end())
            return false;
        item = itor->second;
        return true;
    }

    /**
     * @brief Removes data with matching key
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
     * @brief Returns true if the list is empty
     */
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m_sync);
        return m_map.empty();
    }

    /**
     * @brief Returns number of items in the list
     */
    uint32_t size() const
    {
        std::lock_guard<std::mutex> lock(m_sync);
        return m_map.size();
    }

    /**
     * @brief Removes all items from the list
     */
    void clear()
    {
        std::lock_guard<std::mutex> lock(m_sync);
        m_map.clear();
    }

    /**
     * @brief Calls callbackFunction() for every list until false is returned
     * @param callbackFunction CallbackFunction*, Callback function that is executed for list items
     * @param data void*, Function-specific data
     * @returns true if every list item was processed
     */
    bool each(CallbackFunction* callbackFunction, void* data=NULL)
    {
        std::lock_guard<std::mutex> lock(m_sync);
        typename Map::iterator itor;
        for (itor = m_map.begin(); itor != m_map.end(); itor++) {
            if (!callbackFunction(itor->first, itor->second, data))
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
