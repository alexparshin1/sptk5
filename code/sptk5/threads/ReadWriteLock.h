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

#pragma once

#include "sptk5/Exception.h"
#include "sptk5/threads/ReadWriteMutex.h"

namespace sptk {

/**
 * @brief Read/write lock.
 * @remarks Uses RAII for automatic locking and unlocking.
 */
class SP_EXPORT ReadWriteLock
{
public:
    /**
     * @brief Locking mode.
     */
    enum class Mode : uint8_t
    {
        Reader,
        Writer
    };

    /**
     * @brief Constructor.
     * @remarks Locks the mutex.
     * @param rwMutex Read/write Mutex.
     * @param mode Lock mode.
     */
    explicit ReadWriteLock(ReadWriteMutex& rwMutex, const Mode mode)
        : m_mutex(rwMutex)
        , m_mode(mode)
    {
        if (mode == Mode::Writer)
        {
            m_mutex.lockExclusive();
        }
        else
        {
            m_mutex.lockShared();
        }
    }

    /**
     * @brief Constructor.
     * @remarks Locks the mutex.
     * @param rwMutex Read/write Mutex.
     * @param mode Lock mode.
     * @param timeout Lock timeout.
     * @throws TimeoutException if the lock cannot be acquired within the specified timeout.
     */
    explicit ReadWriteLock(ReadWriteMutex& rwMutex, const Mode mode, const std::chrono::milliseconds timeout)
        : m_mutex(rwMutex)
        , m_mode(mode)
    {
        bool locked;
        if (mode == Mode::Writer)
        {
            locked = m_mutex.tryLockExclusive(timeout);
        }
        else
        {
            locked = m_mutex.tryLockShared(timeout);
        }
        if (!locked)
        {
            throw TimeoutException("Failed to acquire lock within timeout");
        }
    }

    ReadWriteLock(const ReadWriteLock&) = delete;
    ReadWriteLock(ReadWriteLock&&) = delete;
    ReadWriteLock& operator=(const ReadWriteLock&) = delete;
    ReadWriteLock& operator=(ReadWriteLock&&) = delete;

    /**
     * @brief Destructor.
     * @remarks Unlocks the mutex.
     */
    ~ReadWriteLock()
    {
        if (m_mode == Mode::Writer)
        {
            m_mutex.unlockExclusive();
        }
        else
        {
            m_mutex.unlockShared();
        }
    }

    /**
     * @brief Upgrades the lock to write mode.
     * @remarks If the lock is already in write mode, this function does nothing.
     */
    void upgradeToWriteLock() const
    {
        if (m_mode == Mode::Writer)
        {
            return;
        }
        m_mutex.upgradeToExclusive();
        m_mode = Mode::Writer;
    }

    /**
     * @brief Tries to upgrade the lock to write mode with a timeout.
     * @param timeout Maximum time to wait.
     * @return True if upgraded, false on timeout (shared lock is retained).
     */
    bool upgradeToWriteLock(const std::chrono::milliseconds timeout) const
    {
        if (m_mode == Mode::Writer)
        {
            return true;
        }
        if (m_mutex.tryUpgradeToExclusive(timeout))
        {
            m_mode = Mode::Writer;
            return true;
        }
        return false;
    }

    /**
     * @brief Downgrades the lock to read mode.
     * @remarks If the lock is already in read mode, this function does nothing.
     */
    void downgradeToReadLock() const
    {
        if (m_mode == Mode::Reader)
        {
            return;
        }
        m_mutex.downgradeToShared();
        m_mode = Mode::Reader;
    }

private:
    ReadWriteMutex& m_mutex; ///< External mutex.
    mutable Mode    m_mode;  ///< Current lock mode.
};

class ReadLock : public ReadWriteLock
{
public:
    explicit ReadLock(ReadWriteMutex& mutex)
        : ReadWriteLock(mutex, Mode::Reader)
    {
    }
};

class WriteLock : public ReadWriteLock
{
public:
    explicit WriteLock(ReadWriteMutex& mutex)
        : ReadWriteLock(mutex, Mode::Writer)
    {
    }
};

} // namespace sptk
