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

#include <sptk5/sptk.h>

#include "blockingconcurrentqueue.h"

#include <deque>
#include <iterator>
#include <type_traits>
#include <vector>

namespace sptk {
/**
 * @addtogroup threads Thread Classes.
 * @{
 */

/**
 * @brief Synchronized template queue.
 *
 * Simple thread-safe queue. Uses BlockingConcurrentQueue by Cameron Desrochers.
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
     * @param data              A data item.
     */
    void push_back(T&& data)
    {
        m_queue.enqueue(std::move(data));
    }

    /**
     * @brief Pushes a data item to the queue.
     * @param data              A data item.
     */
    void push_back(const T& data)
    {
        m_queue.enqueue(data);
    }

    /**
     * @brief Pops a data item from the queue.
     * @param item              A queue item (output).
     * @param timeout           Operation timeout in microseconds.
     * @returns false if timeout occurs.
     */
    bool pop_front(T& item, const std::chrono::microseconds& timeout)
    {
        return m_queue.wait_dequeue_timed(item, timeout.count());
    }

    /**
     * @brief Pops a data item from the queue.
     * @param item              A queue item (output).
     * @param maxItems          Max number of items to retrieve.
     * @param timeout           Operation timeout in microseconds.
     * @returns false if timeout occurs.
     */
    bool pop_front(std::vector<T>& item, size_t maxItems, const std::chrono::microseconds& timeout)
    {
        size_t count = 0;
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            // Trivial T: default-construction is a cheap memset and the bulk dequeue can memcpy
            // into contiguous storage, so pre-sizing and writing through begin() is fastest.
            item.resize(maxItems);
            count = m_queue.wait_dequeue_bulk_timed(item.begin(), maxItems, timeout.count());
            item.resize(count);
        }
        else
        {
            // Non-trivial T: pre-sizing would default-construct maxItems objects every call and
            // destroy the surplus - pure waste when a poll returns few items or times out. Append
            // exactly `count` moved-in objects instead, reusing the vector's capacity across calls.
            item.clear();
            item.reserve(maxItems);
            count = m_queue.wait_dequeue_bulk_timed(std::back_inserter(item), maxItems, timeout.count());
        }
        return count > 0;
    }

    /**
     * @brief Pops a data item from the queue.
     * @param item              A queue item (output).
     * @param timeout           Operation timeout in microseconds.
     * @returns false if timeout occurs.
     */
    bool pop_front(T&& item, const std::chrono::microseconds& timeout)
    {
        return m_queue.wait_dequeue_timed(item, timeout.count());
    }

    /**
     * @brief Pushes a data item to the back of the queue with construction in-place.
     * @param arguments            Constructor arguments.
     */
    template<typename... Arguments>
    void emplace_back(Arguments&&... arguments)
    {
        m_queue.enqueue(T(std::forward<Arguments>(arguments)...));
    }

    /**
     * @brief Returns true if the queue is empty.
     */
    bool empty() const
    {
        return m_queue.size_approx() == 0;
    }

    /**
     * @brief Returns the number of items in the queue.
     */
    size_t size() const
    {
        return m_queue.size_approx();
    }

    /**
     * @brief Removes all items from the queue.
     */
    void clear()
    {
        T item;
        while (m_queue.try_dequeue(item))
        {
        }
    }

    void wakeup()
    {
        m_queue.wake_up();
    }

private:
    moodycamel::BlockingConcurrentQueue<T> m_queue;
};
/**
 * @}
 */
} // namespace sptk
