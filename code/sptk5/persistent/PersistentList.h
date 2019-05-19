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
#include <sptk5/persistent/MemoryPool.h>
#include <list>

namespace sptk {
namespace persistent {

template <class Struct>
class PersistentList
{
    struct Item
    {
        Struct  data;
        Handle  prior;
        Handle  next;
    };

    SharedMutex         m_mutex;
    MemoryPool          m_pool;
    std::list<Handle>   m_handles;

public:

    PersistentList(MemoryPool& pool) : m_pool(pool) {}

    typedef std::list<Handle>::iterator         iterator;
    typedef std::list<Handle>::const_iterator   const_iterator;

    Handle push_front(const Struct& data)
    {
        UniqueLock(m_mutex);

        Item item;

        if (!m_handles.empty())
            item.next = m_handles.front();

        memcpy(&item.data, data, sizeof(Struct));
        Handle handle = m_pool.insert(&item, sizeof(Item));

        if (!m_handles.empty()) {
            Handle& front = m_handles.front();
            Item* frontItem = (Item*) front.data();
            frontItem->prior = handle;
        }

        m_handles.push_front(handle);

        return m_handles.front();
    }

    Handle push_back(const Struct& data)
    {
        UniqueLock(m_mutex);

        Item item;

        if (!m_handles.empty())
            item.prior = m_handles.back();

        memcpy(&item.data, data, sizeof(Struct));
        Handle handle = m_pool.insert(&item, sizeof(Item));

        if (!m_handles.empty()) {
            Handle& back = m_handles.back();
            Item* backItem = (Item*) back.data();
            backItem->next = handle;
        }

        m_handles.push_front(handle);

        return m_handles.back();
    }

    iterator begin()
    {
        UniqueLock(m_mutex);
        return m_handles.begin();
    }

    iterator end()
    {
        UniqueLock(m_mutex);
        return m_handles.end();
    }

    const_iterator begin() const
    {
        SharedLock(m_mutex);
        return m_handles.begin();
    }

    const_iterator end() const
    {
        SharedLock(m_mutex);
        return m_handles.end();
    }

    /**
     * @brief Returns true if the list is empty
     */
    bool empty() const
    {
        SharedLock(m_mutex);
        return m_handles.empty();
    }

    /**
     * @brief Returns number of items in the list
     */
    size_t size() const
    {
        SharedLock(m_mutex);
        return m_handles.size();
    }

    void erase(iterator from, iterator to)
    {
        UniqueLock(m_mutex);
        for(auto& itor = from; itor != to; itor++) {
            auto& handle = *itor;
            m_pool.free(handle);
        }

        auto& prior = from;
        Item* priorItem;
        if (from != m_handles.begin()) {
            prior--;
            priorItem = (Item*) prior->data();
        } else {
            prior = m_handles.end();
            priorItem = nullptr;
        }

        auto& next = m_handles.erase(from, to);
        Item* nextItem;
        if (next != m_handles.end())
            nextItem = (Item*) next->data();
        else
            nextItem = nullptr;

        if (priorItem != nullptr) {
            if (nextItem != nullptr)
                priorItem->next = *next;
            else
                priorItem->next = Handle();
        }

        if (nextItem != nullptr) {
            if (priorItem != nullptr)
                nextItem->prior = *prior;
            else
                nextItem->prior = Handle();
        }
    }

    /**
     * @brief Removes all items from the list
     */
    void clear()
    {
        UniqueLock(m_mutex);
        for (auto& handle: m_handles)
            m_pool.free(handle);
        m_handles.clear();
    }

};

}
}

#endif
