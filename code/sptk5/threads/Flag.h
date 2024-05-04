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

#include <atomic>
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
 * Generic unnamed semaphore class
 */
class SP_EXPORT Flag
{
public:
    /**
     * Constructor
     *
     * Creates flag with starting value (default false)
     * @param startingValue     Starting semaphore value
     * @param maxValue          Maximum semaphore value, or 0 if unlimited
     */
    explicit Flag(bool startingValue = false);

    /**
     * Destructor
     */
    virtual ~Flag();

    /**
     * Get the flag value
     * @param value             New flag value
     */
    bool get() const;

    /**
     * Set the flag value
     * @param value             New flag value
     */
    void set(bool value);

    /**
     * Adaptor
     */
    operator bool() const
    {
        return get();
    }

    /**
     * Assignment
     */
    Flag& operator=(bool value)
    {
        set(value);
        return *this;
    }

    /**
     * Waits until the flag has the value
     * @param value             Value to wait for
     * @param timeout           Wait timeout
     * @return true if flag received the value, or false if timeout occurs
     */
    bool wait_for(bool value, std::chrono::milliseconds timeout);

    /**
     * Waits until the flag has the value
     * @param value             Value to wait for
     * @param timeoutAt           Wait timeout
     * @return true if flag received the value, or false if timeout occurs
     */
    bool wait_until(bool value, const DateTime& timeoutAt);

private:
    /**
     * Mutex object
     */
    mutable std::mutex m_lockMutex;

    /**
     * Mutex condition object
     */
    std::condition_variable m_condition;

    /**
     * Flag value
     */
    bool m_value {false};

    /**
     * Number of waiters
     */
    size_t m_waiters {0};

    /**
     * Terminated flag
     */
    bool m_terminated {false};

    /**
     * Terminate flag usage
     */
    void terminate();

    /**
     * Current number of waiters
     */
    size_t waiters() const;
};
/**
 * @}
 */
} // namespace sptk
