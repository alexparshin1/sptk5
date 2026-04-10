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

#include <sptk5/sptk.h>

#include <chrono>
#include <mutex>

namespace sptk {
/**
 * @brief Simple stopwatch class useful in measuring time intervals.
 * This class is thread-safe.
 */
class SP_EXPORT Stopwatch
{
public:
    /**
     * @brief Constructor.
     */
    Stopwatch() = default;

    /**
     * @brief Constructor that measures action execution time.
     * @tparam Action           Measured action function type.
     * @param action            Measured action.
     */
    template<class Action>
    explicit Stopwatch(const Action& action)
    {
        start();
        action();
        stop();
    }

    /**
     * @brief Destructor.
     */
    ~Stopwatch() = default;

    /**
     * @brief Set stopwatch start time.
     */
    void start();

    /**
     * @brief Set stopwatch stop time.
     */
    void stop();

    /**
     * @brief Get the difference between stopwatch start and stop times in seconds.
     * @return interval in seconds.
     */
    double seconds() const;

    /**
     * @brief Get the difference between stopwatch start and stop times in seconds.
     * @return interval in seconds.
     */
    double milliseconds() const;

private:
    mutable std::mutex                    m_mutex;                                      ///< Mutex that provides thread-safety.
    std::chrono::steady_clock::time_point m_started {std::chrono::steady_clock::now()}; ///< Start time.
    std::chrono::steady_clock::time_point m_ended;                                      ///< Stop time.
};

} // namespace sptk
