/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Runable.h - description                                ║
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

#ifndef __SPTK_RUNABLE_H__
#define __SPTK_RUNABLE_H__

#include <sptk5/Strings.h>

#include <atomic>
#include "Locks.h"

namespace sptk
{

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
    mutable SharedMutex m_mutex;        ///< Synchronized object locked while the task running
    bool                m_terminated;   ///< Flag indicating if task is terminated
    String              m_name;         ///< Runable object name

    /**
     * Set runable to terminated
     * @param terminated        Is terminated flag
     */
    void setTerminated(bool terminated);

protected:

    /**
     * Method that is executed by worker thread
     *
     * Should be overwritten by derived class.
     */
    virtual void run() = 0;

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
};
/**
 * @}
 */
}

#endif
