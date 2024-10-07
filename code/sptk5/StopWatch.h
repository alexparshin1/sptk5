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

#include <sptk5/DateTime.h>

#include <functional>
#include <mutex>

namespace sptk {

/**
 * @brief Simple stopwatch class useful in measuring time intervals
 * This class is thread-safe.
 */
class SP_EXPORT StopWatch
{
public:
    /**
     * @brief Constructor
     */
    StopWatch() = default;

    /**
     * @brief Constructor that measures action execution time
     * @tparam Action           Measured action function type
     * @param action            Measured action
     */
    template<class Action>
    explicit StopWatch(const Action& action)
    {
        start();
        action();
        stop();
    }

    /**
     * @brief Destructor
     */
    ~StopWatch() = default;

    /**
     * @brief Set stopwatch start time
     */
    void start();

    /**
     * Set stopwatch stop time
     */
    void stop();

    /**
     * @brief Get difference between stopwatch start and stop times in seconds
     * @return interval in seconds
     */
    double seconds() const;

    /**
     * @brief Get difference between stopwatch start and stop times in seconds
     * @return interval in seconds
     */
    double milliseconds() const;

private:
    mutable std::mutex m_mutex;           ///< Mutex that provides thread-safety
    DateTime           m_started {"now"}; ///< Start time
    DateTime           m_ended;           ///< Stop time
};

} // namespace sptk
