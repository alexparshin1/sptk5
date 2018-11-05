/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SynchronizedQueue.h - description                      ║
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

#ifndef __SYNCHRONIZED_QUEUE_H__
#define __SYNCHRONIZED_QUEUE_H__

#include <sptk5/sptk.h>
#include <sptk5/threads/Locks.h>
#include <sptk5/threads/Semaphore.h>
#include <queue>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * @brief Synchronized template queue
 *
 * Simple thread-safe queue
 */
template <class T>
class SynchronizedQueue
{
    /**
     * Semaphore to waiting for an item if queue is empty
     */
    Semaphore               m_semaphore;

    /**
     * Queue
     */
    std::queue<T>*          m_queue;


protected:

    /**
     * Lock to synchronize queue operations
     */
    mutable SharedMutex     m_mutex;


public:

    /**
     * @brief Queue callback function used in each() method.
     *
     * Iterates through queue until false is returned.
     * @param item T&, List item
     * @param data void*, Optional function-specific data
     */
    typedef bool (CallbackFunction)(T& item, void* data);

public:

    /**
     * @brief Default constructor
     */
    SynchronizedQueue() noexcept
    : m_queue(new std::queue<T>)
    {}

    /**
     * @brief Destructor
     */
    virtual ~SynchronizedQueue()
    {
        UniqueLock(m_mutex);
        delete m_queue;
    }

    /**
     * @brief Pushes a data item to the queue
     *
     * Item is moved inside the queue.
     * Automatically posts internal semaphore to indicate
     * queue item availability.
     * @param data T&&, A data item
     */
    void push(T&& data)
    {
        UniqueLock(m_mutex);
        m_queue->push(std::move(data));
        m_semaphore.post();
    }

    /**
     * @brief Pushes a data item to the queue
     *
     * Automatically posts internal semaphore to indicate
     * queue item availability.
     * @param data const T&, A data item
     */
    void push(const T& data)
    {
        UniqueLock(m_mutex);
        m_queue->push(data);
        m_semaphore.post();
    }

    /**
     * @brief Pops a data item from the queue
     *
     * If queue is empty then waits until timeoutMS milliseconds timeout occurs.
     * Returns false if timeout occurs.
     * @param item T&, A queue item (output)
     * @param timeout std::chrono::milliseconds, Operation timeout in milliseconds
     */
    bool pop(T& item, std::chrono::milliseconds timeout)
    {
        if (m_semaphore.sleep_for(timeout)) {
            UniqueLock(m_mutex);
            if (!m_queue->empty()) {
                item = std::move(m_queue->front());
                m_queue->pop();
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Wakes up queue semaphore to interrupt waiting
     *
     * Any waiting pop() operation immediately returns false.
     */
    virtual void wakeup()
    {
        m_semaphore.post();
    }

    /**
     * @brief Returns true if the queue is empty
     */
    bool empty() const
    {
        SharedLock(m_mutex);
        return m_queue->empty();
    }

    /**
     * @brief Returns number of items in the queue
     */
    size_t size() const
    {
        SharedLock(m_mutex);
        return m_queue->size();
    }

    /**
     * @brief Removes all items from the queue
     */
    void clear()
    {
        UniqueLock(m_mutex);
        delete m_queue;
        m_queue = new std::queue<T>;
    }

    /**
     * @brief Calls callbackFunction() for every list until false is returned
     *
     * Current implementation does the job but isn't too efficient due to
     * std::queue class limitations.
     * @param callbackFunction CallbackFunction*, Callback function that is executed for list items
     * @param data void*, Function-specific data
     * @returns true if every list item was processed
     */
    bool each(CallbackFunction* callbackFunction, void* data=NULL)
    {
        UniqueLock(m_mutex);

        std::queue<T> newQueue = new std::queue<T>;

        // Iterating through queue until callback returns false
        bool rc = true;
        while (m_queue->size()) {
            T& item = m_queue->front();
            m_queue->pop();
            newQueue->push(item);
            // When rc switches to false, don't execute callback
            // for the remaining queue items
            if (rc) {
                try {
                    rc = callbackFunction(item, data);
                }
                catch (std::exception&) {
                    rc = false;
                }
            }
        }

        delete m_queue;
        m_queue = newQueue;

        return rc;
    }
};
/**
 * @}
 */
}
#endif
