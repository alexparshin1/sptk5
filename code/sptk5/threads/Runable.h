/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Runable.h - description                                ║
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
 * @brief Abstract runable object.
 *
 * Should be used for deriving a user class for executing by a worker
 * thread in a thread pool. Derived class must override run() method.
 */
class SP_EXPORT Runable
{
    /**
     * Flag: is the task sent terminate request?
     */
    std::atomic_bool    m_terminated;

    /**
     * Synchronized object locked while the task running
     */
    std::mutex          m_running;


protected:

    /**
     * @brief Method that is executed by worker thread
     *
     * Should be overwritten by derived class.
     */
    virtual void run() = 0;

public:

    /**
     * @brief Default Constructor
     */
    Runable();

    /**
     * @brief Destructor
     */
    virtual ~Runable() = default;

    /**
     * @brief Executes task' run method
     *
     * Task may be executed multiple times, but only one caller
     * may execute same task at a time.
     */
    void execute();

    /**
     * @brief Requests execution termination
     */
    void terminate();

    /**
     * @brief Returns true if terminate request is sent to runable
     */
    bool terminated();
};
/**
 * @}
 */
}

#endif
