/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Semaphore.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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
    std::mutex              m_mutex;

    /**
     * Mutex condition object
     */
    std::condition_variable m_condition;

    /**
     * Semaphore value
     */
    uint32_t                m_value;

    /**
     * Semaphore max value
     */
    uint32_t                m_maxValue;

public:
    /**
     * @brief Constructor
     *
     * Creates semaphore with starting value (default 0)
     * @param startingValue     Starting semaphore value
     * @param maxValue          Maximum semaphore value, or 0 if unlimited
     */
    Semaphore(uint32_t startingValue=0, uint32_t maxValue=0);

    /**
     * @brief Destructor
     */
    virtual ~Semaphore() = default;

    /**
     * @brief Posts the semaphore
     *
     * The semaphore value is increased by one.
     */
    void post();

    /**
     * @brief Waits until semaphore value is greater than zero, or until timeout interval is passed
     *
     * If semaphore value is greater than zero, decreases semaphore value by one and returns true.
     * @param timeout           Wait timeout
     * @return true if semaphore was posted (signaled), or false if timeout occurs
     */
    bool sleep_for(std::chrono::milliseconds timeout);

    /**
     * @brief Waits until semaphore value is greater than zero, or until timeoutAt occurs
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
