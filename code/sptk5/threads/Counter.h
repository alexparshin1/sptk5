/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/Exception.h>
#include <sptk5/sptk.h>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * @brief Generic counter class
 */
class SP_EXPORT Counter
{
public:
    /**
     * @brief Constructor
     *
     * Creates counter with starting value
     * @param startingValue     Starting counter value
     */
    explicit Counter(size_t startingValue = 0);

    /**
     * @brief Destructor
     */
    virtual ~Counter();

    /**
     * @brief Get the counter value
     */
    size_t get() const;

    /**
     * @brief Set the counter value
     * @param value             New counter value
     */
    void set(size_t value);

    /**
     * @brief Increment the counter value
     * @param value             Increment value
     * @return new counter value
     */
    size_t increment(size_t value = 1);

    /**
     * @brief Increment the counter value
     * @param value             Increment value
     * @return new counter value
     */
    size_t decrement(size_t value = 1);

    /**
     * @brief Adaptor
     */
    operator size_t() const
    {
        return get();
    }

    /**
     * @brief Assignment
     */
    Counter& operator=(size_t value)
    {
        set(value);
        return *this;
    }

    /**
     * @brief Wait until the counter has the value
     * @param value             Value to wait for
     * @param timeout           Wait timeout
     * @return true if counter received the value, or false if timeout occurs
     */
    bool wait_for(size_t value, std::chrono::milliseconds timeout);

    /**
     * @brief Wait until the counter has the value
     * @param value             Value to wait for
     * @param timeoutAt           Wait timeout
     * @return true if counter received the value, or false if timeout occurs
     */
    [[maybe_unused]] bool wait_until(size_t value, const DateTime& timeoutAt);

private:
    mutable std::mutex m_lockMutex;      ///< Mutex that protects counter operations
    std::condition_variable m_condition; ///< Mutex condition
    size_t m_counter {false};            ///< Counter value
};
/**
 * @}
 */
} // namespace sptk
