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

#include "sptk5/Printer.h"
#include "sptk5/Stopwatch.h"
#include "sptk5/threads/ReadWriteLock.h"


#include <atomic>
#include <future>
#include <gtest/gtest.h>
#include <shared_mutex>
#include <sptk5/threads/ReadWriteMutex.h>
#include <thread>

using namespace std;
using namespace chrono;
using namespace sptk;

TEST(ReadWriteLockTests, sharedLockAllowsConcurrentReaders)
{
    ReadWriteMutex rwMutex;
    atomic         insideCount {0};
    atomic         maxCount {0};

    auto reader = [&rwMutex, &insideCount, &maxCount]
    {
        ReadWriteLock readLock(rwMutex, ReadWriteLock::Mode::Reader);
        const int     current = ++insideCount;
        // Track maximum concurrent readers
        int expected = maxCount.load();
        while (current > expected)
        {
            maxCount.compare_exchange_weak(expected, current);
        }
        this_thread::sleep_for(50ms);
        --insideCount;
    };

    jthread t1(reader);
    jthread t2(reader);
    jthread t3(reader);
    t1.join();
    t2.join();
    t3.join();

    EXPECT_GT(maxCount.load(), 1);
}

TEST(ReadWriteLockTests, exclusiveLockBlocksOtherExclusive)
{
    ReadWriteMutex rwMutex;
    atomic         insideCount {0};
    bool           overlap = false;

    auto writer = [&rwMutex, &insideCount, &overlap]
    {
        ReadWriteLock rwLock(rwMutex, ReadWriteLock::Mode::Writer);

        if (const int current = ++insideCount;
            current > 1)
        {
            overlap = true;
        }

        this_thread::sleep_for(30ms);

        --insideCount;
    };

    jthread t1(writer);
    jthread t2(writer);
    t1.join();
    t2.join();

    EXPECT_FALSE(overlap);
}

TEST(ReadWriteLockTests, exclusiveLockBlocksShared)
{
    ReadWriteMutex rwMutex;
    atomic         sharedWhileExclusive {false};

    ReadWriteLock rwLock(rwMutex, ReadWriteLock::Mode::Writer);

    jthread reader([&rwMutex, &sharedWhileExclusive]
                   {
                       // Try shared with timeout - should fail while exclusive is held
                       try
                       {
                           ReadWriteLock rwLock2(rwMutex, ReadWriteLock::Mode::Reader, 50ms);
                           sharedWhileExclusive = true;
                       }
                       catch (...)
                       {
                           sharedWhileExclusive = false;
                       }
                   });

    this_thread::sleep_for(100ms);
    reader.join();

    EXPECT_FALSE(sharedWhileExclusive);
}

TEST(ReadWriteLockTests, upgradeFromSharedToExclusive)
{
    ReadWriteMutex rwMutex;
    int            sharedValue = 0;

    ReadWriteLock rwLock(rwMutex, ReadWriteLock::Mode::Reader);

    // Read value under reader lock
    const int snapshot = sharedValue;

    // Upgrade to writer lock
    rwLock.upgradeToWriteLock();

    // Now we can write
    sharedValue = snapshot + 1;

    EXPECT_EQ(sharedValue, 1);
}

TEST(ReadWriteLockTests, upgradeWaitsForOtherReaders)
{
    ReadWriteMutex rwMutex;
    atomic         readerInside {false};
    atomic         upgradeCompleted {false};

    // Thread 1: shared lock held for a while
    jthread reader([&rwMutex, &readerInside]
                   {
                       const ReadWriteLock readerLock(rwMutex, ReadWriteLock::Mode::Reader);
                       readerInside = true;
                       this_thread::sleep_for(100ms);
                   });

    // Wait for reader to acquire shared
    while (!readerInside.load())
    {
        this_thread::yield();
    }

    // Thread 2: acquire shared then upgrade - should block until reader releases
    jthread upgrader([&rwMutex, &upgradeCompleted]
                     {
                         const ReadWriteLock writerLock(rwMutex, ReadWriteLock::Mode::Reader);
                         writerLock.upgradeToWriteLock();
                         upgradeCompleted = true;
                     });

    reader.join();
    upgrader.join();

    EXPECT_TRUE(upgradeCompleted);
}

TEST(ReadWriteLockTests, tryLockSharedSuccess)
{
    ReadWriteMutex rwMutex;
    EXPECT_NO_THROW({ const ReadWriteLock rwLock(rwMutex, ReadWriteLock::Mode::Reader, 100ms); });
}

TEST(ReadWriteLockTests, tryLockSharedTimeout)
{
    ReadWriteMutex rwMutex;
    ReadWriteLock  rwLock(rwMutex, ReadWriteLock::Mode::Writer);

    auto acquiredShared = false;
    auto thread = jthread([&rwMutex, &acquiredShared]
                          {
                              try
                              {
                                  ReadWriteLock rwLock(rwMutex, ReadWriteLock::Mode::Reader, 50ms);
                                  acquiredShared = true;
                              }
                              catch (...)
                              {
                                  acquiredShared = false;
                              }
                          });

    thread.join();
    EXPECT_FALSE(acquiredShared);
}

TEST(ReadWriteLockTests, tryLockExclusiveSuccess)
{
    ReadWriteMutex rwMutex;
    EXPECT_NO_THROW({ ReadWriteLock rwLock(rwMutex, ReadWriteLock::Mode::Writer, 100ms); });
}

TEST(ReadWriteLockTests, tryLockExclusiveTimeoutByExclusive)
{
    ReadWriteMutex rwMutex;
    ReadWriteLock  lock(rwMutex, ReadWriteLock::Mode::Writer);

    auto acquiredWriter = false;
    auto thread = jthread([&rwMutex, &acquiredWriter]
                          {
                              try
                              {
                                  ReadWriteLock rwLock(rwMutex, ReadWriteLock::Mode::Writer, 50ms);
                                  acquiredWriter = true;
                              }
                              catch (...)
                              {
                                  acquiredWriter = false;
                              }
                          });

    thread.join();
    EXPECT_FALSE(acquiredWriter);
}

TEST(ReadWriteLockTests, tryLockExclusiveTimeoutByShared)
{
    ReadWriteMutex rwMutex;
    ReadWriteLock  lock(rwMutex, ReadWriteLock::Mode::Reader);

    // Another thread acquires shared and tries to upgrade — should time out
    // because this thread still holds a shared lock
    bool result = false;
    auto thread = jthread([&rwMutex, &result]
                          {
                              ReadWriteLock lock2(rwMutex, ReadWriteLock::Mode::Reader);
                              result = lock2.upgradeToWriteLock(50ms);
                          });
    thread.join();
    EXPECT_FALSE(result);
}

TEST(ReadWriteLockTests, tryUpgradeTimeout)
{
    ReadWriteMutex rwMutex;
    atomic         readerHolding {false};

    // Another thread holds shared lock for a long time
    jthread reader([&readerHolding, &rwMutex]
                   {
                       ReadWriteLock rwLock(rwMutex, ReadWriteLock::Mode::Reader);
                       readerHolding = true;
                       this_thread::sleep_for(200ms);
                   });

    while (!readerHolding.load())
    {
        this_thread::yield();
    }

    {
        // Acquire shared, then try to upgrade with short timeout
        ReadWriteLock rwLock(rwMutex, ReadWriteLock::Mode::Reader);
        const bool    upgraded = rwLock.upgradeToWriteLock(50ms);
        EXPECT_FALSE(upgraded);
    }

    // After failed upgrade, we should still hold shared lock
    reader.join();
}

TEST(ReadWriteLockTests, tryUpgradeSuccess)
{
    ReadWriteMutex      rwMutex;
    const ReadWriteLock lock(rwMutex, ReadWriteLock::Mode::Reader);

    const bool upgraded = lock.upgradeToWriteLock(100ms);
    EXPECT_TRUE(upgraded);
}

TEST(ReadWriteLockTests, performance)
{
    constexpr size_t iterationCount = 4 * 1024 * 1024;
    constexpr size_t threadCount = 2;
    constexpr size_t iterationsPerThread = iterationCount / threadCount;

    // Shared lock upgrade to unique lock, 8 threads
    vector<jthread> threads;
    shared_mutex    shared_mu;
    Stopwatch       stopwatch;
    stopwatch.start();
    for (size_t i = 0; i < threadCount; ++i)
    {
        threads.emplace_back([&shared_mu]
                             {
                                 for (size_t j = 0; j < iterationsPerThread; ++j)
                                 {
                                     shared_lock lock1(shared_mu);
                                     lock1.unlock();
                                     unique_lock lock2(shared_mu);
                                 }
                             });
    }
    threads.clear();
    stopwatch.stop();
    COUT("Shared/Unique locks:  " << fixed << setprecision(1) << stopwatch.milliseconds() << " ms: " << iterationCount / stopwatch.milliseconds() << "K/s");

    ReadWriteMutex rwMutex;
    stopwatch.start();
    for (size_t i = 0; i < threadCount; ++i)
    {
        threads.emplace_back([&rwMutex]
                             {
                                 for (size_t j = 0; j < iterationsPerThread; ++j)
                                 {
                                     ReadWriteLock lock(rwMutex, ReadWriteLock::Mode::Reader);
                                     lock.upgradeToWriteLock();
                                 }
                             });
    }
    threads.clear();
    stopwatch.stop();
    COUT("RWLock/Upgrade locks: " << fixed << setprecision(1) << stopwatch.milliseconds() << " ms: " << iterationCount / stopwatch.milliseconds() << "K/s");
}
