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

#ifndef __SPTK_LOCKS_H__
#define __SPTK_LOCKS_H__

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <shared_mutex>

namespace sptk {

typedef std::shared_timed_mutex             SharedMutex;

class UniqueLockInt
{
    SharedMutex&    mutex;
    bool            locked {false};
public:
    UniqueLockInt(SharedMutex& mutex);
    UniqueLockInt(SharedMutex& mutex, std::chrono::milliseconds timeout, const char* file, size_t line);

    virtual ~UniqueLockInt()
    {
        if (locked)
            mutex.unlock();
    }
};

class SharedLockInt
{
    SharedMutex&    mutex;
    bool            locked {false};
public:
    SharedLockInt(SharedMutex& mutex);
    SharedLockInt(SharedMutex& mutex, std::chrono::milliseconds timeout, const char* file, size_t line);

    virtual ~SharedLockInt()
    {
        if (locked)
            mutex.unlock_shared();
    }
};

class CopyLockInt
{
    std::unique_lock<SharedMutex>   destinationLock;
    std::shared_lock<SharedMutex>   sourceLock;
public:
    CopyLockInt(SharedMutex& destinationMutex, SharedMutex& sourceMutex);
};

class CompareLockInt
{
    std::shared_lock<SharedMutex>   lock1;
    std::shared_lock<SharedMutex>   lock2;
public:
    CompareLockInt(SharedMutex& lock1, SharedMutex& lock2);
};

#define UniqueLock(amutex)                      UniqueLockInt  lock(amutex)
#define TimedUniqueLock(amutex,timeout)         UniqueLockInt  lock(amutex,timeout,__FILE__,__LINE__)
#define SharedLock(amutex)                      SharedLockInt  lock(amutex)
#define TimedSharedLock(amutex,timeout)         SharedLockInt  lock(amutex,timeout,__FILE__,__LINE__)
#define CompareLock(mutex1,mutex2)              CompareLockInt lock(mutex1, mutex2)
#define CopyLock(destinationMutex,sourceMutex)  CopyLockInt    lock(destinationMutex, sourceMutex)

} // namespace sptk

#endif
