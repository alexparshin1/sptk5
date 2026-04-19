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
#include <sptk5/DateTime.h>
#include <sptk5/sptk.h>

namespace sptk {
/**
 * @addtogroup threads Thread Classes.
 * @{
 */

/**
 * @brief The generic thread-safe counter.
 */
class SP_EXPORT Counter final
{
public:
    /**
     * @brief Constructor.
     *
     * Creates a counter with the starting value.
     * @param startingValue     The starting counter's value.
     */
    explicit Counter(int startingValue = 0);

    /**
     * @brief Destructor.
     */
    ~Counter();

    /**
     * @brief Get the counter's value.
     */
    int get() const;

    /**
     * @brief Set the counter's value.
     * @param value             New counter value.
     */
    void set(int value);

    /**
     * @brief Increment the counter's value.
     * @return new counter's value.
     */
    Counter& operator++();

    /**
     * @brief Increment the counter's value.
     * @return new counter's value.
     */
    Counter& operator++(int);

    /**
     * @brief Increment the counter's value.
     * @param value             Increment value.
     * @return new counter's value.
     */
    Counter& operator+=(int value);

    /**
     * @brief Decrement the counter's value.
     * @return new counter's value.
     */
    Counter& operator--();

    /**
     * @brief Decrement the counter's value.
     * @return new counter's value.
     */
    Counter& operator--(int);

    /**
     * @brief Decrement the counter's value.
     * @param value             Decrement value.
     * @return new counter's value.
     */
    Counter& operator-=(int value);

    /**
     * @brief Adaptor.
     */
    operator int() const
    {
        return get();
    }

    /**
     * @brief Assignment.
     */
    Counter& operator=(int value)
    {
        set(value);
        return *this;
    }

    /**
     * @brief Wait until the counter has the value.
     * @param value             Value to wait for.
     * @param timeout           Wait timeout.
     * @return true if counter received the value, or false if timeout occurs.
     */
    bool wait_for(int value, const std::chrono::milliseconds& timeout);

    /**
     * @brief Wait until the counter has the value.
     * @param value             Value to wait for.
     * @param timeoutAt           Wait timeout.
     * @return true if counter received the value, or false if timeout occurs.
     */
    [[maybe_unused]] bool wait_until(int value, const DateTime& timeoutAt);

    auto operator<=>(const Counter& rhs) const
    {
        std::scoped_lock lock(m_lockMutex, rhs.m_lockMutex);
        return m_counter <=> rhs.m_counter;
    }

    auto operator==(const Counter& rhs) const
    {
        std::scoped_lock lock(m_lockMutex, rhs.m_lockMutex);
        return m_counter == rhs.m_counter;
    }

    auto operator<=>(const int rhs) const
    {
        std::scoped_lock lock(m_lockMutex);
        return m_counter <=> rhs;
    }

    auto operator==(const int rhs) const
    {
        std::scoped_lock lock(m_lockMutex);
        return m_counter == rhs;
    }

private:
    mutable std::mutex      m_lockMutex;       ///< Mutex that protects counter's operations.
    std::condition_variable m_condition;       ///< Mutex condition.
    int                     m_counter {0}; ///< Counter value.
};
/**
 * @}
 */
} // namespace sptk
