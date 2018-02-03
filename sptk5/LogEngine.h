/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       LogEngine.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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
#include <sptk5/threads/SynchronizedCode.h>

#include <iostream>

#ifndef _WIN32
    #include <syslog.h>
#else
    /* priority codes */
    #define LOG_EMERG   0   /* system is unusable */
    #define LOG_ALERT   1   /* action must be taken immediately */
    #define LOG_CRIT    2   /* critical conditions */
    #define LOG_ERR     3   /* error conditions */
    #define LOG_WARNING 4   /* warning conditions */
    #define LOG_NOTICE  5   /* normal but significant condition */
    #define LOG_INFO    6   /* informational */
    #define LOG_DEBUG   7   /* debug-level messages */

    /* facility codes */
    #define LOG_KERN        (0<<3)  /* kernel messages */
    #define LOG_USER        (1<<3)  /* random user-level messages */
    #define LOG_MAIL        (2<<3)  /* mail system */
    #define LOG_DAEMON      (3<<3)  /* system daemons */
    #define LOG_AUTH        (4<<3)  /* security/authorization messages */
    #define LOG_SYSLOG      (5<<3)  /* messages generated internally by syslogd */
    #define LOG_LPR         (6<<3)  /* line printer subsystem */
    #define LOG_NEWS        (7<<3)  /* network news subsystem */
    #define LOG_UUCP        (8<<3)  /* UUCP subsystem */
    #define LOG_CRON        (9<<3)  /* clock daemon */
#endif

namespace sptk {

/**
 * @addtogroup log Log Classes
 * @{
 */

/**
 * @brief Log message priority
 */
enum LogPriority
{
    /**
     * Debug message priority
     */
    LP_DEBUG    = LOG_DEBUG,

    /**
     * Information message priority
     */
    LP_INFO     = LOG_INFO,

    /**
     * Notice message priority
     */
    LP_NOTICE   = LOG_NOTICE,

    /**
     * Warning message priority
     */
    LP_WARNING  = LOG_WARNING,

    /**
     * Error message priority
     */
    LP_ERROR    = LOG_ERR,

    /**
     * Critical message priority
     */
    LP_CRITICAL = LOG_CRIT,

    /**
     * Alert message priority
     */
    LP_ALERT    = LOG_ALERT,

    /**
     * Panic message priority
     */
    LP_PANIC    = LOG_EMERG

};

/**
 * @brief Base class for various log engines.
 *
 * This class is abstract. Derived classes have to implement
 * at least saveMessage() method.
 */
class SP_EXPORT LogEngine: public Synchronized
{
protected:
    /**
     * The default priority for the new message
     */
    LogPriority     m_defaultPriority;

    /**
     * Min message priority, should be defined for every message
     */
    LogPriority     m_minPriority;

    /**
     * Log options, a bit combination of Option
     */
    int             m_options;


public:
    /**
     * @brief Stores or sends log message to actual destination
     * @param date const DateTime&, message timestamp
     * @param message const char *, message text
     * @param sz uint32_t, message size
     * @param priority LogPriority, message priority. @see LogPriority for more information.
     */
    virtual void saveMessage(const DateTime& date, const char *message, uint32_t sz, LogPriority priority) = 0;

    /**
     * @brief Log options
     */
    enum Option
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

public:
    /**
     * @brief Constructor
     *
     * Creates a new log object.
     */
    LogEngine();

    /**
     * @brief Destructor
     *
     * Destructs the log object, releases all the allocated resources
     */
    virtual ~LogEngine();

    /**
     * @brief Restarts the log
     *
     * The current log content is cleared.
     * Actual result depends on derived log engine.
     */
    virtual void reset() {}

    /**
     * @brief Sets log options
     * @param ops int, a bit combination of Option
     */
    void options(int ops)
    {
        SYNCHRONIZED_CODE;
        m_options = ops;
    }

    /**
     * @brief Returns log options
     * @returns a bit combination of Option
     */
    int options()
    {
        SYNCHRONIZED_CODE;
        return m_options;
    }

    /**
     * @brief Sets an option to true or false
     */
    void option(Option option, bool flag);

    /**
     * @brief Sets current message priority
     * @param prt LogPriority, current message priority
     */
    void priority(LogPriority prt)
    {
        SYNCHRONIZED_CODE;
        m_minPriority = prt;
    }

    /**
     * @brief Sets the default priority
     *
     * The default priority is used for the new message,
     * if you are not defining priority.
     * @param priority LogPriority, new default priority
     */
    virtual void defaultPriority(LogPriority priority)
    {
        SYNCHRONIZED_CODE;
        m_defaultPriority = priority;
    }

    /**
     * @brief Returns the default priority
     *
     * The default priority is used for the new message,
     * if you are not defining priority.
     */
    virtual LogPriority defaultPriority()
    {
        SYNCHRONIZED_CODE;
        return m_defaultPriority;
    }

    /**
     * @brief Sets min message priority
     *
     * Messages with priority less than requested are ignored
     * @param prt LogPriority, min message priority
     */
    virtual void minPriority(LogPriority prt)
    {
        SYNCHRONIZED_CODE;
        m_minPriority = prt;
    }

    /**
     * @brief Returns the min priority
     *
     * Messages with priority less than requested are ignored
     */
    virtual LogPriority minPriority()
    {
        SYNCHRONIZED_CODE;
        return m_minPriority;
    }

    /**
     * @brief String representation of priority
     */
    static String priorityName(LogPriority prt);

    /**
     * @brief Priotrity from string representation
     */
    static LogPriority priorityFromName(const String& prt);
};

/**
 * @}
 */
}

#endif
