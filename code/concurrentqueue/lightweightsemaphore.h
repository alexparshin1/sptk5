// Provides an efficient implementation of a semaphore (LightweightSemaphore).
// This is an extension of Jeff Preshing's sempahore implementation (licensed
// under the terms of its separate zlib license) that has been adapted and
// extended by Cameron Desrochers.

#pragma once

#include <atomic>
#include <cassert> // assert
#include <cerrno>  // For EINTR
#include <cstddef> // For std::size_t
#include <cstdint> // For std::uint64_t
#include <ctime>   // For clock_gettime
#include <semaphore>
#include <type_traits> // For std::make_signed<T>

#ifndef MOODYCAMEL_DELETE_FUNCTION
#if __cplusplus >= 201103L || _MSC_VER >= 1900
#define MOODYCAMEL_DELETE_FUNCTION = delete
#else
#define MOODYCAMEL_DELETE_FUNCTION
#endif
#endif

namespace moodycamel {

// Code in the mpmc_sema namespace below is an adaptation of Jeff Preshing's
// portable + lightweight semaphore implementations, originally from
// https://github.com/preshing/cpp11-on-multicore/blob/master/common/sema.h
// LICENSE:
// Copyright (c) 2015 Jeff Preshing
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//	claim that you wrote the original software. If you use this software
//	in a product, an acknowledgement in the product documentation would be
//	appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//	misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

class Semaphore
{
public:
    explicit Semaphore(const int initialCount = 0)
        : m_hSema(initialCount)
    {
    }

    ~Semaphore()
    {
    }

    bool acquire()
    {
        m_hSema.acquire();
        return true;
    }

    bool try_acquire()
    {
        return m_hSema.try_acquire();
    }

    bool try_acquire_for(std::chrono::microseconds usecs)
    {
        return m_hSema.try_acquire_for(usecs);
    }

    void release(int count = 1)
    {
        m_hSema.release(count);
    }

private:
    std::counting_semaphore<> m_hSema;

    Semaphore(const Semaphore& other) = delete;
    Semaphore& operator=(const Semaphore& other) = delete;
};

//---------------------------------------------------------
// LightweightSemaphore
//---------------------------------------------------------
class LightweightSemaphore
{
public:
    typedef std::make_signed<std::size_t>::type ssize_t;

private:
    std::atomic<ssize_t>  m_count;
    moodycamel::Semaphore m_sema;
    int                   m_maxSpins;

    bool waitWithPartialSpinning(std::int64_t timeout_usecs = -1)
    {
        ssize_t oldCount;
        int     spin = m_maxSpins;
        while (--spin >= 0)
        {
            oldCount = m_count.load(std::memory_order_relaxed);
            if ((oldCount > 0) && m_count.compare_exchange_strong(oldCount, oldCount - 1, std::memory_order_acquire, std::memory_order_relaxed))
                return true;
            std::atomic_signal_fence(std::memory_order_acquire); // Prevent the compiler from collapsing the loop.
        }
        oldCount = m_count.fetch_sub(1, std::memory_order_acquire);
        if (oldCount > 0)
            return true;
        if (timeout_usecs < 0)
        {
            if (m_sema.acquire())
                return true;
        }
        if (timeout_usecs > 0 && m_sema.try_acquire_for(std::chrono::microseconds(timeout_usecs)))
            return true;
        // At this point, we've timed out waiting for the semaphore, but the
        // count is still decremented indicating we may still be waiting on
        // it. So we have to re-adjust the count, but only if the semaphore
        // wasn't signaled enough times for us too since then. If it was, we
        // need to release the semaphore too.
        while (true)
        {
            oldCount = m_count.load(std::memory_order_acquire);
            if (oldCount >= 0 && m_sema.try_acquire())
                return true;
            if (oldCount < 0 && m_count.compare_exchange_strong(oldCount, oldCount + 1, std::memory_order_relaxed, std::memory_order_relaxed))
                return false;
        }
    }

    ssize_t waitManyWithPartialSpinning(ssize_t max, std::int64_t timeout_usecs = -1)
    {
        assert(max > 0);
        ssize_t oldCount;
        int     spin = m_maxSpins;
        while (--spin >= 0)
        {
            oldCount = m_count.load(std::memory_order_relaxed);
            if (oldCount > 0)
            {
                ssize_t newCount = oldCount > max ? oldCount - max : 0;
                if (m_count.compare_exchange_strong(oldCount, newCount, std::memory_order_acquire, std::memory_order_relaxed))
                    return oldCount - newCount;
            }
            std::atomic_signal_fence(std::memory_order_acquire);
        }
        oldCount = m_count.fetch_sub(1, std::memory_order_acquire);
        if (oldCount <= 0)
        {
            if (timeout_usecs == 0 || (timeout_usecs < 0 && !m_sema.acquire()) || (timeout_usecs > 0 && !m_sema.try_acquire_for(std::chrono::microseconds(timeout_usecs))))
            {
                while (true)
                {
                    oldCount = m_count.load(std::memory_order_acquire);
                    if (oldCount >= 0 && m_sema.try_acquire())
                        break;
                    if (oldCount < 0 && m_count.compare_exchange_strong(oldCount, oldCount + 1, std::memory_order_relaxed, std::memory_order_relaxed))
                        return 0;
                }
            }
        }
        if (max > 1)
            return 1 + tryWaitMany(max - 1);
        return 1;
    }

public:
    LightweightSemaphore(ssize_t initialCount = 0, int maxSpins = 10000)
        : m_count(initialCount)
        , m_maxSpins(maxSpins)
    {
        assert(initialCount >= 0);
        assert(maxSpins >= 0);
    }

    bool tryWait()
    {
        ssize_t oldCount = m_count.load(std::memory_order_relaxed);
        while (oldCount > 0)
        {
            if (m_count.compare_exchange_weak(oldCount, oldCount - 1, std::memory_order_acquire, std::memory_order_relaxed))
                return true;
        }
        return false;
    }

    bool wait()
    {
        return tryWait() || waitWithPartialSpinning();
    }

    bool wait(std::int64_t timeout_usecs)
    {
        return tryWait() || waitWithPartialSpinning(timeout_usecs);
    }

    // Acquires between 0 and (greedily) max, inclusive
    ssize_t tryWaitMany(ssize_t max)
    {
        assert(max >= 0);
        ssize_t oldCount = m_count.load(std::memory_order_relaxed);
        while (oldCount > 0)
        {
            ssize_t newCount = oldCount > max ? oldCount - max : 0;
            if (m_count.compare_exchange_weak(oldCount, newCount, std::memory_order_acquire, std::memory_order_relaxed))
                return oldCount - newCount;
        }
        return 0;
    }

    // Acquires at least one, and (greedily) at most max
    ssize_t waitMany(ssize_t max, std::int64_t timeout_usecs)
    {
        assert(max >= 0);
        ssize_t result = tryWaitMany(max);
        if (result == 0 && max > 0)
            result = waitManyWithPartialSpinning(max, timeout_usecs);
        return result;
    }

    ssize_t waitMany(ssize_t max)
    {
        ssize_t result = waitMany(max, -1);
        assert(result > 0);
        return result;
    }

    void signal(ssize_t count = 1)
    {
        assert(count >= 0);
        ssize_t oldCount = m_count.fetch_add(count, std::memory_order_release);
        ssize_t toRelease = -oldCount < count ? -oldCount : count;
        if (toRelease > 0)
        {
            m_sema.release(static_cast<int>(toRelease));
        }
    }

    std::size_t availableApprox() const
    {
        ssize_t count = m_count.load(std::memory_order_relaxed);
        return count > 0 ? static_cast<std::size_t>(count) : 0;
    }
};

} // end namespace moodycamel
