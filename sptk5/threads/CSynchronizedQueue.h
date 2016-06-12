/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CSynchronizedQueue.h - description                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __CSYNCHRONIZEDQUEUE_H__
#define __CSYNCHRONIZEDQUEUE_H__

#include <sptk5/sptk.h>
#include <sptk5/threads/CSynchronizedCode.h>
#include <sptk5/threads/CSemaphore.h>
#include <queue>

namespace sptk {

/// @addtogroup threads Thread Classes
/// @{

/// @brief Synchronized template queue
///
/// Simple thread-safe queue
template <class T>
class CSynchronizedQueue
{
    CSemaphore              m_semaphore;        ///< Semaphore to waiting for an item if queue is empty
    std::queue<T>*          m_queue;            ///< Queue

protected:

    mutable CSynchronized   m_sync;             ///< Lock to synchronize queue operations

public:

    /// @brief Queue callback function used in each() method.
    ///
    /// Iterates through queue until false is returned.
    /// @param item T&, List item
    /// @param data void*, Optional function-specific data
    typedef bool (CallbackFunction)(T& item, void* data);

public:

    /// @brief Default constructor
    CSynchronizedQueue() :
        m_queue(new std::queue<T>)
    {}

    /// @brief Destructor
    virtual ~CSynchronizedQueue()
    {
        delete m_queue;
    }

    /// @brief Pushes a data item to the queue
    ///
    /// Automatically posts internal semaphore to indicate
    /// queue item availability.
    /// @param data const T&, A data item
    void push(const T& data)
    {
        CSynchronizedCode sc(m_sync);
        m_queue->push(data);
        m_semaphore.post();
    }

    /// @brief Pops a data item from the queue
    ///
    /// If queue is empty then waits until timeoutMS milliseconds timeout occurs.
    /// Returns false if timeout occurs.
    /// @param item T&, A queue item (output)
    /// @param timeoutMS int32_t, Operation timeout in milliseconds
    bool pop(T& item, int32_t timeoutMS)
    {
        if (m_semaphore.wait(uint32_t(timeoutMS))) {
            CSynchronizedCode sc(m_sync);
            if (!m_queue->empty()) {
                item = m_queue->front();
                m_queue->pop();
                return true;
            }
        }
        return false;
    }

    /// @brief Wakes up queue semaphore to interrupt waiting
    ///
    /// Any waiting pop() operation immediately returns false.
    virtual void wakeup()
    {
        m_semaphore.post();
    }

    /// @brief Returns true if the queue is empty
    bool empty() const
    {
        CSynchronizedCode sc(m_sync);
        return m_queue->empty();
    }

    /// @brief Returns number of items in the queue
    uint32_t size() const
    {
        CSynchronizedCode sc(m_sync);
        return m_queue->size();
    }

    /// @brief Removes all items from the queue
    void clear()
    {
        CSynchronizedCode sc(m_sync);
        delete m_queue;
        m_queue = new std::queue<T>;
    }

    /// @brief Calls callbackFunction() for every list until false is returned
    ///
    /// Current implementation does the job but isn't too efficient due to
    /// std::queue class limitations.
    /// @param callbackFunction CallbackFunction*, Callback function that is executed for list items
    /// @param data void*, Function-specific data
    /// @returns true if every list item was processed
    bool each(CallbackFunction* callbackFunction, void* data=NULL)
    {
        CSynchronizedCode sc(m_sync);

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
/// @}
}
#endif
