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

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <vector>

#include <sptk5/Printer.h>
#include <sptk5/threads/Semaphore.h>

namespace sptk {
/**
 * @addtogroup threads Thread Classes.
 * @{
 */

/**
 * @brief Synchronized template queue.
 *
 * Simple thread-safe queue.
 */
template<class T>
class SynchronizedQueue
{
public:
    /**
     * @brief Destructor.
     */
    virtual ~SynchronizedQueue() = default;

    /**
     * @brief Pushes a data item to the queue.
     *
     * Item is moved inside the queue.
     * Automatically posts internal semaphore to indicate.
     * queue item availability.
     * @param data T&&, A data item.
     */
    void push_back(T&& data)
    {
        {
            std::scoped_lock lock(m_mutex);
            m_queue.push_back(std::move(data));
        }
        m_condition.notify_one();
    }

    /**
     * @brief Pushes a data item to the queue.
     *
     * Item is moved inside the queue.
     * Automatically posts internal semaphore to indicate.
     * queue item availability.
     * @param data T&&, A data item.
     */
    [[maybe_unused]] void push_front(T&& data)
    {
        {
            std::scoped_lock lock(m_mutex);
            m_queue.push_front(std::move(data));
        }
        m_condition.notify_one();
    }

    /**
     * @brief Pushes a data item to the queue.
     *
     * Automatically posts internal semaphore to indicate queue item availability.
     * @param data const T&, A data item.
     */
    void push_back(const T& data)
    {
        {
            std::scoped_lock lock(m_mutex);
            m_queue.push_back(data);
        }
        m_condition.notify_one();
    }

    /**
     * @brief Pushes a data item to the queue.
     *
     * Automatically posts internal semaphore to indicate queue item availability.
     * @param data const T&, A data item.
     */
    [[maybe_unused]] void push_front(const T& data)
    {
        {
            std::scoped_lock lock(m_mutex);
            m_queue.push_front(data);
        }
        m_condition.notify_one();
    }

    /**
     * @brief Pops a data item from the queue.
     *
     * If the queue is empty, then wait until timeoutMS milliseconds timeout occurs.
     * Returns false if timeout occurs.
     * @param item T&, A queue item (output).
     * @param timeout std::chrono::milliseconds, Operation timeout in milliseconds.
     */
    bool pop_front(T& item, const std::chrono::milliseconds& timeout)
    {
        std::unique_lock lock(m_mutex);
        if (!m_condition.wait_for(lock, timeout, [this] { return !m_queue.empty() || m_wakeups > 0; }))
        {
            return false;
        }
        if (m_queue.empty())
        {
            if (m_wakeups > 0)
            {
                --m_wakeups;
            }
            return false;
        }
        item = std::move(m_queue.front());
        m_queue.pop_front();
        return true;
    }

    /**
     * @brief Pops multiple data items from the queue.
     *
     * If the queue is empty, then wait until timeoutMS milliseconds timeout occurs.
     * If the queue has fewer items than requested, then returns all available items.
     * Returns false if timeout occurs.
     * @param items             A queue items (output).
     * @param itemCount         Maximum number of items to pop.
     * @param timeout           Operation timeout in milliseconds.
     */
    bool pop_front(std::vector<T>& items, size_t itemCount, const std::chrono::milliseconds& timeout)
    {
        std::unique_lock lock(m_mutex);
        if (!m_condition.wait_for(lock, timeout, [this] { return !m_queue.empty() || m_wakeups > 0; }))
        {
            return false;
        }
        if (m_queue.empty())
        {
            if (m_wakeups > 0)
            {
                --m_wakeups;
            }
            return false;
        }
        items.clear();
        while (!m_queue.empty() && itemCount > 0)
        {
            items.push_back(std::move(m_queue.front()));
            m_queue.pop_front();
            --itemCount;
        }
        return !items.empty();
    }

    /**
     * @brief Pops a data item from the queue.
     *
     * If the queue is empty, then wait until timeoutMS milliseconds timeout occurs.
     * Returns false if timeout occurs.
     * @param item T&, A queue item (output).
     * @param timeout std::chrono::milliseconds, Operation timeout in milliseconds.
     */
    [[maybe_unused]] bool pop_back(T& item, const std::chrono::milliseconds& timeout)
    {
        std::unique_lock lock(m_mutex);
        if (!m_condition.wait_for(lock, timeout, [this] { return !m_queue.empty() || m_wakeups > 0; }))
        {
            return false;
        }
        if (m_queue.empty())
        {
            if (m_wakeups > 0)
            {
                --m_wakeups;
            }
            return false;
        }
        item = std::move(m_queue.back());
        m_queue.pop_back();
        return true;
    }

    /**
     * @brief Pushes a data item to the back of the queue with construction in-place.
     * @param arguments            Constructor arguments.
     */
    template<typename... Arguments>
    void emplace_back(Arguments&&... arguments)
    {
        {
            std::scoped_lock lock(m_mutex);
            m_queue.emplace_back(std::forward<Arguments>(arguments)...);
        }
        m_condition.notify_one();
    }

    /**
     * @brief Pushes a data item to the front of the queue with construction in-place.
     * @param arguments            Constructor arguments.
     */
    template<typename... Arguments>
    void emplace_front(Arguments&&... arguments)
    {
        {
            std::scoped_lock lock(m_mutex);
            m_queue.emplace_front(std::forward<Arguments>(arguments)...);
        }
        m_condition.notify_one();
    }

    /**
     * @brief Wakes up queue semaphore to interrupt waiting.
     *
     * Any waiting pop() operation immediately returns false.
     */
    virtual void wakeup()
    {
        {
            std::scoped_lock lock(m_mutex);
            ++m_wakeups;
        }
        m_condition.notify_one();
    }

    /**
     * @brief Returns true if the queue is empty.
     */
    bool empty() const
    {
        std::scoped_lock lock(m_mutex);
        return m_queue.empty();
    }

    /**
     * @brief Returns the number of items in the queue.
     */
    size_t size() const
    {
        std::scoped_lock lock(m_mutex);
        return m_queue.size();
    }

    /**
     * @brief Removes all items from the queue.
     */
    void clear()
    {
        std::scoped_lock lock(m_mutex);
        m_queue.clear();
    }

    /**
     * @brief Calls callbackFunction() for every list until false is returned.
     *
     * The current implementation does the job but isn't too efficient due to std::deque class limitations.
     * @param callbackFunction  Callback function that is executed for list items.
     * @returns true if every list item was processed.
     */
    template<typename CallbackFunction>
    bool each(const CallbackFunction& callbackFunction)
    {
        std::scoped_lock lock(m_mutex);

        // Iterating through the queue until the callback returns false.
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
     * @brief Lock to synchronize queue operations.
     */
    mutable std::mutex m_mutex;

    /**
     * @brief Condition variable to wait for an item if the queue is empty.
     */
    std::condition_variable m_condition;

    /**
     * @brief Pending one-shot wakeups, each releasing one waiting pop with false.
     */
    size_t m_wakeups = 0;

    /**
     * @brief Queue.
     */
    std::deque<T> m_queue;
};
/**
 * @}
 */
} // namespace sptk
