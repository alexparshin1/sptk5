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

// Notes on the wake-up strategy (performance):
//
// Readers wait on m_readerCondition; writers and the single upgrader wait on
// m_writerCondition. Keeping the two classes on separate condition variables means an
// unlocker never wakes the class that cannot make progress (e.g. releasing an exclusive
// lock to a queued writer no longer also stampedes every blocked reader).
//
// Unlockers also avoid touching m_mutex / the condition variable at all when there is
// provably no one to wake. Each potential waiter publishes its presence before sleeping —
// writers via m_writersWaiting, the upgrader via UPGRADING_BIT, readers via
// m_readersWaiting — and the unlocker samples those counters first. Because the counter
// load and the state store race across threads, a seq_cst fence is inserted between the
// state mutation and the counter load (and symmetrically, every blocking predicate runs a
// seq_cst fence before sampling the state). This pair of fences forms a StoreLoad barrier:
// if an unlocker observes "no waiters" it is guaranteed the would-be waiter observes the
// freed state and therefore never sleeps — so no wake-up can be lost.

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
            m_readersWaiting.fetch_add(1, memory_order_relaxed);
            m_readerCondition.wait(lock, [this, &state]
                             {
                                 // seq_cst fence pairs with the unlocker's fence so that a
                                 // missed wake-up is impossible (see header note above).
                                 atomic_thread_fence(memory_order_seq_cst);
                                 state = m_state.load(memory_order_relaxed);
                                 return !(state & (EXCLUSIVE_BIT | UPGRADING_BIT)) &&
                                        m_writersWaiting.load(memory_order_relaxed) == 0;
                             });
            m_readersWaiting.fetch_sub(1, memory_order_relaxed);
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
            m_readersWaiting.fetch_add(1, memory_order_relaxed);
            const bool signalled = m_readerCondition.wait_for(lock, timeout, [this, &state]
                                      {
                                          atomic_thread_fence(memory_order_seq_cst);
                                          state = m_state.load(memory_order_relaxed);
                                          return !(state & (EXCLUSIVE_BIT | UPGRADING_BIT)) &&
                                                 m_writersWaiting.load(memory_order_relaxed) == 0;
                                      });
            m_readersWaiting.fetch_sub(1, memory_order_relaxed);
            if (!signalled)
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
    if ((prev & READER_MASK) != 1)
    {
        return; // Not the last reader — nobody can be unblocked by this release.
    }
    if (prev & UPGRADING_BIT)
    {
        // An upgrader is draining readers and is parked on m_writerCondition. We learned
        // of it from our own RMW result (no cross-thread race), so just wake it. Plain
        // writers may be parked on the same CV but cannot proceed while UPGRADING_BIT is
        // set, hence notify_all to be certain the upgrader is among those woken.
        const lock_guard lock(m_mutex);
        m_writerCondition.notify_all();
        return;
    }
    // StoreLoad barrier: order our decrement-to-zero before sampling m_writersWaiting, so
    // that a writer which queued concurrently is never left asleep (see header note).
    atomic_thread_fence(memory_order_seq_cst);
    if (m_writersWaiting.load(memory_order_relaxed) != 0)
    {
        // Only one writer can take the exclusive lock; wake exactly one. If it fails to
        // acquire (loses a race or times out) the baton is passed on by unlockExclusive()
        // or tryLockExclusive()'s give-up path.
        const lock_guard lock(m_mutex);
        m_writerCondition.notify_one();
    }
}

void ReadWriteMutex::unlockExclusive()
{
    m_state.fetch_and(~EXCLUSIVE_BIT, memory_order_release);
    // StoreLoad barrier before sampling the waiter counters (see header note).
    atomic_thread_fence(memory_order_seq_cst);
    // Writer preference: hand the lock to a single queued writer if one exists, otherwise
    // release all blocked readers. The last writer to drain (m_writersWaiting == 0) is the
    // one that finally wakes the readers.
    if (m_writersWaiting.load(memory_order_relaxed) != 0)
    {
        const lock_guard lock(m_mutex);
        m_writerCondition.notify_one();
    }
    else if (m_readersWaiting.load(memory_order_relaxed) != 0)
    {
        const lock_guard lock(m_mutex);
        m_readerCondition.notify_all();
    }
}

void ReadWriteMutex::lockExclusive()
{
    // Register as a pending writer so new shared locks yield to us, preventing writer
    // starvation. The guard releases the count on scope exit (or if wait() throws); once
    // EXCLUSIVE_BIT is held, readers stay blocked until unlockExclusive() regardless.
    const PendingWriterGuard pendingWriter(m_writersWaiting);
    unique_lock              lock(m_mutex);
    m_writerCondition.wait(lock, [this]
                     {
                         // seq_cst fence (after the pending-writer increment) pairs with
                         // the unlocker's fence so our queued status cannot be missed.
                         atomic_thread_fence(memory_order_seq_cst);
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
    const bool         acquired = m_writerCondition.wait_for(lock, timeout, [this]
                                {
                                    atomic_thread_fence(memory_order_seq_cst);
                                    uint32_t expected = 0;
                                    return m_state.compare_exchange_strong(expected, EXCLUSIVE_BIT,
                                                                          memory_order_acquire,
                                                                          memory_order_relaxed);
                                });
    if (!acquired)
    {
        // We gave up. Drop our pending status, then pass the baton: any wake-up that
        // targeted us must be forwarded so no other waiter is stranded. We still hold
        // m_mutex, so these notifications cannot be lost against threads that block under it.
        m_writersWaiting.fetch_sub(1, memory_order_relaxed);
        pendingWriter.release();
        if (m_writersWaiting.load(memory_order_relaxed) != 0)
        {
            // Other writers remain — hand off to one of them (preserves writer preference).
            m_writerCondition.notify_one();
        }
        else if (m_readersWaiting.load(memory_order_relaxed) != 0)
        {
            // No writers left — readers that yielded to us may now proceed.
            m_readerCondition.notify_all();
        }
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

    // Wait for remaining readers to drain. We park on m_writerCondition; the last reader's
    // unlockShared() detects UPGRADING_BIT and wakes us. Because that wake-up always takes
    // m_mutex (no lazy skip on the UPGRADING_BIT path), it cannot be lost.
    if ((m_state.load(memory_order_acquire) & READER_MASK) != 0)
    {
        unique_lock lock(m_mutex);
        m_writerCondition.wait(lock, [this]
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

    // Slow path: wait for readers to drain (parked on m_writerCondition, woken by the last
    // reader's unlockShared() via the UPGRADING_BIT path).
    {
        unique_lock lock(m_mutex);
        if (!m_writerCondition.wait_for(lock, timeout, [this]
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
            // Clearing UPGRADING_BIT may unblock readers that yielded to us. We are back to
            // holding a shared lock, so queued writers stay blocked (woken later when we
            // release). Notify under m_mutex, so registered readers cannot be missed.
            if (m_readersWaiting.load(memory_order_relaxed) != 0)
            {
                m_readerCondition.notify_all();
            }
            return false;
        }
    }

    m_state.store(EXCLUSIVE_BIT, memory_order_release);
    return true;
}

void ReadWriteMutex::downgradeToShared()
{
    // We hold EXCLUSIVE_BIT. Atomically convert the exclusive hold into a single reader
    // hold. Release ordering publishes the writes made under the exclusive lock to the
    // readers that proceed once they observe the reader count. The lock is never fully
    // released, so the operation always succeeds without blocking.
    m_state.store(1, memory_order_release);
    // StoreLoad barrier before sampling m_readersWaiting (see header note). Only readers
    // can be unblocked: a reader is now held, so queued writers stay parked.
    atomic_thread_fence(memory_order_seq_cst);
    if (m_readersWaiting.load(memory_order_relaxed) != 0)
    {
        const lock_guard lock(m_mutex);
        m_readerCondition.notify_all();
    }
}
