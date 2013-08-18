/***************************************************************************
*                          SIMPLY POWERFUL TOOLKIT (SPTK)
*                          CSynchronizedMap.h  -  description
*                             -------------------
*    begin                : Sat Aug 17 2013
*    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
*    email                : alexeyp@gmail.com
***************************************************************************/

/***************************************************************************
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* Neither the name of the <ORGANIZATION> nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
*  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
*  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
*  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
*  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/

#ifndef __CSYNCHRONIZEDMAP_H__
#define __CSYNCHRONIZEDMAP_H__

#include <sptk5/sptk.h>
#include <sptk5/threads/CSynchronizedCode.h>
#include <map>

namespace sptk {

/// @addtogroup threads Thread Classes
/// @{

/// @brief Synchronized map
///
/// Simple thread-safe map
template <class K, class T>
class CSynchronizedMap
{
    typedef std::map<K,T>   Map;
    Map                     m_map;              ///< Map

protected:

    mutable CSynchronized   m_sync;             ///< Lock to synchronize map operations

public:

    /// @brief Map callback function used in each() method.
    ///
    /// Iterates through list until false is returned.
    /// @param const key K&, Map item key
    /// @param item T&, Map item
    /// @param data void*, Optional function-specific data
    typedef bool (CallbackFunction)(const K& key, T& item, void* data);

public:

    /// @brief Default constructor
    CSynchronizedMap()
    {}

    /// @brief Destructor
    virtual ~CSynchronizedMap()
    {}

    /// @brief Inserts data item to the map
    /// @param key const K&, A data key
    /// @param data const T&, A data item
    virtual void insert(const K& key, const T& data)
    {
        CSynchronizedCode sc(m_sync);
        m_map[key] = data;
    }

    /// @brief Finds a data item from the list front
    ///
    /// Returns true if key exists and data populated.
    /// @param key const K&, A data key
    /// @param item T&, A list item (output)
    virtual bool get(const K& key, T& item)
    {
        CSynchronizedCode sc(m_sync);
        typename Map::iterator itor = m_map.find(key);
        if (itor == m_map.end())
            return false;
        item = itor->second;
        return true;
    }

    /// @brief Removes data with matching key
    ///
    /// Returns true if key existed.
    /// @param key const K&, A data key
    virtual bool remove(const K& key)
    {
        CSynchronizedCode sc(m_sync);
        typename Map::iterator itor = m_map.find(key);
        if (itor == m_map.end())
            return false;
        m_map.erase(itor);
        return true;
    }

    /// @brief Returns true if the list is empty
    bool empty() const
    {
        CSynchronizedCode sc(m_sync);
        return m_map.empty();
    }

    /// @brief Returns number of items in the list
    uint32_t size() const
    {
        CSynchronizedCode sc(m_sync);
        return m_map.size();
    }

    /// @brief Removes all items from the list
    void clear()
    {
        CSynchronizedCode sc(m_sync);
        m_map.clear();
    }

    /// @brief Calls callbackFunction() for every list until false is returned
    /// @param callbackFunction CallbackFunction*, Callback function that is executed for list items
    /// @param data void*, Function-specific data
    /// @returns true if every list item was processed
    bool each(CallbackFunction* callbackFunction, void* data=NULL)
    {
        CSynchronizedCode sc(m_sync);
        typename Map::iterator itor;
        for (itor = m_map.begin(); itor != m_map.end(); itor++) {
            if (!callbackFunction(itor->first, itor->second, data))
                return false;
        }
        return true;
    }
};

/// @}

}
#endif
