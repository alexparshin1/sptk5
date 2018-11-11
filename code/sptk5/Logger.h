/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Logger.h - description                                 ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <sptk5/sptk.h>
#include <sptk5/DateTime.h>
#include "LogPriority.h"

namespace sptk {

class LogEngine;

/**
 * @addtogroup log Log Classes
 * @{
 */

/**
 * @brief A log that sends all the log messages into another log.
 *
 * The destination log is locked for a message adding period.
 * Multiple Logger objects may send messages from different threads
 * into the same destination log.
 * The log options defining message format and min priority are used
 * from destination log.
 * @see CBaseLog for more information about basic log abilities.
 */
class SP_EXPORT Logger
{
    /**
     * The actual log to store messages to (destination log)
     */
    LogEngine&      m_destination;

public:

    /**
     * Log message
     */
    struct Message
    {
        DateTime    timestamp;      ///< Message timestamp
        LogPriority priority;       ///< Message priority
        String      message;        ///< Message text
        /**
         * Constructor
         * @param priority       Message priority
         * @param message        Message text
         */
        Message(LogPriority priority, const String& message);
    };

public:
    /**
     * @brief Constructor
     * @param destination LogEngine&, destination logger
     */
    explicit Logger(LogEngine& destination);

    /**
     * @brief Returns log engine (destination logger)
     */
    LogEngine& destination()
    {
        return m_destination;
    }

    /**
     * Log message with any priority
     * @param priority          Message priority
     * @param message           Message text
     */
    void log(LogPriority priority, const String& message);

    /**
     * Log message with debug priority
     * @param message           Message text
     */
    void debug(const String& message);

    /**
     * Log message with info priority
     * @param message           Message text
     */
    void info(const String& message);

    /**
     * Log message with notice priority
     * @param message           Message text
     */
    void notice(const String& message);

    /**
     * Log message with warning priority
     * @param message           Message text
     */
    void warning(const String& message);

    /**
     * Log message with error priority
     * @param message           Message text
     */
    void error(const String& message);

    /**
     * Log message with critical priority
     * @param message           Message text
     */
    void critical(const String& message);
};

/**
 * @}
 */
}

#endif
