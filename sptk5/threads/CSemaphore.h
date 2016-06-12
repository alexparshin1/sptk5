/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CSemaphore.h - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __CSEMAPHORE_H__
#define __CSEMAPHORE_H__

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <condition_variable>
#include <atomic>
#include <chrono>

namespace sptk {

/// @addtogroup threads Thread Classes
/// @{

/// @brief Generic unnamed semaphore class
class SP_EXPORT CSemaphore
{
    std::mutex              m_mutex;        ///< Mutex object
    std::condition_variable m_condition;    ///< Mutex condition object
    std::atomic<int>        m_value;        ///< Semaphore value

public:
    /// @brief Constructor
    ///
    /// Creates semaphore with starting value (default 0)
    /// @param startingValue uint32_t, starting semaphore value
    CSemaphore(uint32_t startingValue=0);

    /// @brief Destructor
    virtual ~CSemaphore();

    /// @brief Posts the semaphore
    ///
    /// The semaphore value is increased by one.
    void post() THROWS_EXCEPTIONS;

    /// @brief Waits until semaphore value is greater than zero, or until timeout occurs
    ///
    /// If semaphore value is greater than zero, decreases semaphore value by one and returns true.
    /// Timeout interval is in milliseconds.
    /// @param timeoutMS int32_t, wait timeout in milliseconds
    /// @return true if semaphore was posted (signaled), or false if timeout occurs
    bool wait(uint32_t timeoutMS) THROWS_EXCEPTIONS;
};
/// @}
}

#endif
