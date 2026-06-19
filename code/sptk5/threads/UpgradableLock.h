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

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <set>
#include <thread>

namespace sptk {

/**
 * @brief Upgradable lock.
 *
 * The lock can be locked in shared mode, which allows multiple threads to access
 * the protected resource simultaneously.
 *
 * It also can be locked in exclusive mode, which allows only one thread to access
 * the protected resource at a time.
 *
 * And in shared mode, it can be atomically upgraded to exclusive mode.
 */
class UpgradableLock
{
public:
    /**
     * @brief Locks the lock in shared mode.
     */
    void lockShared();

    /**
     * @brief Tries to lock in shared mode with a timeout.
     * @param timeout Maximum time to wait for the lock.
     * @return True if the lock was acquired, false on timeout.
     */
    bool tryLockShared(std::chrono::milliseconds timeout);

    /**
     * @brief Unlocks the lock in shared mode.
     */
    void unlock();

    /**
     * @brief Locks the lock in exclusive mode.
     * @remarks If the lock is already locked in shared mode, it will be upgraded to exclusive mode.
     */
    void lockExclusive();

    /**
     * @brief Tries to lock in exclusive mode with a timeout.
     * @param timeout Maximum time to wait for the lock.
     * @return True if the lock was acquired, false on timeout.
     * @remarks If the lock is already locked in shared mode by this thread, it will be upgraded to exclusive mode.
     */
    bool tryLockExclusive(std::chrono::milliseconds timeout);

private:
    std::mutex                m_mutex;             ///< Protects internal state
    std::condition_variable   m_condition;         ///< Used for blocking waiters
    unsigned                  m_sharedCount {0};   ///< Number of active shared lock holders
    bool                      m_exclusive {false}; ///< True when exclusive lock is held
    bool                      m_upgrading {false}; ///< True when a shared holder is upgrading to exclusive
    std::set<std::thread::id> m_sharedOwners;      ///< Tracks which threads hold shared locks (for upgrade detection)
};

} // namespace sptk
