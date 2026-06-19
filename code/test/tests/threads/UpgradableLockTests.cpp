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

#include <atomic>
#include <future>
#include <gtest/gtest.h>
#include <sptk5/threads/UpgradableLock.h>

using namespace std;
using namespace chrono;
using namespace sptk;

TEST(UpgradableLockTests, sharedLockAllowsConcurrentReaders)
{
    UpgradableLock lock;
    atomic         insideCount {0};
    atomic         maxCount {0};

    auto reader = [&lock, &insideCount, &maxCount]
    {
        lock.lockShared();
        const int current = ++insideCount;
        // Track maximum concurrent readers
        int expected = maxCount.load();
        while (current > expected)
            maxCount.compare_exchange_weak(expected, current);
        this_thread::sleep_for(50ms);
        --insideCount;
        lock.unlockShared();
    };

    jthread t1(reader);
    jthread t2(reader);
    jthread t3(reader);
    t1.join();
    t2.join();
    t3.join();

    EXPECT_GT(maxCount.load(), 1);
}

TEST(UpgradableLockTests, exclusiveLockBlocksOtherExclusive)
{
    UpgradableLock lock;
    atomic         insideCount {0};
    bool           overlap = false;

    auto writer = [&lock, &insideCount, &overlap]
    {
        lock.lockExclusive();

        if (const int current = ++insideCount;
            current > 1)
        {
            overlap = true;
        }

        this_thread::sleep_for(30ms);

        --insideCount;
        lock.unlockExclusive();
    };

    jthread t1(writer);
    jthread t2(writer);
    t1.join();
    t2.join();

    EXPECT_FALSE(overlap);
}

TEST(UpgradableLockTests, exclusiveLockBlocksShared)
{
    UpgradableLock lock;
    atomic         exclusiveHeld {false};
    atomic         sharedWhileExclusive {false};

    lock.lockExclusive();
    exclusiveHeld = true;

    jthread reader([&lock, &exclusiveHeld, &sharedWhileExclusive]
                   {
                       // Try shared with timeout - should fail while exclusive is held
                       if (lock.tryLockShared(50ms))
                       {
                           if (exclusiveHeld.load())
                           {
                               sharedWhileExclusive = true;
                           }
                           lock.unlockShared();
                       }
                   });

    this_thread::sleep_for(100ms);
    exclusiveHeld = false;
    lock.unlockExclusive();
    reader.join();

    EXPECT_FALSE(sharedWhileExclusive);
}

TEST(UpgradableLockTests, upgradeFromSharedToExclusive)
{
    UpgradableLock lock;
    int            sharedValue = 0;

    lock.lockShared();
    // Read value under shared lock
    const int snapshot = sharedValue;

    // Upgrade to exclusive
    lock.lockExclusive();
    // Now we can write
    sharedValue = snapshot + 1;
    lock.unlockExclusive();

    EXPECT_EQ(sharedValue, 1);
}

TEST(UpgradableLockTests, upgradeWaitsForOtherReaders)
{
    UpgradableLock lock;
    atomic         readerInside {false};
    atomic         upgradeCompleted {false};

    // Thread 1: shared lock held for a while
    jthread reader([&lock, &readerInside]
                   {
                       lock.lockShared();
                       readerInside = true;
                       this_thread::sleep_for(100ms);
                       lock.unlockShared();
                   });

    // Wait for reader to acquire shared
    while (!readerInside.load())
    {
        this_thread::yield();
    }

    // Thread 2: acquire shared then upgrade - should block until reader releases
    jthread upgrader([&lock, &upgradeCompleted]
                     {
                         lock.lockShared();
                         lock.lockExclusive();
                         upgradeCompleted = true;
                         lock.unlockExclusive();
                     });

    reader.join();
    upgrader.join();

    EXPECT_TRUE(upgradeCompleted);
}

TEST(UpgradableLockTests, tryLockSharedSuccess)
{
    UpgradableLock lock;
    const bool     acquired = lock.tryLockShared(100ms);
    EXPECT_TRUE(acquired);
    lock.unlockShared();
}

TEST(UpgradableLockTests, tryLockSharedTimeout)
{
    UpgradableLock lock;
    lock.lockExclusive();

    auto result = false;
    auto thread = jthread([&lock, &result]
                          {
                              result = lock.tryLockShared(50ms);
                          });
    thread.join();
    EXPECT_FALSE(result);

    lock.unlockExclusive();
}

TEST(UpgradableLockTests, tryLockExclusiveSuccess)
{
    UpgradableLock lock;
    const bool     acquired = lock.tryLockExclusive(100ms);
    EXPECT_TRUE(acquired);
    lock.unlockExclusive();
}

TEST(UpgradableLockTests, tryLockExclusiveTimeoutByExclusive)
{
    UpgradableLock lock;
    lock.lockExclusive();

    auto result = false;
    auto thread = jthread([&lock, &result]
                          {
                              result = lock.tryLockExclusive(50ms);
                          });
    thread.join();
    EXPECT_FALSE(result);

    lock.unlockExclusive();
}

TEST(UpgradableLockTests, tryLockExclusiveTimeoutByShared)
{
    UpgradableLock lock;

    // Hold shared lock from another thread
    lock.lockShared();
    bool result = false;
    auto thread = jthread([&lock, &result]
                          {
                              result = lock.tryLockExclusive(50ms);
                          });
    thread.join();
    EXPECT_FALSE(result);
    lock.unlockShared();
}

TEST(UpgradableLockTests, tryUpgradeTimeout)
{
    UpgradableLock lock;
    atomic         readerHolding {false};

    // Another thread holds shared lock for a long time
    jthread reader([&readerHolding, &lock]
                   {
                       lock.lockShared();
                       readerHolding = true;
                       this_thread::sleep_for(200ms);
                       lock.unlockShared();
                   });

    while (!readerHolding.load())
    {
        this_thread::yield();
    }

    // Acquire shared, then try to upgrade with short timeout
    lock.lockShared();
    const bool upgraded = lock.tryLockExclusive(50ms);
    EXPECT_FALSE(upgraded);

    // After failed upgrade, we should still hold shared lock
    lock.unlockShared();
    reader.join();
}

TEST(UpgradableLockTests, tryUpgradeSuccess)
{
    UpgradableLock lock;

    lock.lockShared();
    const bool upgraded = lock.tryLockExclusive(100ms);
    EXPECT_TRUE(upgraded);
    lock.unlockExclusive();
}
