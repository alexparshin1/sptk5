/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          SysLogEngine.h  -  description
                             -------------------
    begin                : Tue Jan 31 2006
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 
    This module creation was sponsored by Total Knowledge (http://www.total-knowledge.com).
    Author thanks the developers of CPPSERV project (http://www.total-knowledge.com/progs/cppserv)
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

#ifndef __CSYSLOG_H__
#define __CSYSLOG_H__

#ifndef _WIN32
#include <syslog.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

#include <sptk5/LogEngine.h>

namespace sptk
{

/// @addtogroup log Log Classes
/// @{

/// @brief A log stored in the system log.
///
/// On *nix , the log is sent to *nix syslog daemon.
/// On Windows NT/2000+/XP the log is sent to Event Log (Application).
/// On Windows 95/98/ME the system log isn't supported..
/// The facility method allows to define - which system log is used
/// @see CBaseLog for more information about basic log abilities.
class SP_EXPORT SysLogEngine: public LogEngine
{
#ifdef _WIN32
    HANDLE              m_logHandle;    ///< (Windows) The handle of the log file
#endif
    
    uint32_t            m_facilities;   ///< List of facilities allows to define one or more system logs where messages would be sent
    std::string         m_programName;

    void programName(std::string progName);
    
public:
    /// @brief Stores or sends log message to actual destination
    ///
    /// This method should be overwritten by the actual log implementation
    /// @param date CDateTime, message timestamp
    /// @param message const char *, message text
    /// @param sz uint32_t, message size
    /// @param priority CLogPriority, message priority. @see CLogPriority for more information.
    virtual void saveMessage(CDateTime date, const char *message, uint32_t sz, LogPriority priority) THROWS_EXCEPTIONS;
public:
    /// @brief Constructor
    ///
    /// Creates a new log object based on the syslog facility (or facilities).
    /// For Windows, parameter facilities is ignored and messages are stored
    /// into Application event log.
    /// The program name is optional. It is set for all the CSysLog objects at once.
    /// If set, it appears in the log as a message prefix. Every time the program
    /// name is changed, the log is closed to be re-opened on next message.
    /// @param progName std::string, a program name
    /// @param facilities int, log facility or a set of facilities.
    SysLogEngine(std::string programName = "", uint32_t facilities = LOG_USER);

    /// @brief Destructor
    ///
    /// Destructs the log object, closes the log descriptor, releases all the allocated resources
    virtual ~SysLogEngine();
};
/// @}
}
#endif
