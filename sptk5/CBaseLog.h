/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CBaseLog.h  -  description
                             -------------------
    begin                : Mon Jan 30 2006
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com

    This module creation was sponsored by Total Knowledge
    (http://www.total-knowledge.com).
    Author thanks the developers of CPPSERV project
    (http://www.total-knowledge.com/progs/cppserv)
    for defining the requirements for this class.
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CBASELOG_H__
#define __CBASELOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sptk5/sptk.h>
#include <sptk5/CDateTime.h>
#include <sptk5/threads/CSynchronizedCode.h>
#include <sptk5/CException.h>
#include <iostream>
#include <streambuf>

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

/// @addtogroup log Log Classes
/// @{

class CBaseLog;

/// @brief Log message priority
enum CLogPriority {
    CLP_DEBUG = LOG_DEBUG,      ///< Debug message priority
    CLP_INFO = LOG_INFO,        ///< Information message priority
    CLP_NOTICE = LOG_NOTICE,    ///< Notice message priority
    CLP_WARNING = LOG_WARNING,  ///< Warning message priority
    CLP_ERROR = LOG_ERR,        ///< Error message priority
    CLP_CRITICAL = LOG_CRIT,    ///< Critical message priority
    CLP_ALERT = LOG_ALERT,      ///< Alert message priority
    CLP_PANIC = LOG_EMERG       ///< Panic message priority
};

/// @brief Internal buffer for the CLogStream class
class CLogStreamBuf: public std::streambuf
{
    friend class CBaseLog;
private:
    char*           m_buffer;           ///< Internal buffer to store the current log message
    uint32_t        m_size;             ///< The size of the internal buffer
    uint32_t        m_bytes;            ///< The number of characters in the buffer
    CDateTime       m_date;             ///< Message timestamp
    CLogPriority    m_priority;         ///< Current message priority, should be defined for every message
    CBaseLog*       m_parent;           ///< Parent log object

protected:
    /// @brief Assignes the parent log object
    ///
    /// @param par CBaseLog *, parent log object
    void parent(CBaseLog *par)
    {
        m_parent = par;
    }

    /// Overwritten virtual method for std::streambuf
    /// @param c int_type, a character sent to the stream on overflow
    virtual int_type overflow(int_type c);

public:
    /// @brief Constructor
    ///
    /// Constructs a buffer for CBaseLog.
    CLogStreamBuf();

    /// @brief Destructor
    ///
    /// Sends the remaining part of the message to the log,
    /// then releases the allocated memory
    ~CLogStreamBuf()
    {
        flush();
        free(m_buffer);
    }

    /// @brief Flushes message buffer
    ///
    /// Sends the remaining part of the message to the log
    void flush()
    {
        overflow(char(13));
    }

    /// @brief Sets current message priority
    ///
    /// @param prt CLogPriority, current message priority
    void priority(CLogPriority prt)
    {
        m_priority = prt;
    }
};

/// Class name alias for std::ostream, to make M$ VC-- compile the the class
typedef std::ostream _ostream;

/// Class name alias for std::ios, to make M$ VC-- compile the the class
typedef std::ios _ios;

/// Base class for all log classes
class CBaseLog: public CSynchronized, public _ostream
{
    friend class CLogStreamBuf;

protected:
    CLogStreamBuf   m_buffer;            ///< Log buffer
    int             m_indent;            ///< Text indent
    CLogPriority    m_defaultPriority;   ///< The default priority for the new message
    CLogPriority    m_minPriority;       ///< Min message priority, should be defined for every message
    int             m_options;           ///< Log options, a bit combination of CLogOption

public:
    /// @brief Stores or sends log message to actual destination
    ///
    /// This method should be overwritten by the actual log implementation
    /// @param date CDateTime, message timestamp
    /// @param message const char *, message text
    /// @param sz uint32_t, message size
    /// @param priority CLogPriority, message priority. @see CLogPriority for more information.
    virtual void saveMessage(CDateTime date, const char *message, uint32_t sz, CLogPriority priority) throw (CException)
    {
    }

protected:
    /// @brief Protected constructor
    ///
    /// Creates an empty log object. You should never try to instantiate this class.
    /// CBaseLog can only be used as a base class for log classes that implement at least
    /// saveMessage() method.
    /// The default message priority is CLP_NOTICE.
    CBaseLog() :
        _ios(0),
        _ostream(&m_buffer)
    {
        m_buffer.parent(this);
        m_indent = 0;
        m_defaultPriority = CLP_NOTICE;
        m_minPriority = CLP_NOTICE;
        m_options = CLO_ENABLE | CLO_DATE | CLO_TIME | CLO_PRIORITY;
    }
public:

    /// @brief Log options
    enum CLogOption
    {
        CLO_STDOUT = 1,    ///< Duplicate messages to stdout
        CLO_DATE = 2,      ///< Print date for every log message
        CLO_TIME = 4,      ///< Print time for every log message
        CLO_PRIORITY = 8,  ///< Print message priority
        CLO_ENABLE = 16
    ///< Enable logging (doesn't affect stdout if CLO_STDOUT is on)
    };

    /// @brief Destructor
    /// Flushes the log and releases any allocated resources
    virtual ~CBaseLog()
    {
        flush();
    }

    /// @brief Sets log options
    /// @param ops int, a bit combination of CLogOption
    void options(int ops)
    {
        m_options = ops;
    }

    /// @brief Returns log options
    /// @returns a bit combination of CLogOption
    int options() const
    {
        return m_options;
    }

    /// @brief Sets an option to true or false
    void option(CLogOption option, bool flag);

    /// @brief Sets current message priority
    /// @param prt CLogPriority, current message priority
    void priority(CLogPriority prt)
    {
        m_buffer.priority(prt);
    }

    /// @brief Restarts the log, if applicable
    /// In CBaseLog it does nothing
    virtual void reset() throw (CException)
    {
    }

    /// @brief Sets the default priority
    ///
    /// The default priority is used for the new message,
    /// if you are not defining priority.
    /// @param priority CLogPriority, new default priority
    virtual void defaultPriority(CLogPriority priority)
    {
        m_defaultPriority = priority;
    }

    /// @brief Returns the default priority
    ///
    /// The default priority is used for the new message,
    /// if you are not defining priority.
    virtual CLogPriority defaultPriority() const
    {
        return m_defaultPriority;
    }

    /// @brief Sets min message priority
    ///
    /// Messages with priority less than requested are ignored
    /// @param prt CLogPriority, min message priority
    virtual void minPriority(CLogPriority prt)
    {
        m_minPriority = prt;
    }

    /// @brief Returns the min priority
    ///
    /// Messages with priority less than requested are ignored
    virtual CLogPriority minPriority() const
    {
        return m_minPriority;
    }

    /// @brief Returns message priority name
    /// @param prt CLogPriority, message priority
    static std::string priorityName(sptk::CLogPriority prt);
};

/// @brief Sets the message priority
///
/// By default, the message priority is CLP_NOTICE.
/// Changing the priority would hold till the new log message.
/// The new message would start with the default priority.
SP_EXPORT sptk::CBaseLog& operator <<(sptk::CBaseLog &, sptk::CLogPriority);

/// @}
}

#endif
