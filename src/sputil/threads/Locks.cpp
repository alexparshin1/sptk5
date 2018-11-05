/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Locks.h - description                                  ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday September 22 2018                             ║
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

#include <sptk5/threads/Locks.h>
#include <mutex>

using namespace std;
using namespace sptk;

UniqueLockInt::UniqueLockInt(SharedMutex& mutex)
: mutex(mutex)
{
    mutex.lock();
    locked = true;
}

UniqueLockInt::UniqueLockInt(SharedMutex& mutex, std::chrono::milliseconds timeout, const char* file, size_t line)
: mutex(mutex)
{
    if (!mutex.try_lock_for(timeout)) {
        std::stringstream error;
        error << "Can't lock for write, " << file << "(" << line << ")";
        throw TimeoutException(error.str());
    }
}

SharedLockInt::SharedLockInt(SharedMutex& mutex)
: mutex(mutex)
{
    mutex.lock_shared();
    locked = true;
}

SharedLockInt::SharedLockInt(SharedMutex& mutex, std::chrono::milliseconds timeout, const char* file, size_t line)
: mutex(mutex)
{
    if (!mutex.try_lock_shared_for(timeout)) {
        std::stringstream error;
        error << "Can't lock for write, " << file << "(" << line << ")";
        throw TimeoutException(error.str());
    }
}

CopyLockInt::CopyLockInt(SharedMutex& destinationMutex, SharedMutex& sourceMutex)
: destinationLock(destinationMutex, defer_lock),
  sourceLock(sourceMutex, defer_lock)
{
    lock(destinationLock, sourceLock);
}

CompareLockInt::CompareLockInt(SharedMutex& mutex1, SharedMutex& mutex2)
: lock1(mutex1, std::defer_lock),
  lock2(mutex2, std::defer_lock)
{
    lock(lock1, lock2);
}

#if USE_GTEST
#include <gtest/gtest.h>

#include <atomic>

#include <sptk5/threads/Thread.h>

static SharedMutex     amutex;

class TestThread : public Thread
{
public:
    TestThread() : Thread("test")
    {
        run();
    }

    String result;

    void threadFunction() override
    {
        try {
            TimedUniqueLock(amutex, chrono::milliseconds(1000));
            result = "locked";
        }
        catch (const exception& e) {
            result = e.what();
        }
    }
};

TEST(SPTK_Locks, writeLockAndWait)
{
    TimedUniqueLock(amutex, chrono::seconds(2));
    TestThread      th;
    this_thread::sleep_for(chrono::seconds(2));
    th.join();
    cout << th.result << endl;
}

#endif
