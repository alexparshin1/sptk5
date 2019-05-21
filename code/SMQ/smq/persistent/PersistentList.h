/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       PersistentList.cpp - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday May 19 2019                                     ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __PERSISTENT_LIST_H__
#define __PERSISTENT_LIST_H__

#include <sptk5/threads/Locks.h>
#include <smq/persistent/MemoryPool.h>
#include <list>

namespace smq {
namespace persistent {

class PersistentList
{
    struct Header
    {
        HandleStorage       first;
        uint32_t            nameLength {0};
        char                name[1];
    };

    struct Item
    {
        HandleStorage       prior;
        HandleStorage       next;
        uint32_t            dataLength {0};
        uint8_t             data[1];
    };

    mutable sptk::SharedMutex   m_mutex;
    MemoryPool&                 m_pool;
    Handle                      m_header;
    std::list<Handle>           m_handles;

public:

    PersistentList(MemoryPool& pool, const sptk::String& name);

    typedef std::list<Handle>::iterator         iterator;
    typedef std::list<Handle>::const_iterator   const_iterator;

    Handle push_front(const void* data, size_t size);

    Handle push_back(const void* data, size_t size);

    iterator begin()
    {
        sptk::UniqueLock(m_mutex);
        return m_handles.begin();
    }

    iterator end()
    {
        sptk::UniqueLock(m_mutex);
        return m_handles.end();
    }

    const_iterator begin() const
    {
        sptk::SharedLock(m_mutex);
        return m_handles.begin();
    }

    const_iterator end() const
    {
        sptk::SharedLock(m_mutex);
        return m_handles.end();
    }

    /**
     * @brief Returns true if the list is empty
     */
    bool empty() const
    {
        sptk::SharedLock(m_mutex);
        return m_handles.empty();
    }

    /**
     * @brief Returns number of items in the list
     */
    size_t size() const
    {
        sptk::SharedLock(m_mutex);
        return m_handles.size();
    }

    void erase(iterator from, iterator to);

    /**
     * @brief Removes all items from the list
     */
    void clear();

};

}
}

#endif
