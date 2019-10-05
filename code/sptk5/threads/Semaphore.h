/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Semaphore.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_SEMAPHORE_H__
#define __SPTK_SEMAPHORE_H__

#include <sptk5/sptk.h>
#include <sptk5/DateTime.h>
#include <sptk5/Exception.h>
#include <condition_variable>
#include <atomic>
#include <chrono>

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
    /**
     * Mutex object
     */
    mutable std::mutex      m_lockMutex;

    /**
     * Mutex condition object
     */
    std::condition_variable m_condition;

    /**
     * Semaphore value
     */
    size_t                  m_value {0};

    /**
     * Semaphore max value
     */
    size_t                  m_maxValue {0};

    /**
     * Number of waiters
     */
    size_t                  m_waiters {0};

    /**
     * Terminated flag
     */
    bool                    m_terminated {false};

    void terminate();

    /**
     * Current number of waiters
     */
    size_t waiters() const;

public:

    /**
     * @brief Constructor
     *
     * Creates semaphore with starting value (default 0)
     * @param startingValue     Starting semaphore value
     * @param maxValue          Maximum semaphore value, or 0 if unlimited
     */
    explicit Semaphore(size_t startingValue=0, size_t maxValue=0);

    /**
     * @brief Destructor
     */
    virtual ~Semaphore();

    /**
     * @brief Set the semaphore value
     */
    void set(size_t value);

    /**
     * @brief Post the semaphore
     *
     * The semaphore value is increased by one.
     */
    void post();

    /**
     * @brief Wait until semaphore value is greater than zero, or until timeout interval is passed
     *
     * If semaphore value is greater than zero, decreases semaphore value by one and returns true.
     * @param timeout           Wait timeout
     * @return true if semaphore was posted (signaled), or false if timeout occurs
     */
    bool sleep_for(std::chrono::milliseconds timeout);

    /**
     * @brief Wait until semaphore value is greater than zero, or until timeoutAt occurs
     *
     * If semaphore value is greater than zero, decreases semaphore value by one and returns true.
     * @param timeout           Timeout moment
     * @return true if semaphore was posted (signaled), or false if timeout occurs
     */
    bool sleep_until(DateTime timeout);
};
/**
 * @}
 */
}

#endif
