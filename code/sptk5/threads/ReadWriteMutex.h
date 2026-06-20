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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace sptk {

/**
 * @brief Upgradable read/write mutex.
 *
 * The mutex can be locked in shared mode, which allows multiple threads to access
 * the protected resource simultaneously.
 *
 * It also can be locked in exclusive mode, which allows only one thread to access
 * the protected resource at a time.
 *
 * And in shared mode, it can be atomically upgraded to exclusive mode.
 *
 * @remarks The mutex is writer-preferring: once a thread is waiting for an exclusive
 * lock, new shared-lock requests block until the writer is served. This prevents writer
 * starvation under a steady stream of readers.
 *
 * @warning Because of writer preference, the mutex is NOT recursive in shared mode: a
 * thread that already holds a shared lock must not request another shared lock, as a
 * writer queued in between would deadlock (the writer waits for the first shared lock to
 * be released, while the thread waits for the writer to clear). Use tryLockShared() with
 * a timeout if a nested shared acquisition is unavoidable.
 */
class ReadWriteMutex
{
public:
    /**
     * @brief Locks the mutex in shared mode.
     * @remarks Blocks while an exclusive lock is held or a writer is queued.
     * @warning Not recursive: see the class-level warning about nested shared locks.
     */
    void lockShared();

    /**
     * @brief Tries to lock in shared mode with a timeout.
     * @param timeout Maximum time to wait for the lock.
     * @return True if the lock was acquired, false on timeout.
     */
    bool tryLockShared(std::chrono::milliseconds timeout);

    /**
     * @brief Unlocks shared lock.
     */
    void unlockShared();

    /**
     * @brief Unlocks exclusive lock.
     */
    void unlockExclusive();

    /**
     * @brief Locks the mutex in exclusive mode.
     */
    void lockExclusive();

    /**
     * @brief Tries to lock in exclusive mode with a timeout.
     * @param timeout Maximum time to wait for the lock.
     * @return True if the lock was acquired, false on timeout.
     */
    bool tryLockExclusive(std::chrono::milliseconds timeout);

    /**
     * @brief Upgrades from shared to exclusive lock.
     * @remarks Caller must already hold a shared lock.
     */
    void upgradeToExclusive();

    /**
     * @brief Tries to upgrade from shared to exclusive lock with a timeout.
     * @param timeout Maximum time to wait.
     * @return True if upgraded, false on timeout (shared lock is retained).
     */
    bool tryUpgradeToExclusive(std::chrono::milliseconds timeout);

    /**
     * @brief Downgrades from exclusive to shared lock.
     * @remarks Caller must already hold an exclusive lock. The operation never blocks:
     * the exclusive hold is atomically converted into a single shared hold, so the lock
     * is retained throughout and queued readers are woken.
     */
    void downgradeToShared();

private:
    static constexpr uint32_t EXCLUSIVE_BIT = 1u << 31;
    static constexpr uint32_t UPGRADING_BIT = 1u << 30;
    static constexpr uint32_t READER_MASK   = ~(EXCLUSIVE_BIT | UPGRADING_BIT);

    std::atomic<uint32_t>   m_state {0};           ///< Packed state: bits 31=exclusive, 30=upgrading, 0-29=reader count
    std::atomic<uint32_t>   m_writersWaiting {0};  ///< Count of writers queued for exclusive access; new shared locks yield to them
    std::atomic<uint32_t>   m_readersWaiting {0};  ///< Count of readers blocked on m_readerCondition; lets unlockers skip notifying when zero
    std::mutex              m_mutex;               ///< Protects condition variable waits
    std::condition_variable m_readerCondition;     ///< Blocks readers waiting for writers/upgraders to clear
    std::condition_variable m_writerCondition;     ///< Blocks writers and the upgrader waiting for exclusive access
};

} // namespace sptk
