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
 * @brief Thread-safe flag class.
 */
class SP_EXPORT Flag
{
public:
    /**
     * @brief Constructor.
     *
     * Creates the flag with the starting value (default false).
     * @param startingValue     Starting semaphore value.
     */
    explicit Flag(bool startingValue = false);

    /**
     * @brief Destructor.
     */
    virtual ~Flag();

    /**
     * @brief Get the flag value.
     * @return the flag value.
     */
    bool get() const;

    /**
     * @brief Set the flag value.
     * @param value             New flag value.
     */
    void set(bool value);

    /**
     * @brief Adaptor.
     */
    operator bool() const
    {
        return get();
    }

    /**
     * @brief Assignment.
     */
    Flag& operator=(bool value);

    /**
     * @brief Waits until the flag has the value.
     * @param value             Value to wait for.
     * @param timeout           Wait timeout.
     * @return true if the flag received the value, or false if timeout occurs.
     */
    bool wait_for(bool value, const std::chrono::milliseconds& timeout);

    /**
     * @brief Waits until the flag has the value.
     * @param value             Value to wait for.
     * @param timeoutAt           Wait timeout.
     * @return true if the flag received the value, or false if timeout occurs.
     */
    bool wait_until(bool value, const DateTime& timeoutAt);

private:
    /**
     * @brief Mutex object.
     */
    mutable std::mutex m_lockMutex;

    /**
     * @brief Mutex condition object.
     */
    std::condition_variable m_condition;

    /**
     * @brief Flag value.
     */
    bool m_value {false};

    /**
     * @brief Terminated flag.
     */
    bool m_terminated {false};

    /**
     * @brief Terminate flag usage.
     */
    void terminate();
};
/**
 * @}
 */
} // namespace sptk
