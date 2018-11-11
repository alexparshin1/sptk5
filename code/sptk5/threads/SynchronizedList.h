/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SynchronizedList.h - description                       ║
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

#ifndef __SYNCHRONIZED_LIST_H__
#define __SYNCHRONIZED_LIST_H__

#include <sptk5/sptk.h>
#include <sptk5/threads/Semaphore.h>
#include <list>
#include "Locks.h"

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * @brief Synchronized template list
 *
 * Simple thread-safe list
 */
template <class T>
class SynchronizedList
{
    /**
     * Semaphore to waiting for an item if list is empty
     */
    Semaphore               m_semaphore;

    /**
     * List
     */
    std::list<T>*           m_list;


protected:

    /**
     * Lock to synchronize list operations
     */
    mutable SharedMutex     m_sync;


public:

    /**
     * @brief List callback function used in each() method.
     *
     * Iterates through list until false is returned.
     * @param item T&, List item
     * @param data void*, Optional function-specific data
     */
    typedef bool (CallbackFunction)(T& item, void* data);

public:

    /**
     * @brief Default constructor
     */
    SynchronizedList() :
        m_list(new std::list<T>)
    {}

    /**
     * @brief Destructor
     */
    virtual ~SynchronizedList()
    {
        delete m_list;
    }

    /**
     * @brief Pushes a data item to the list front
     *
     * Automatically posts internal semaphore to indicate
     * list item availability.
     * @param data const T&, A data item
     */
    virtual void push_front(const T& data)
    {
        UniqueLock(m_sync);
        m_list->push_front(data);
        m_semaphore.post();
    }

    /**
     * @brief Pops a data item from the list front
     *
     * If list is empty then waits until timeout milliseconds occurs.
     * Returns false if timeout occurs.
     * @param item T&, A list item (output)
     * @param timeout std::chrono::milliseconds, Operation timeout
     */
    virtual bool pop_front(T& item, std::chrono::milliseconds timeout)
    {
        if (m_semaphore.sleep_for(timeout)) {
            UniqueLock(m_sync);
            if (!m_list->empty()) {
                item = m_list->front();
                m_list->pop_front();
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Pushes a data item to the list back
     *
     * Automatically posts internal semaphore to indicate
     * list item availability.
     * @param data const T&, A data item
     */
    virtual void push_back(const T& data)
    {
        UniqueLock(m_sync);
        m_list->push_back(data);
        m_semaphore.post();
    }

    /**
     * @brief Pops a data item from the list back
     *
     * If list is empty then waits until timeout occurs.
     * Returns false if timeout occurs.
     * @param item T&, A list item (output)
     * @param timeout std::chrono::milliseconds, Operation timeout
     */
    virtual bool pop_back(T& item, std::chrono::milliseconds timeout)
    {
        if (m_semaphore.sleep_for(timeout)) {
            UniqueLock(m_sync);
            if (!m_list->empty()) {
                item = m_list->back();
                m_list->pop_back();
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Removes all elements with the specific value from the list
     */
    virtual void remove(T& item)
    {
        UniqueLock(m_sync);
        m_list->remove(item);
    }

    /**
     * @brief Wakes up list semaphore to interrupt waiting
     *
     * Any waiting pop() operation immediately returns false.
     */
    virtual void wakeup()
    {
        m_semaphore.post();
    }

    /**
     * @brief Returns true if the list is empty
     */
    bool empty() const
    {
        SharedLock(m_sync);
        return m_list->empty();
    }

    /**
     * @brief Returns number of items in the list
     */
    size_t size() const
    {
        SharedLock(m_sync);
        return m_list->size();
    }

    /**
     * @brief Removes all items from the list
     */
    void clear()
    {
        UniqueLock(m_sync);
        m_list->clear();
    }

    /**
     * @brief Calls callbackFunction() for every list until false is returned
     * @param callbackFunction CallbackFunction*, Callback function that is executed for list items
     * @param data void*, Function-specific data
     * @returns true if every list item was processed
     */
    bool each(CallbackFunction* callbackFunction, void* data=NULL)
    {
        UniqueLock(m_sync);
        typename std::list<T>::iterator itor;
        for (itor = m_list->begin(); itor != m_list->end(); ++itor) {
            if (!callbackFunction(*itor, data))
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
