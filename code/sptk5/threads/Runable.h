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

#include "atomic"
#include <sptk5/String.h>

namespace sptk {

/**
 * @addtogroup threads Thread Classes.
 * @{
 */

/**
 * @brief Abstract runable object.
 *
 * Should be used for deriving a user class for executing by a worker
 * thread in a thread pool. Derived class must override the run() method.
 */
class SP_EXPORT Runable
{
public:
    /**
     * @brief Default Constructor.
     */
    explicit Runable(String name);

    /**
     * @brief Destructor.
     */
    virtual ~Runable() = default;

    /**
     * @brief Executes the task's run method.
     *
     * Task may be executed multiple times, but only one caller
     * may execute the same task at a time.
     */
    void execute();

    /**
     * @brief Requests execution termination.
     */
    virtual void terminate();

    /**
     * @brief Returns true if the terminate() request is sent to runable.
     */
    [[nodiscard]] bool terminated() const;

    /**
     * @return object name.
     */
    [[nodiscard]] String name() const
    {
        return m_name;
    }

protected:
    /**
     * @brief Method that is executed by worker thread.
     *
     * Should be overwritten by derived class.
     */
    virtual void run() = 0;

private:
    std::atomic<bool> m_terminated {false}; ///< Flag indicating if the task is terminated.
    String            m_name;               ///< Runable object name.

    /**
     * @brief Set runable to terminated state.
     * @param terminated        Is terminated flag.
     */
    void setTerminated(bool terminated);
};

using SRunable = std::shared_ptr<Runable>;
using URunable = std::unique_ptr<Runable>;

/**
 * @}
 */
} // namespace sptk
