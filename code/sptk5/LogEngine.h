/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/LogPriority.h>
#include <sptk5/threads/JoiningThread.h>
#include <sptk5/Logger.h>
#include <sptk5/threads/SynchronizedQueue.h>

#include <mutex>
#include <atomic>
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
 * This class is abstract. Derived classes have to implement at least saveMessage(), and each
 * one must stop the save-message thread from its own destructor - terminate() or shutdown().
 *
 * Leaving that to ~LogEngine() is too late. A base destructor runs after the derived one, so
 * by then the vtable has reverted to this class and the still-running thread calls the pure
 * virtual saveMessage() above: __cxa_pure_virtual, then abort. It is the same hazard that
 * keeps the thread out of the constructor (see startSaveMessageThread), at the other end of
 * the object lifetime.
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
     * Stores or sends the log message to actual destination.
     * @param message           Log message.
     */
    virtual bool saveMessage(const Logger::Message& message) = 0;

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
     * @brief Restarts the log.
     *
     * The current log content is cleared.
     * The actual result depends on the derived log engine.
     */
    virtual void reset()
    {
        m_messages.clear();
    }

    /**
     * @brief Sets log options.
     * @param ops               Log options.
     */
    void options(const std::set<Option>& ops)
    {
        const std::lock_guard lock(m_mutex);
        m_options = ops;
    }

    /**
     * @brief Returns log options.
     * @returns log options.
     */
    std::set<Option> options() const
    {
        const std::lock_guard lock(m_mutex);
        return m_options;
    }

    /**
     * @brief Sets an option to true or false.
     * @param option            Log option, one or more of LO_* constants.
     * @param flag              Set option on or off?
     */
    void option(Option option, bool flag);

    /**
     * @brief Gets an option value.
     * @param option            Log option, one or more of LO_* constants.
     * @returns Option value.
     */
    bool option(Option option) const;

    /**
     * @brief Sets current message priority.
     * @param prt LogPriority, current message priority.
     */
    void priority(LogPriority prt)
    {
        m_minPriority = prt;
    }

    /**
     * @brief Get min message priority.
     */
    virtual LogPriority minPriority() const
    {
        return m_minPriority;
    }

    /**
     * @brief Set min message priority.
     *
     * Messages with priority less than requested are ignored
     * @param prt LogPriority, min message priority.
     */
    virtual void minPriority(LogPriority prt)
    {
        m_minPriority = prt;
    }

    /**
     * @brief String representation of priority.
     */
    static String priorityName(LogPriority prt);

    /**
     * @brief Priority from string representation.
     */
    static LogPriority priorityFromName(const String& prt);

    virtual void flush()
    {
        // Implement in derived class
    }

protected:
    enum class ProcessResult : uint8_t
    {
        Ok,
        NoMoreMessages,
        Error
    };

    void          threadFunction();
    ProcessResult processNextMessage();

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
        // Implement in derived class
    }

    /**
     * Shutdown log worker thread
     */
    void shutdown();

    /**
     * Mutex for using in derived classes
     */
    std::mutex& masterLock() const
    {
        return m_mutex;
    }

    /**
     * Terminate the message save thread
     */
    void terminate();

    /**
     * Terminate the message save thread
     */
    bool terminated() const
    {
        return m_terminated;
    }

private:
    /**
     * @brief Starts the message save thread, at most once.
     *
     * Called when the first message is logged rather than from the constructor: the thread calls
     * the pure virtual saveMessage(), which belongs to a derived class that is not constructed yet
     * while the base constructor runs.
     */
    void startSaveMessageThread();

    /**
     * Mutex that protects internal data access
     */
    mutable std::mutex m_mutex;

    /**
     * Thread that saves messages into the backend
     */
    JoiningThread m_saveMessageThread;

    /**
     * Guards the one-time start of m_saveMessageThread
     */
    std::once_flag m_saveMessageThreadStarted;

    /**
     * Min message priority should be defined for every message
     */
    std::atomic<LogPriority> m_minPriority {LogPriority::Info};

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
