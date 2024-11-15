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

#include <functional>
#include <list>
#include <mutex>
#include <sptk5/sptk.h>
#include <sptk5/threads/Semaphore.h>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * Synchronized template list
 *
 * Simple thread-safe list
 */
template<class T>
class SynchronizedList
{
public:
    /**
     * @brief List callback function used in each() method.
     *
     * Iterates through list until false is returned.
     * @param item T&, List item
     * @param data void*, Optional function-specific data
     */
    using CallbackFunction = std::function<bool(T& item)>;

    /**
     * @brief Default constructor
     */
    SynchronizedList() = default;

    /**
     * @brief Copy constructor
     * @param other             other list
     */
    SynchronizedList(const SynchronizedList& other)
    {
        std::scoped_lock lock(other.m_mutex);
        m_list = other.m_list;
    }

    /**
     * @brief Move constructor
     * @param other             other list
     */
    SynchronizedList(SynchronizedList&& other) noexcept
    {
        std::scoped_lock lock(other.m_mutex);
        m_list = std::move(other.m_list);
    }

    /**
     * @brief Copy assignment
     * @param other             other list
     */
    SynchronizedList& operator=(const SynchronizedList& other)
    {
        if (this != &other)
        {
            std::scoped_lock lock(m_mutex, other.m_mutex);
            m_list = other.m_list;
        }
    }

    /**
     * @brief Copy assignment
     * @param other             other list
     */
    SynchronizedList& operator=(SynchronizedList&& other) noexcept
    {
        if (this != &other)
        {
            std::scoped_lock lock(m_mutex, other.m_mutex);
            m_list = std::move(other.m_list);
        }
    }

    /**
     * @brief Destructor
     */
    virtual ~SynchronizedList() = default;

    /**
     * @brief Pushes a data item to the list front
     *
     * Automatically posts internal semaphore to indicate
     * list item availability.
     * @param data const T&, A data item
     */
    virtual void push_front(const T& data)
    {
        std::scoped_lock lock(m_mutex);
        m_list.push_front(data);
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
    virtual bool pop_front(T& item, const std::chrono::milliseconds& timeout)
    {
        if (m_semaphore.wait_for(timeout))
        {
            std::scoped_lock lock(m_mutex);
            if (!m_list.empty())
            {
                item = m_list.front();
                m_list.pop_front();
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
        std::scoped_lock lock(m_mutex);
        m_list.push_back(data);
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
    virtual bool pop_back(T& item, const std::chrono::milliseconds& timeout)
    {
        if (m_semaphore.wait_for(timeout))
        {
            std::scoped_lock lock(m_mutex);
            if (!m_list.empty())
            {
                item = m_list.back();
                m_list.pop_back();
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
        std::scoped_lock lock(m_mutex);
        m_list.remove(item);
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
        std::scoped_lock lock(m_mutex);
        return m_list.empty();
    }

    /**
     * @brief Returns number of items in the list
     */
    size_t size() const
    {
        std::scoped_lock lock(m_mutex);
        return m_list.size();
    }

    /**
     * @brief Removes all items from the list
     */
    void clear()
    {
        std::scoped_lock lock(m_mutex);
        m_list.clear();
    }

    /**
     * @brief Calls for every list until false is returned
     * @param callbackFunction  Callback function that is executed for list items
     * @param data              Function-specific data
     * @returns true if every list item was processed
     */
    bool each(const CallbackFunction& callbackFunction)
    {
        std::scoped_lock lock(m_mutex);
        for (auto& item: m_list)
        {
            if (!callbackFunction(item))
            {
                return false;
            }
        }
        return true;
    }

private:
    mutable std::mutex m_mutex;     ///< Lock to synchronize list operations
    Semaphore          m_semaphore; ///< Semaphore to waiting for an item if list is empty
    std::list<T>       m_list;      ///< List
};

/**
 * @}
 */

} // namespace sptk
