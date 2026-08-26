/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include "sptk5/Printer.h"
#include <sptk5/threads/JoiningThread.h>
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

namespace sptk {

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

    JoiningThread t1(reader);
    JoiningThread t2(reader);
    JoiningThread t3(reader);
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

    JoiningThread t1(writer);
    JoiningThread t2(writer);
    t1.join();
    t2.join();

    EXPECT_FALSE(overlap);
}

TEST(ReadWriteLockTests, exclusiveLockBlocksShared)
{
    ReadWriteMutex rwMutex;
    atomic         sharedWhileExclusive {false};

    ReadWriteLock rwLock(rwMutex, ReadWriteLock::Mode::Writer);

    JoiningThread reader([&rwMutex, &sharedWhileExclusive]
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
    JoiningThread reader([&rwMutex, &readerInside]
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
    JoiningThread upgrader([&rwMutex, &upgradeCompleted]
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
    auto thread = JoiningThread([&rwMutex, &acquiredShared]
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
    auto thread = JoiningThread([&rwMutex, &acquiredWriter]
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
    auto thread = JoiningThread([&rwMutex, &result]
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
    JoiningThread reader([&readerHolding, &rwMutex]
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

TEST(ReadWriteLockTests, pendingWriterBlocksNewReaders)
{
    ReadWriteMutex rwMutex;
    atomic         writerQueued {false};
    atomic         writerAcquired {false};

    JoiningThread writer;
    {
        // Hold a shared lock so a queued writer cannot acquire immediately
        const ReadWriteLock readerLock(rwMutex, ReadWriteLock::Mode::Reader);

        // Writer queues for exclusive access and blocks behind the reader above
        writer = JoiningThread([&rwMutex, &writerQueued, &writerAcquired]
                         {
                             writerQueued = true;
                             const ReadWriteLock writerLock(rwMutex, ReadWriteLock::Mode::Writer);
                             writerAcquired = true;
                         });

        // Give the writer time to register itself as a pending writer
        while (!writerQueued.load())
        {
            this_thread::yield();
        }
        this_thread::sleep_for(50ms);

        // A new reader must yield to the pending writer and time out,
        // even though no exclusive lock is held yet (writer starvation prevention)
        auto lateReaderAcquired = true;
        try
        {
            const ReadWriteLock lateReader(rwMutex, ReadWriteLock::Mode::Reader, 50ms);
        }
        catch (...)
        {
            lateReaderAcquired = false;
        }
        EXPECT_FALSE(lateReaderAcquired);

        // The writer must not have acquired while the original shared lock is held
        EXPECT_FALSE(writerAcquired.load());
    } // shared lock released here, allowing the queued writer to proceed

    writer.join();
    EXPECT_TRUE(writerAcquired.load());
}

TEST(ReadWriteLockTests, writerNotStarvedByContinuousReaders)
{
    ReadWriteMutex rwMutex;
    atomic         stop {false};
    atomic         readerThreadsRunning {0};

    // Steady stream of readers continuously acquiring/releasing shared locks
    JoiningThreads readers;
    readers.reserve(4);
    for (int i = 0; i < 4; ++i)
    {
        readers.emplace_back([&rwMutex, &stop, &readerThreadsRunning]
                             {
                                 ++readerThreadsRunning;
                                 while (!stop.load())
                                 {
                                     ReadWriteLock readerLock(rwMutex, ReadWriteLock::Mode::Reader);
                                     this_thread::sleep_for(1ms);
                                 }
                             });
    }

    // Wait until all reader threads are churning
    while (readerThreadsRunning.load() < 4)
    {
        this_thread::yield();
    }
    this_thread::sleep_for(20ms);

    // The writer should acquire despite the constant reader load
    bool acquired;
    try
    {
        const ReadWriteLock writerLock(rwMutex, ReadWriteLock::Mode::Writer, 2000ms);
        acquired = true;
    }
    catch (...)
    {
        acquired = false;
    }

    stop = true;
    EXPECT_TRUE(acquired);
}

TEST(ReadWriteLockTests, downgradeFromExclusiveToShared)
{
    ReadWriteMutex rwMutex;

    const ReadWriteLock rwLock(rwMutex, ReadWriteLock::Mode::Writer);

    // Write under exclusive lock
    int sharedValue = 42;

    // Downgrade to reader lock
    rwLock.downgradeToReadLock();

    // After downgrade other readers may join concurrently
    EXPECT_TRUE(rwMutex.tryLockShared(50ms));
    rwMutex.unlockShared();

    EXPECT_EQ(sharedValue, 42);
}

TEST(ReadWriteLockTests, downgradeAllowsConcurrentReaders)
{
    ReadWriteMutex rwMutex;
    atomic         readerJoined {false};

    ReadWriteLock writerLock(rwMutex, ReadWriteLock::Mode::Writer);

    // A reader cannot join while exclusive is held
    EXPECT_FALSE(rwMutex.tryLockShared(50ms));

    writerLock.downgradeToReadLock();

    // Now a concurrent reader can join
    JoiningThread reader([&rwMutex, &readerJoined]
                   {
                       const ReadWriteLock readerLock(rwMutex, ReadWriteLock::Mode::Reader, 200ms);
                       readerJoined = true;
                   });
    reader.join();

    EXPECT_TRUE(readerJoined.load());
}

TEST(ReadWriteLockTests, mixedContentionStress)
{
    // Hammer the mutex with readers, writers, upgraders and downgraders all at once to
    // surface lost wake-ups or deadlocks in the lazy-notify / split-CV logic. A protected
    // counter is checked for exclusive-section integrity.
    ReadWriteMutex rwMutex;
    atomic         stop {false};
    atomic<int>    activeWriters {0};
    atomic<bool>   writerOverlap {false};
    long long      protectedValue = 0;
    atomic<bool>   readInconsistency {false};

    JoiningThreads threads;

    auto enterWriteSection = [&]
    {
        if (++activeWriters != 1)
        {
            writerOverlap = true;
        }
        // Two-step mutation: a reader must never observe the intermediate odd state.
        protectedValue += 1;
        protectedValue += 1;
        --activeWriters;
    };

    // Pure readers
    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&]
                             {
                                 while (!stop.load())
                                 {
                                     const ReadWriteLock lock(rwMutex, ReadWriteLock::Mode::Reader);
                                     if (protectedValue % 2 != 0)
                                     {
                                         readInconsistency = true;
                                     }
                                 }
                             });
    }

    // Pure writers
    for (int i = 0; i < 3; ++i)
    {
        threads.emplace_back([&]
                             {
                                 while (!stop.load())
                                 {
                                     const ReadWriteLock lock(rwMutex, ReadWriteLock::Mode::Writer);
                                     enterWriteSection();
                                 }
                             });
    }

    // Upgraders, then downgraders
    for (int i = 0; i < 3; ++i)
    {
        threads.emplace_back([&]
                             {
                                 while (!stop.load())
                                 {
                                     ReadWriteLock lock(rwMutex, ReadWriteLock::Mode::Reader);
                                     if (protectedValue % 2 != 0)
                                     {
                                         readInconsistency = true;
                                     }
                                     if (lock.upgradeToWriteLock(20ms))
                                     {
                                         enterWriteSection();
                                         lock.downgradeToReadLock();
                                         if (protectedValue % 2 != 0)
                                         {
                                             readInconsistency = true;
                                         }
                                     }
                                 }
                             });
    }

    // Timed try-writers (exercise the give-up / baton-passing path)
    for (int i = 0; i < 2; ++i)
    {
        threads.emplace_back([&]
                             {
                                 while (!stop.load())
                                 {
                                     try
                                     {
                                         const ReadWriteLock lock(rwMutex, ReadWriteLock::Mode::Writer, 5ms);
                                         enterWriteSection();
                                     }
                                     catch (const Exception& e)
                                     {
                                         CERR("Timed out waiting for write lock: " << e.what() << ", retrying...");
                                     }
                                 }
                             });
    }

    this_thread::sleep_for(1500ms);
    stop = true;
    threads.clear(); // destroys each JoiningThread, and so joins them all

    EXPECT_FALSE(writerOverlap.load()) << "two writers were in the exclusive section at once";
    EXPECT_FALSE(readInconsistency.load()) << "a reader observed a half-finished write";
    EXPECT_EQ(protectedValue % 2, 0);
    EXPECT_GT(protectedValue, 0) << "no writer ever made progress (possible lost wake-up)";
}

TEST(ReadWriteLockTests, performance)
{
    // 1M lock/unlock cycles across 4 threads is enough to get a stable throughput figure; the
    // 4M this used to run took ~2.8s of the suite to report the same ratio.
    constexpr size_t iterationCount = 1024ul * 1024;
    constexpr size_t threadCount = 4;
    constexpr auto   iterationsPerThread = iterationCount / threadCount;

    // Shared lock upgrade to unique lock
    JoiningThreads threads;
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

} // namespace sptk