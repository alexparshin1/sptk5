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

#include <sptk5/LogEngine.h>
#include <fstream>

namespace sptk {

/**
 * Class name aliases, to make M$ VC-- compile the the class
 */
typedef std::ostream _ostream;
typedef std::ios _ios;

class Logger;

/**
 * @addtogroup log Log Classes
 * @{
 */

/**
 * @brief Internal buffer for the CLogStream class
 */
class SP_EXPORT CLogStreamBuf: public std::streambuf //, public Synchronized
{
    friend class Logger;
private:
    /**
     * Internal buffer to store the current log message
     */
    char*           m_buffer;

    /**
     * The size of the internal buffer
     */
    uint32_t        m_size;

    /**
     * The number of characters in the buffer
     */
    uint32_t        m_bytes;

    /**
     * Message timestamp
     */
    DateTime        m_date;

    /**
     * Current message priority, should be defined for every message
     */
    LogPriority     m_priority;

    /**
     * Parent log object
     */
    Logger*         m_parent;


protected:
    /**
     * @brief Assignes the parent log object
     *
     * @param par Logger *, parent log object
     */
    void parent(Logger *par)
    {
        //SYNCHRONIZED_CODE;
        m_parent = par;
    }

    /**
     * Overwritten virtual method for std::streambuf
     * @param c int_type, a character sent to the stream on overflow
     */
    virtual int_type overflow(int_type c);

public:
    /**
     * @brief Constructor
     *
     * Constructs a buffer for CBaseLog.
     */
    CLogStreamBuf();

    /**
     * @brief Destructor
     *
     * Sends the remaining part of the message to the log,
     * then releases the allocated memory
     */
    ~CLogStreamBuf()
    {
        //SYNCHRONIZED_CODE;
        free(m_buffer);
    }

    /**
     * @brief Flushes message buffer
     *
     * Sends the remaining part of the message to the log
     */
    void flush()
    {
        overflow(char(13));
    }

    /**
     * @brief Sets current message priority
     *
     * @param prt LogPriority, current message priority
     */
    void priority(LogPriority prt)
    {
        //SYNCHRONIZED_CODE;
        m_priority = prt;
    }
};

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
class SP_EXPORT Logger: public _ostream
{
    friend class CLogStreamBuf;

    /**
     * The actual log to store messages to (destination log)
     */
    LogEngine&      m_destination;

    /**
     * Log buffer
     */
    CLogStreamBuf*  m_buffer;


protected:

    /**
     * @brief Sends log message to actual destination
     * @param date DateTime, message timestamp
     * @param message const char *, message text
     * @param sz uint32_t, message size
     * @param priority LogPriority, message priority. @see LogPriority for more information.
     */
    virtual void saveMessage(const DateTime& date, const char* message, uint32_t sz, LogPriority priority);

public:
    /**
     * @brief Constructor
     * @param destination LogEngine&, destination logger
     */
    explicit Logger(LogEngine& destination);

    /**
     * @brief Destructor
     */
    ~Logger();

    /**
     * @brief Sets the message priority for the following messages
     */
    void messagePriority(LogPriority prt) 
    {
        m_buffer->priority(prt);
    }

    /**
     * @brief Returns log engine (destination logger)
     */
    LogEngine& destination()
    {
        return m_destination;
    }
};

/**
 * @brief Sets the message priority
 *
 * By default, the message priority is CLP_NOTICE.
 * Changing the priority would hold till the new log message.
 * The new message would start with the default priority.
 */
SP_EXPORT sptk::Logger& operator <<(sptk::Logger&, sptk::LogPriority);

/**
 * @}
 */
}

#endif
