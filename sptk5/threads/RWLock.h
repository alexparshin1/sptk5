/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       RWLock.h - description                                 ║
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

#ifndef __CRWLOCK_H__
#define __CRWLOCK_H__

#include <sptk5/sptk.h>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace sptk {

/**
 * @addtogroup threads Thread Classes
 * @{
 */

/**
 * Read-write synchronization object
 */
class SP_EXPORT RWLock
{
protected:

    /**
     * Lock mutex
     */
    std::mutex              m_writeLock;

    /**
     * Lock condition variable
     */
    std::condition_variable m_condition;

    /**
     * Reader lock count
     */
    std::atomic<int>        m_readerCount;

    /**
     * Writer mode flag
     */
    std::atomic<bool>       m_writerMode;


public:

    /**
     * Constructor
     */
    RWLock();

    /**
     * Destructor
     */
    ~RWLock() = default;

    /**
     * Try to lock the object for reading. Blocks if object is locked for writing, or there are pending write locks.
     * @param timeout std::chrono::milliseconds, Lock timeout
     */
    int lockR(std::chrono::milliseconds timeout);

    /**
     * Try to lock the object for writing. Blocks if object is locked for reading or writing.
     * @param timeout std::chrono::milliseconds, Lock timeout
     */
    int lockRW(std::chrono::milliseconds timeout);

    /**
     * Releases lock on the object.
     */
    void unlock();
};
/**
 * @}
 */
}

#endif
