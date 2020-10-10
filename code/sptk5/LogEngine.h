/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __LOGENGINE_H__
#define __LOGENGINE_H__

#include <sptk5/DateTime.h>
#include <sptk5/threads/SynchronizedQueue.h>
#include <sptk5/LogPriority.h>
#include <sptk5/Logger.h>

#include <atomic>
#include <iostream>
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
class SP_EXPORT LogEngine : public Thread
{
    friend class Logger;

public:
    /**
     * Stores or sends log message to actual destination
     * @param message           Log message
     */
    virtual void saveMessage(const Logger::Message* message) = 0;

    /**
     * Log options
     */
    enum Option : unsigned
    {
        /**
         * Duplicate messages to stdout
         */
        LO_STDOUT = 1,

        /**
         * Print date for every log message
         */
        LO_DATE = 2,

        /**
         * Print time for every log message
         */
        LO_TIME = 4,

        /**
         * Print message priority
         */
        LO_PRIORITY = 8,

        /**
         * Enable logging (doesn't affect stdout if CLO_STDOUT is on)
         */
        LO_ENABLE = 16

    };

    /**
     * Constructor
     *
     * Creates a new log object.
     */
    explicit LogEngine(const String& logEngineName);

    ~LogEngine() override;

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
     * @param ops int, a bit combination of Option
     */
    void options(int ops)
    {
        m_options = ops;
    }

    /**
     * Returns log options
     * @returns a bit combination of Option
     */
    size_t options() const
    {
        return m_options;
    }

    /**
     * Sets an option to true or false
     */
    void option(Option option, bool flag);

    /**
     * Sets current message priority
     * @param prt LogPriority, current message priority
     */
    void priority(LogPriority prt)
    {
        m_minPriority = prt;
    }

    /**
     * Sets min message priority
     *
     * Messages with priority less than requested are ignored
     * @param prt LogPriority, min message priority
     */
    virtual void minPriority(LogPriority prt)
    {
        m_minPriority = prt;
    }

    /**
     * Returns the min priority
     *
     * Messages with priority less than requested are ignored
     */
    virtual LogPriority minPriority() const
    {
        return m_minPriority;
    }

    /**
     * String representation of priority
     */
    static String priorityName(LogPriority prt);

    /**
     * Priotrity from string representation
     */
    static LogPriority priorityFromName(const String& prt);

protected:

    void threadFunction() override;

    /**
     * Log a message
     * @param message           Message
     */
    void log(Logger::Message* message);

private:
    /**
     * Mutex that protects internal data access
     */
    mutable SharedMutex                 m_mutex;

    /**
     * Min message priority, should be defined for every message
     */
    std::atomic<LogPriority>            m_minPriority {LP_INFO};

    /**
     * Log options, a bit combination of Option
     */
    std::atomic<uint32_t>               m_options {LO_ENABLE | LO_DATE | LO_TIME | LO_PRIORITY};

    /**
     * Message queue
     */
    SynchronizedQueue<Logger::Message*> m_messages;
};

/**
 * @}
 */
}

#endif
