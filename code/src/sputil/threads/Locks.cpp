/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/Printer.h>
#include <mutex>

#ifdef USE_GTEST

#include <sptk5/threads/Thread.h>

#endif

using namespace std;
using namespace sptk;

UniqueLockInt::UniqueLockInt(SharedMutex& mutex)
    : mutex(mutex)
{
    mutex.lock();
}

UniqueLockInt::UniqueLockInt(SharedMutex& mutex, std::chrono::milliseconds timeout, const char* file, size_t line)
    : mutex(mutex)
{
    if (!mutex.try_lock_for(timeout))
    {
        locked = false;
        std::stringstream error;
        error << "Can't lock for write, " << file << "(" << line << ")";
        throw TimeoutException(error.str());
    }
}

SharedLockInt::SharedLockInt(SharedMutex& mutex)
    : mutex(mutex)
{
#if USE_SHARED_MUTEX
    mutex.lock_shared();
#else
    mutex.lock();
#endif
}

SharedLockInt::SharedLockInt(SharedMutex& mutex, std::chrono::milliseconds timeout, const char* file, size_t line)
    : mutex(mutex)
{
#if USE_SHARED_MUTEX
    if (!mutex.try_lock_shared_for(timeout)) {
#else
    if (!mutex.try_lock_for(timeout))
    {
#endif
        locked = false;
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

#ifdef USE_GTEST

class LockTestThread
    : public Thread
{
public:
    static SharedMutex amutex;

    LockTestThread()
        : Thread("test")
    {
    }

    void threadFunction() override
    {
        try
        {
            TimedUniqueLock(amutex, chrono::milliseconds(100));
            aresult = "locked";
        }
        catch (const Exception& e)
        {
            aresult = "lock timeout: " + String(e.what());
        }
    }

    String result() const
    {
        return aresult;
    }

private:

    String aresult;
};

SharedMutex  LockTestThread::amutex;

TEST(SPTK_Locks, writeLockAndWait)
{
    UniqueLock(LockTestThread::amutex);
    LockTestThread th;
    th.run();
    this_thread::sleep_for(chrono::milliseconds(200));
    th.join();
    EXPECT_TRUE(th.result().startsWith("lock timeout"));
}

#endif
