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

#include <sptk5/cutils>

namespace sptk {

class DebugMutex
{
public:
    void lock(std::source_location sourceLocation)
    {
        m_mutex.lock();
        m_lockLocation = sourceLocation;
    }

    bool try_lock_for(std::chrono::milliseconds timeout, std::source_location sourceLocation)
    {
        if (m_mutex.try_lock_for(timeout))
        {
            m_lockLocation = sourceLocation;
            return true;
        }
        return false;
    }

    void unlock()
    {
        m_mutex.unlock();
    }

    std::string location() const
    {
        return m_lockLocation.file_name() + std::string(":") + std::to_string(m_lockLocation.line());
    }

private:
    std::source_location m_lockLocation;
    std::timed_mutex m_mutex;
};

class DebugLock
{
public:
    /**
     * @brief Constructor
     */
    explicit DebugLock(DebugMutex& mutex,
                       std::chrono::milliseconds timeout = std::chrono::seconds(10),
                       std::source_location sourceLocation = std::source_location::current())
        : m_mutex(mutex)
    {
        if (!m_mutex.try_lock_for(timeout, sourceLocation))
        {
            m_locked = false;
            throw std::runtime_error("Failed to acquire lock at " + std::string(sourceLocation.file_name()) +
                                     ":" + std::to_string(sourceLocation.line()) +
                                     ": locked by another thread at" + m_mutex.location());
        }
    }

    ~DebugLock()
    {
        if (m_locked)
        {
            m_mutex.unlock();
        }
    }

private:
    bool m_locked {true};
    DebugMutex& m_mutex;
};

} // namespace sptk
