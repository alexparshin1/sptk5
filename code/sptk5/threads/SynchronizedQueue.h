/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <deque>
#include <functional>
#include <mutex>
#include <sptk5/Exception.h>
#include <sptk5/Printer.h>
#include <sptk5/threads/Semaphore.h>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * Synchronized template queue
 *
 * Simple thread-safe queue
 */
template<class T>
class SynchronizedQueue
{
public:
    /**
     * @brief Destructor
     */
    virtual ~SynchronizedQueue() = default;

    /**
     * @brief Pushes a data item to the queue
     *
     * Item is moved inside the queue.
     * Automatically posts internal semaphore to indicate
     * queue item availability.
     * @param data T&&, A data item
     */
    void push_back(T&& data)
    {
        std::unique_lock lock(m_mutex);
        m_queue.push_back(std::move(data));
        lock.unlock();
        m_semaphore.post();
    }

    /**
     * @brief Pushes a data item to the queue
     *
     * Item is moved inside the queue.
     * Automatically posts internal semaphore to indicate
     * queue item availability.
     * @param data T&&, A data item
     */
    [[maybe_unused]] void push_front(T&& data)
    {
        std::unique_lock lock(m_mutex);
        m_queue.push_front(std::move(data));
        lock.unlock();
        m_semaphore.post();
    }

    /**
     * @brief Pushes a data item to the queue
     *
     * Automatically posts internal semaphore to indicate
     * queue item availability.
     * @param data const T&, A data item
     */
    void push_back(const T& data)
    {
        std::unique_lock lock(m_mutex);
        m_queue.push_back(data);
        lock.unlock();
        m_semaphore.post();
    }

    /**
     * @brief Pushes a data item to the queue
     *
     * Automatically posts internal semaphore to indicate
     * queue item availability.
     * @param data const T&, A data item
     */
    [[maybe_unused]] void push_front(const T& data)
    {
        std::unique_lock lock(m_mutex);
        m_queue.push_front(data);
        lock.unlock();
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
    bool pop_front(T& item, const std::chrono::milliseconds& timeout)
    {
        if (m_semaphore.wait_for(timeout))
        {
            std::unique_lock lock(m_mutex);
            if (!m_queue.empty())
            {
                item = std::move(m_queue.front());
                m_queue.pop_front();
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Pops a data item from the queue
     *
     * If queue is empty then waits until timeoutMS milliseconds timeout occurs.
     * Returns false if timeout occurs.
     * @param item T&, A queue item (output)
     * @param timeout std::chrono::milliseconds, Operation timeout in milliseconds
     */
    [[maybe_unused]] bool pop_back(T& item, const std::chrono::milliseconds& timeout)
    {
        if (m_semaphore.wait_for(timeout))
        {
            std::unique_lock lock(m_mutex);
            if (!m_queue.empty())
            {
                item = std::move(m_queue.back());
                m_queue.pop_back();
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
        std::scoped_lock lock(m_mutex);
        return m_queue.empty();
    }

    /**
     * @brief Returns number of items in the queue
     */
    size_t size() const
    {
        std::scoped_lock lock(m_mutex);
        return m_queue.size();
    }

    /**
     * @brief Removes all items from the queue
     */
    void clear()
    {
        std::scoped_lock lock(m_mutex);
        m_queue.clear();
    }

    /**
     * @brief Calls callbackFunction() for every list until false is returned
     *
     * Current implementation does the job but isn't too efficient due to
     * std::deque class limitations.
     * @param callbackFunction  Callback function that is executed for list items
     * @param data              Function-specific data
     * @returns true if every list item was processed
     */
    template<typename CallbackFunction>
    bool each(const CallbackFunction& callbackFunction)
    {
        std::scoped_lock lock(m_mutex);

        // Iterating through queue until callback returns false
        bool rc = true;
        for (auto& item: m_queue)
        {
            rc = callbackFunction(item);
            if (!rc)
            {
                break;
            }
        }

        return rc;
    }

private:
    /**
     * @brief Lock to synchronize queue operations
     */
    mutable std::mutex m_mutex;

    /**
     * @brief Semaphore to waiting for an item if queue is empty
     */
    Semaphore m_semaphore;

    /**
     * @brief Queue
     */
    std::deque<T> m_queue;
};
/**
 * @}
 */
} // namespace sptk
