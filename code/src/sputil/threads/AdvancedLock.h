//
// Created by alexeyp on 8/10/23.
//

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
    explicit DebugLock(DebugMutex& mutex)
        : m_mutex(mutex)
    {
    }

    /**
     * @brief Constructor
     */
    explicit DebugLock(DebugMutex& mutex, std::chrono::milliseconds timeout, std::source_location sourceLocation = std::source_location::current())
        : m_mutex(mutex)
    {
        if (!m_mutex.try_lock_for(timeout, sourceLocation))
        {
            m_locked = false;
            throw std::runtime_error("Failed to acquire lock at " + std::string(sourceLocation.file_name()) + ":" + std::to_string(sourceLocation.line()) +
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
