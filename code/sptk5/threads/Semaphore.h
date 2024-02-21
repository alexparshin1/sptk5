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

#include <sptk5/DateTime.h>
#include <sptk5/Exception.h>
#include <sptk5/sptk.h>

#include <chrono>
#include <semaphore>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * @brief Generic unnamed semaphore class
 */
class SP_EXPORT Semaphore
{
public:
    /**
     * @brief Constructor
     * @param initialValue      Initial semaphore value
     */
    explicit Semaphore(int initialValue = 0)
        : m_value(initialValue)
    {
    }

    /**
     * @brief Post the semaphore
     *
     * The semaphore value is increased by count.
     * @param count             Count to increase the semaphore.
     */
    void post(size_t count = 1)
    {
        m_value.release((ptrdiff_t) count);
    }

    /**
     * @brief Check if semaphore value is greater than zero
     *
     * If semaphore value is greater than zero, decreases semaphore value by one and returns true.
     * Otherwise, exits immediately.
     * @return true if semaphore was posted (signaled)
     */
    bool check()
    {
        return m_value.try_acquire();
    }

    /**
     * @brief Wait until semaphore value is greater than zero
     *
     * Decreases semaphore value by one and returns true.
     */
    void wait()
    {
        m_value.acquire();
    }

    /**
     * @brief Wait until semaphore value is greater than zero, or until timeout interval is passed
     *
     * If semaphore value is greater than zero, decreases semaphore value by one and returns true.
     * @param timeout           Wait timeout
     * @return true if semaphore was posted (signaled), or false if timeout occurs
     */
    bool wait_for(std::chrono::microseconds timeout)
    {
        return m_value.try_acquire_for(timeout);
    }

    /**
     * @brief Wait until semaphore value is greater than zero, or until timeoutAt occurs
     *
     * If semaphore value is greater than zero, decreases semaphore value by one and returns true.
     * @param timeoutAt           Timeout moment
     * @return true if semaphore was posted (signaled), or false if timeout occurs
     */
    bool wait_until(const DateTime& timeoutAt)
    {
        return wait_until(timeoutAt.timePoint());
    }

    /**
     * @brief Wait until semaphore value is greater than zero, or until timeoutAt occurs
     *
     * If semaphore value is greater than zero, decreases semaphore value by one and returns true.
     * @param timeoutAt           Timeout moment
     * @return true if semaphore was posted (signaled), or false if timeout occurs
     */
    bool wait_until(const DateTime::time_point& timeoutAt)
    {
        return m_value.try_acquire_until(timeoutAt);
    }

private:
    static constexpr auto MaxSemaphoreValue = 0x7FFFFFFF;
    std::counting_semaphore<MaxSemaphoreValue> m_value;
};
/**
 * @}
 */
} // namespace sptk
