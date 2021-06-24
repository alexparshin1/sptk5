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

#pragma once

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>

#include <shared_mutex>

namespace sptk {

using SharedMutex = std::shared_timed_mutex;
using ReadLockType = std::shared_lock<SharedMutex>;
using WriteLockType = std::unique_lock<SharedMutex>;

/**
 * Unique lock
 *
 * Lock is acquired by constructor.
 * For timed lock, waiting for longer than timeout, throws an exception.
 */
class SP_EXPORT UniqueLockInt
{
public:

    /**
     * Constructor
     * Waits until lock is acquired.
     * @param mutex             Shared mutex that controls lock
     */
    explicit UniqueLockInt(SharedMutex& mutex);

    UniqueLockInt(const UniqueLockInt&) = delete;

    /**
     * Constructor
     * Waits until lock is acquired, or until timeout, and then exception is thrown.
     * @param mutex             Shared mutex that controls lock
     * @param timeout           Lock timeout
     * @param file              Source file name where lock is attempted
     * @param line              Source file line number where lock is attempted
     */
    UniqueLockInt(SharedMutex& mutex, std::chrono::milliseconds timeout, const char* file, size_t line);

    /**
     * Destructor
     * Unlocks lock if it was acquired.
     */
    virtual ~UniqueLockInt()
    {
        if (locked)
        {
            mutex.unlock();
        }
    }

private:

    SharedMutex& mutex;              ///< Shared mutex that controls lock
    bool locked {true};      ///< True if lock is acquired
};

/**
 * Shared lock
 *
 * Lock is acquired by constructor.
 * For timed lock, waiting for longer than timeout, throws an exception.
 */
class SP_EXPORT SharedLockInt
{
public:

    /**
     * Constructor
     * @param mutex             Shared mutex that controls lock
     */
    explicit SharedLockInt(SharedMutex& mutex);

    /**
     * Constructor
     * Waits until lock is acquired, or until timeout, and then exception is thrown.
     * @param mutex             Shared mutex that controls lock
     * @param timeout           Lock timeout
     * @param file              Source file name where lock is attempted
     * @param line              Source file line number where lock is attempted
     */
    SharedLockInt(SharedMutex& mutex, std::chrono::milliseconds timeout, const char* file, size_t line);

    SharedLockInt(const SharedLockInt&) = delete;

    /**
     * Destructor
     * Unlocks lock if it was acquired.
     */
    virtual ~SharedLockInt()
    {
        if (locked)
#if USE_SHARED_MUTEX
            mutex.unlock_shared();
#else
        {
            mutex.unlock();
        }
#endif
    }

private:

    SharedMutex& mutex;              ///< Shared mutex that controls lock
    bool locked {true};      ///< True if lock is acquired
};

/**
 * Copy lock
 * Useful when there is need to copy one object (locked with shared lock),
 * into another (locked with unique lock).
 * Locks are released when CopyLockInt leaves scope.
 */
class CopyLockInt
{
    WriteLockType destinationLock;    ///< Unique lock that belongs to destination object
    ReadLockType sourceLock;         ///< Shared lock that belongs to source object


public:

    /**
     * Constructor
     * Locks both mutexes.
     * @param destinationMutex  Destination mutex
     * @param sourceMutex       Source mutex
     */
    CopyLockInt(SharedMutex& destinationMutex, SharedMutex& sourceMutex);
};

/**
 * Compare lock
 * Useful when there is need to coMPARE one object to another (both locked with shared lock).
 * Locks are released when CompareLockInt leaves scope.
 */
class CompareLockInt
{
    ReadLockType lock1;   ///< Shared lock that belongs to first object
    ReadLockType lock2;   ///< Shared lock that belongs to second object

public:

    /**
     * Constructor
     * Locks both mutexes.
     * @param lock1             First object mutex
     * @param lock2             Second object mutex
     */
    CompareLockInt(SharedMutex& lock1, SharedMutex& lock2);
};

#define UniqueLock(amutex)                      UniqueLockInt  lock(amutex)
#define TimedUniqueLock(amutex, timeout)         UniqueLockInt  lock(amutex,timeout,__FILE__,__LINE__)
#define SharedLock(amutex)                      SharedLockInt  lock(amutex)
#define TimedSharedLock(amutex, timeout)         SharedLockInt  lock(amutex,timeout,__FILE__,__LINE__)
#define CompareLock(mutex1, mutex2)              CompareLockInt lock(mutex1, mutex2)
#define CopyLock(destinationMutex, sourceMutex)  CopyLockInt    lock(destinationMutex, sourceMutex)

} // namespace sptk
