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

#include "../../../sptk5/threads/ReadWriteMutex.h"

#include <atomic>
#include <cstdint>

using namespace std;
using namespace sptk;

namespace {

/**
 * @brief RAII guard for the pending-writer counter.
 *
 * Increments the counter on construction and decrements it on destruction,
 * guaranteeing the pending-writer count is released even if condition_variable::wait
 * throws — otherwise a leaked count would block every future reader forever.
 *
 * The decrement can be suppressed with release() when the caller needs to perform
 * the decrement itself under a held mutex (so a paired notify cannot be lost).
 */
class PendingWriterGuard
{
public:
    explicit PendingWriterGuard(atomic<uint32_t>& counter)
        : m_counter(counter)
    {
        m_counter.fetch_add(1, memory_order_relaxed);
    }

    ~PendingWriterGuard()
    {
        if (m_armed)
        {
            m_counter.fetch_sub(1, memory_order_relaxed);
        }
    }

    PendingWriterGuard(const PendingWriterGuard&) = delete;
    PendingWriterGuard& operator=(const PendingWriterGuard&) = delete;

    /**
     * @brief Suppresses the automatic decrement; the caller takes ownership of it.
     */
    void release()
    {
        m_armed = false;
    }

private:
    atomic<uint32_t>& m_counter;
    bool              m_armed {true};
};

} // namespace

void ReadWriteMutex::lockShared()
{
    uint32_t state = m_state.load(memory_order_relaxed);
    while (true)
    {
        // Yield to queued writers (m_writersWaiting) as well as active exclusive/upgrade
        // holders. m_writersWaiting is only a fairness gate — mutual exclusion is enforced
        // solely by the m_state acquire/release CAS below — so relaxed loads are sufficient.
        if ((state & (EXCLUSIVE_BIT | UPGRADING_BIT)) ||
            m_writersWaiting.load(memory_order_relaxed) != 0)
        {
            unique_lock lock(m_mutex);
            m_condition.wait(lock, [this, &state]
                             {
                                 state = m_state.load(memory_order_relaxed);
                                 return !(state & (EXCLUSIVE_BIT | UPGRADING_BIT)) &&
                                        m_writersWaiting.load(memory_order_relaxed) == 0;
                             });
        }
        if (m_state.compare_exchange_weak(state, state + 1,
                                          memory_order_acquire,
                                          memory_order_relaxed))
        {
            return;
        }
    }
}

bool ReadWriteMutex::tryLockShared(chrono::milliseconds timeout)
{
    uint32_t state = m_state.load(memory_order_relaxed);
    while (true)
    {
        if ((state & (EXCLUSIVE_BIT | UPGRADING_BIT)) ||
            m_writersWaiting.load(memory_order_relaxed) != 0)
        {
            unique_lock lock(m_mutex);
            if (!m_condition.wait_for(lock, timeout, [this, &state]
                                      {
                                          state = m_state.load(memory_order_relaxed);
                                          return !(state & (EXCLUSIVE_BIT | UPGRADING_BIT)) &&
                                                 m_writersWaiting.load(memory_order_relaxed) == 0;
                                      }))
            {
                return false;
            }
        }
        if (m_state.compare_exchange_weak(state, state + 1,
                                          memory_order_acquire,
                                          memory_order_relaxed))
        {
            return true;
        }
    }
}

void ReadWriteMutex::unlockShared()
{
    const uint32_t prev = m_state.fetch_sub(1, memory_order_release);
    if ((prev & READER_MASK) == 1)
    {
        lock_guard lock(m_mutex);
        m_condition.notify_all();
    }
}

void ReadWriteMutex::unlockExclusive()
{
    m_state.fetch_and(~EXCLUSIVE_BIT, memory_order_release);
    lock_guard lock(m_mutex);
    m_condition.notify_all();
}

void ReadWriteMutex::lockExclusive()
{
    // Register as a pending writer so new shared locks yield to us, preventing writer
    // starvation. The guard releases the count on scope exit (or if wait() throws); once
    // EXCLUSIVE_BIT is held, readers stay blocked until unlockExclusive() regardless.
    const PendingWriterGuard pendingWriter(m_writersWaiting);
    unique_lock              lock(m_mutex);
    m_condition.wait(lock, [this]
                     {
                         uint32_t expected = 0;
                         return m_state.compare_exchange_strong(expected, EXCLUSIVE_BIT,
                                                               memory_order_acquire,
                                                               memory_order_relaxed);
                     });
}

bool ReadWriteMutex::tryLockExclusive(const chrono::milliseconds timeout)
{
    // Register as a pending writer so new shared locks yield to us, preventing writer starvation
    PendingWriterGuard pendingWriter(m_writersWaiting);
    unique_lock        lock(m_mutex);
    const bool         acquired = m_condition.wait_for(lock, timeout, [this]
                                {
                                    uint32_t expected = 0;
                                    return m_state.compare_exchange_strong(expected, EXCLUSIVE_BIT,
                                                                          memory_order_acquire,
                                                                          memory_order_relaxed);
                                });
    if (!acquired)
    {
        // We gave up: clear our pending status and wake readers that blocked solely on it.
        // The decrement and notify both happen under m_mutex so the wakeup cannot be lost.
        m_writersWaiting.fetch_sub(1, memory_order_relaxed);
        pendingWriter.release();
        m_condition.notify_all();
    }
    // On success the guard releases the count on scope exit; EXCLUSIVE_BIT keeps readers
    // blocked until unlockExclusive() in the meantime.
    return acquired;
}

void ReadWriteMutex::upgradeToExclusive()
{
    uint32_t state = m_state.load(memory_order_relaxed);
    while (true)
    {
        if (state & (EXCLUSIVE_BIT | UPGRADING_BIT))
        {
            // Another exclusive/upgrader is active — release our shared lock
            // to avoid deadlock, then acquire exclusive from scratch
            unlockShared();
            lockExclusive();
            return;
        }
        // Try to claim UPGRADING_BIT and release our reader hold
        const uint32_t newState = (state | UPGRADING_BIT) - 1;
        if (m_state.compare_exchange_weak(state, newState,
                                          memory_order_acquire,
                                          memory_order_relaxed))
        {
            break;
        }
    }

    // Wait for remaining readers to drain
    if ((m_state.load(memory_order_acquire) & READER_MASK) != 0)
    {
        unique_lock lock(m_mutex);
        m_condition.wait(lock, [this]
                         {
                             return (m_state.load(memory_order_acquire) & READER_MASK) == 0;
                         });
    }

    // All readers gone — transition to exclusive
    m_state.store(EXCLUSIVE_BIT, memory_order_release);
}

bool ReadWriteMutex::tryUpgradeToExclusive(const chrono::milliseconds timeout)
{
    uint32_t state = m_state.load(memory_order_relaxed);
    while (true)
    {
        if (state & (EXCLUSIVE_BIT | UPGRADING_BIT))
        {
            // Another exclusive/upgrader active — release shared, try exclusive with timeout
            unlockShared();
            if (tryLockExclusive(timeout))
            {
                return true;
            }
            // Timeout — re-acquire shared before returning false
            lockShared();
            return false;
        }
        const uint32_t newState = (state | UPGRADING_BIT) - 1;
        if (m_state.compare_exchange_weak(state, newState,
                                          memory_order_acquire,
                                          memory_order_relaxed))
        {
            break;
        }
    }

    // Fast path: no other readers
    if ((m_state.load(memory_order_acquire) & READER_MASK) == 0)
    {
        m_state.store(EXCLUSIVE_BIT, memory_order_release);
        return true;
    }

    // Slow path: wait for readers to drain
    {
        unique_lock lock(m_mutex);
        if (!m_condition.wait_for(lock, timeout, [this]
                                  {
                                      return (m_state.load(memory_order_acquire) & READER_MASK) == 0;
                                  }))
        {
            // Timeout: restore shared state (clear UPGRADING_BIT, add back reader count)
            state = m_state.load(memory_order_relaxed);
            while (true)
            {
                const uint32_t restored = (state & ~UPGRADING_BIT) + 1;
                if (m_state.compare_exchange_weak(state, restored,
                                                  memory_order_release,
                                                  memory_order_relaxed))
                {
                    break;
                }
            }
            m_condition.notify_all();
            return false;
        }
    }

    m_state.store(EXCLUSIVE_BIT, memory_order_release);
    return true;
}
