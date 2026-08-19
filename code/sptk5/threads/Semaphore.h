/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/DateTime.h>
#include <sptk5/Exception.h>
#include <sptk5/sptk.h>

#include <chrono>
#include <limits>
#include <semaphore>

#ifdef max
#undef max
#endif

namespace sptk {
/**
 * @addtogroup threads Thread Classes.
 * @{
 */

/**
 * @brief Generic unnamed semaphore class.
 */
class SP_EXPORT Semaphore
{
public:
    /**
     * @brief Constructor.
     * @param initialValue      Initial semaphore value.
     */
    explicit Semaphore(const size_t initialValue = 0)
        : m_value(initialValue > MaxSemaphoreValue
                      ? MaxSemaphoreValue
                      : static_cast<ptrdiff_t>(initialValue))
    {
    }

    /**
     * @brief Post the semaphore.
     *
     * The semaphore value is increased by count.
     * @param count             Count to increase the semaphore.
     */
    void post(const size_t count = 1)
    {
        m_value.release(static_cast<ptrdiff_t>(count & MaxSemaphoreValue));
    }

    /**
     * @brief Check if the semaphore value is greater than zero.
     *
     * If the semaphore value is greater than zero, decreases the semaphore value by one and returns true.
     * Otherwise, exits immediately.
     * @return true if semaphore was posted (signaled).
     */
    bool check()
    {
        return m_value.try_acquire();
    }

    /**
     * @brief Wait until the semaphore value is greater than zero.
     *
     * Decreases semaphore value by one and returns true.
     */
    void wait()
    {
        m_value.acquire();
    }

    /**
     * @brief Wait until semaphore value is greater than zero, or until the timeout interval is passed.
     *
     * If the semaphore value is greater than zero, decreases the semaphore value by one and returns true.
     * @param timeout           Wait timeout.
     * @return true if semaphore was posted (signaled), or false if timeout occurs.
     */
    bool wait_for(const std::chrono::microseconds timeout)
    {
        return m_value.try_acquire_for(timeout);
    }

    /**
     * @brief Wait until semaphore value is greater than zero, or until timeoutAt occurs.
     *
     * If the semaphore value is greater than zero, decreases the semaphore value by one and returns true.
     * @param timeoutAt           Timeout moment.
     * @return true if semaphore was posted (signaled), or false if timeout occurs.
     */
    bool wait_until(const DateTime& timeoutAt)
    {
        return wait_until(timeoutAt.timePoint());
    }

    /**
     * @brief Wait until semaphore value is greater than zero, or until timeoutAt occurs.
     *
     * If the semaphore value is greater than zero, decreases the semaphore value by one and returns true.
     * @param timeoutAt           Timeout moment.
     * @return true if semaphore was posted (signaled), or false if timeout occurs.
     */
    bool wait_until(const DateTime::time_point& timeoutAt)
    {
        return m_value.try_acquire_until(timeoutAt);
    }

private:
    static constexpr auto                      MaxSemaphoreValue = std::numeric_limits<int>::max();
    std::counting_semaphore<MaxSemaphoreValue> m_value;
};
/**
 * @}
 */
} // namespace sptk
