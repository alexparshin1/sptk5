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

#include <sptk5/DateTime.h>
#include <sptk5/LogPriority.h>
#include <sptk5/Logger.h>
#include <sptk5/threads/SynchronizedQueue.h>

#include <atomic>
#include <iostream>
#include <set>
#include <sptk5/threads/Thread.h>

namespace sptk {

/**
 * @addtogroup log Log Classes
 * @{
 */

/**
 * Base class for various log engines.
 *
 * This class is abstract. Derived classes have to implement
 * at least saveMessage() method.
 */
class SP_EXPORT LogEngine
{
    friend class Logger;

public:
    // Log options
    enum class Option : uint8_t
    {
        STDOUT,      ///< Duplicate messages to stdout
        DATE,        ///< Print date for every log message
        TIME,        ///< Print time for every log message
        PRIORITY,    ///< Print message priority
        ENABLE,      ///< Enable logging (doesn't affect stdout if CLO_STDOUT is on)
        MILLISECONDS ///< Enable logging (doesn't affect stdout if CLO_STDOUT is on)
    };

    /**
     * Stores or sends log message to actual destination
     * @param message           Log message
     */
    virtual void saveMessage(const Logger::Message& message) = 0;

    /**
     * Constructor
     *
     * Creates a new log object.
     */
    explicit LogEngine(const String& logEngineName);

    /**
     * Destructor
     */
    virtual ~LogEngine();

    /**
     * Restarts the log
     *
     * The current log content is cleared.
     * Actual result depends on derived log engine.
     */
    virtual void reset()
    {
        // Implement in derived class
    }

    /**
     * Sets log options
     * @param ops               Log options
     */
    void options(const std::set<Option>& ops)
    {
        const std::scoped_lock lock(m_mutex);
        m_options = ops;
    }

    /**
     * Returns log options
     * @returns log options
     */
    std::set<Option> options() const
    {
        const std::scoped_lock lock(m_mutex);
        return m_options;
    }

    /**
     * Sets an option to true or false
     * @param option            Log option, one or more of LO_* constants
     * @param flag              Set option on or off?
     */
    void option(Option option, bool flag);

    /**
     * Gets an option value
     * @param option            Log option, one or more of LO_* constants
     * @returns Option value
     */
    bool option(Option option) const;

    /**
     * Sets current message priority
     * @param prt LogPriority, current message priority
     */
    void priority(LogPriority prt)
    {
        m_minPriority = prt;
    }

    /**
     * Get min message priority
     */
    virtual LogPriority minPriority() const
    {
        return m_minPriority;
    }

    /**
     * Set min message priority
     *
     * Messages with priority less than requested are ignored
     * @param prt LogPriority, min message priority
     */
    virtual void minPriority(LogPriority prt)
    {
        m_minPriority = prt;
    }

    /**
     * String representation of priority
     */
    static String priorityName(LogPriority prt);

    /**
     * Priority from string representation
     */
    static LogPriority priorityFromName(const String& prt);

protected:
    void threadFunction();

    /**
     * Log a message
     * @param message           Message
     */
    void log(Logger::UMessage&& message);

    /**
     * Flush messages
     */
    virtual void close()
    {
    }

    /**
     * Shutdown log worker thread
     */
    void shutdown();

    /**
     * Mutex for using in derived classes
     */
    std::mutex& masterLock()
    {
        return m_mutex;
    }

    /**
     * Terminate message save thread
     */
    void terminate();

    /**
     * Terminate message save thread
     */
    bool terminated() const
    {
        return m_terminated;
    }

private:
    /**
     * Mutex that protects internal data access
     */
    mutable std::mutex m_mutex;

    /**
     * Thread that saves messages into backend
     */
    std::jthread m_saveMessageThread;

    /**
     * Min message priority, should be defined for every message
     */
    std::atomic<LogPriority> m_minPriority {LogPriority::INFO};

    /**
     * Log options, a bit combination of Option
     */
    std::set<Option> m_options {Option::ENABLE, Option::DATE, Option::TIME, Option::PRIORITY};

    std::atomic_bool m_terminated {false};

    using MessageQueue = SynchronizedQueue<Logger::UMessage>;
    /**
     * Message queue
     */
    MessageQueue m_messages;
};

/**
 * @}
 */
} // namespace sptk
