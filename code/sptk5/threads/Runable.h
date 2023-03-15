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

#include <sptk5/Strings.h>

#include <list>
#include <mutex>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * Abstract runable object.
 *
 * Should be used for deriving a user class for executing by a worker
 * thread in a thread pool. Derived class must override run() method.
 */
class SP_EXPORT Runable
{
public:
    /**
     * Default Constructor
     */
    explicit Runable(const String& name);

    /**
     * Destructor
     */
    virtual ~Runable() = default;

    /**
     * Executes task' run method
     *
     * Task may be executed multiple times, but only one caller
     * may execute same task at a time.
     */
    void execute();

    /**
     * Requests execution termination
     */
    virtual void terminate();

    /**
     * Returns true if terminate request is sent to runable
     */
    bool terminated() const;

    /**
     * @return object name
     */
    String name() const
    {
        return m_name;
    }

protected:
    /**
     * Method that is executed by worker thread
     *
     * Should be overwritten by derived class.
     */
    virtual void run() = 0;

private:
    using SRunable = std::shared_ptr<Runable>;

    mutable std::mutex m_dataMutex;           ///< Synchronized object that protects internal data
    bool m_terminated {false};                ///< Flag indicating if task is terminated
    const String m_name;                      ///< Runable object name
    std::list<SRunable>::iterator m_position; ///< Runable position in the queue

    /**
     * Set runable to terminated
     * @param terminated        Is terminated flag
     */
    void setTerminated(bool terminated);
};

using SRunable = std::shared_ptr<Runable>;

class RunableQueue
{
public:
    void push(const SRunable& runable);

private:
    mutable std::mutex m_mutex;
    std::list<SRunable> m_queue;
};

/**
 * @}
 */
} // namespace sptk
