/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CProxyLog.h  -  description
                             -------------------
    begin                : Tue Jan 11 2008
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
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

#ifndef __CProxyLog_H__
#define __CProxyLog_H__

#include <sptk5/CBaseLog.h>
#include <fstream>

namespace sptk {

/// @addtogroup log Log Classes
/// @{

/// @brief A log that sends all the log messages into another log.
///
/// The destination log is locked for a message adding period.
/// Multiple CProxyLog objects may send messages from different threads
/// into the same destination log.
/// The log options defining message format and min priority are used
/// from destination log.
/// @see CBaseLog for more information about basic log abilities.
class SP_EXPORT CProxyLog: public CBaseLog
{
    CBaseLog& m_destination;  /// The actual log to store messages to (destination log)

protected:

    /// @brief Sets the default priority
    ///
    /// Does nothing since the priority should be defined for the shared (parent) log object
    /// @param priority CLogPriority, new default priority
    virtual void defaultPriority(CLogPriority priority)
    {
    }

    /// @brief Sets min message priority
    ///
    /// Does nothing since the min message priority should be defined for the shared (parent) log object
    /// @param priority CLogPriority, min message priority
    virtual void minPriority(CLogPriority priority)
    {
    }

    /// @brief Sends log message to actual destination
    /// @param date CDateTime, message timestamp
    /// @param message const char *, message text
    /// @param sz uint32_t, message size
    /// @param priority CLogPriority, message priority. @see CLogPriority for more information.
    virtual void saveMessage(CDateTime date, const char *message, uint32_t sz, CLogPriority priority) THROWS_EXCEPTIONS;

public:
    /// @brief Constructor
    ///
    /// Creates a new log object based on the file name.
    /// If this file doesn't exist - it will be created.
    /// @param destination CBaseLog&, destination log object
    CProxyLog(CBaseLog& destination)
    : m_destination(destination)
    {
    }

    /// @brief Restarts the log
    ///
    /// The current log content is cleared. The file is recreated.
    virtual void reset() THROWS_EXCEPTIONS;

    /// @brief Returns the default priority
    ///
    /// The default priority is used for the new message,
    /// if you are not defining priority.
    virtual CLogPriority defaultPriority() const
    {
        return m_destination.defaultPriority();
    }

    /// @brief Returns the min priority
    ///
    /// Messages with priority less than requested are ignored
    virtual CLogPriority minPriority() const
    {
        return m_destination.minPriority();
    }
};
/// @}
}

#endif
