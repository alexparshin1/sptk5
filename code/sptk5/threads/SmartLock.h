/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <atomic>
#include <mutex>
#include <source_location>
#include <string>

namespace sptk {

/**
 * @brief Mutex that stores lock location information
 */
class SP_EXPORT SmartMutex
{
public:
    /**
     * @brief Lock mutex and stor lock location
     * @param sourceLocation    Lock location
     */
    void lock(std::source_location sourceLocation)
    {
        m_mutex.lock();
        m_lockLocation = sourceLocation;
    }

    /**
     * @brief Try to lock mutex and store lock location
     * @param timeout           Lock timeout
     * @param sourceLocation    Lock location
     * @return
     */
    bool try_lock_for(std::chrono::milliseconds timeout, std::source_location sourceLocation)
    {
        if (m_mutex.try_lock_for(timeout))
        {
            m_lockLocation = sourceLocation;
            return true;
        }
        return false;
    }

    /**
     * @brief Unlock mutex
     */
    void unlock()
    {
        m_mutex.unlock();
    }

    /**
     * @return Locl location as string
     */
    [[nodiscard]] std::string location() const;

private:
    std::source_location m_lockLocation; ///< Lock location
    std::timed_mutex m_mutex;            ///< Mutex
};

/**
 * @brief Debug lock that stores the location of the successful lock
 */
class SP_EXPORT SmartLock
{
public:
    /**
     * @brief Constructor
     * @param mutex             Mutex
     * @param timeout           Lock timeout
     * @param sourceLocation    Lock location
     */
    explicit SmartLock(SmartMutex& mutex,
                       std::chrono::milliseconds timeout = std::chrono::seconds(5),
                       std::source_location sourceLocation = std::source_location::current())
        : m_mutex(mutex)
    {
        if (!m_mutex.try_lock_for(timeout, sourceLocation))
        {
            throwTimeout(sourceLocation);
        }
    }

    /**
     * Destructor that unlocks the mutex if it was locked
     */
    ~SmartLock()
    {
        if (m_locked)
        {
            m_mutex.unlock();
        }
    }

private:
    std::atomic_bool m_locked {true};                                    ///< Mutex was locked flag
    SmartMutex& m_mutex;                                                 ///< Mutex
    [[noreturn]] void throwTimeout(std::source_location sourceLocation); ///< Throw timeout
};

} // namespace sptk
